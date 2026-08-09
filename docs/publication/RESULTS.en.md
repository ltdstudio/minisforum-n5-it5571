# Minisforum N5 (F8NAA) EC Fan and Temperature Reverse-Engineering Results

Status: pre-publication results draft  
Hardware validation date: 2026-08-08  
Target: Minisforum N5 / F8NAA mainboard / BIOS 1.04  
Validated OS: Unraid 7.3.2 / Linux 6.18.38-Unraid

## 1. Goal

This project adds a standard Linux hwmon interface for the Minisforum N5
embedded controller so that Dynamix Fan Auto Control and Dynamix System
Temperature can use the board's native fan and temperature functions.

The target feature set is:

- Four PWM outputs: CPU, SSD, HDD, and PCIe.
- Three confirmed tachometer inputs: CPU, SSD, and HDD.
- Four EC temperatures: CPU, System, Board, and Ambient.
- Optional PCIe PWM control linked to a selected PCIe-device temperature.
- Fail-safe manual/automatic transitions and automatic restoration on unload.

## 2. Hardware identity

| Field | Validated value |
|---|---|
| System Product | N5 |
| Board Name | F8NAA |
| Board Vendor | Shenzhen Meigao Electronic Equipment Co., Ltd. |
| EC/Super I/O | ITE IT5571 C Version |
| BIOS | 1.04 |

The public driver should require both the `N5` product and `F8NAA` board DMI
matches. A debug-only `force=1` option must not be the normal installation path.

## 3. Host interfaces and firmware protocol

| Interface | Data | Command/Status | Purpose |
|---|---:|---:|---|
| PMC1 / ACPI EC | `0x62` | `0x66` | EC RAM temperatures |
| PMC2 | `0x68` | `0x6c` | PWM and RPM commands |

PMC2 uses top-level command `0xd5`. An older `0xd9` hypothesis was disproved on
hardware: `0xd9` times out while `0xd5` works reliably.

### PWM protocol

| Linux channel | Physical use | Firmware PWM | Value command(s) | Manual | EC auto |
|---|---|---|---|---:|---:|
| `pwm1` | CPU Fan | DCR1 | `0x23/25/27/29` | `0x20` | `0x21` |
| `pwm2` | SSD Fan | DCR2 | `0x2f` | `0x2d` | `0x2e` |
| `pwm3` | HDD Fan Group | DCR3 + DCR4 | `0x2c` | `0x2a` | `0x2b` |
| `pwm4` | PCIe Fan | DCR5 | `0x33/35/37/39` | `0x30` | `0x31` |

### Firmware-exported tachometers

| Tachometer | Low | High | Validated use |
|---|---:|---:|---|
| TACH3 | `0x14` | `0x15` | HDD Fan |
| TACH2 | `0x16` | `0x17` | SSD Fan |
| TACH1 | `0x18` | `0x19` | CPU Fan |

The exported value is `rpm = (high << 8) | low`. The driver uses a high-low-high
sampling sequence to reject torn 16-bit samples.

## 4. Temperature registers

| hwmon node | Label | EC RAM | Validation behavior |
|---|---|---:|---|
| `temp1_input` | CPU Temp | `0x09` | Tracks CPU load quickly |
| `temp2_input` | System Temp | `0x04` | Tracks chassis heat slowly |
| `temp3_input` | Board Temp | `0x05` | Board/auxiliary thermal zone |
| `temp4_input` | Ambient Temp | `0x06` | Ambient/intake region |

Representative readings were CPU 44°C, System 42°C, Board 40°C, and Ambient
31°C. The BIOS does not register a standard ACPI EC instance usable by Linux
`ec_read()`, so the working implementation performs EC transactions through
ports `0x62/0x66`.

## 5. PCIe fan validation

A known-good PWM fan with a working tachometer output was moved from the SSD
header to the PCIe Fan header:

- `pwm4=128`: the fan slowed noticeably, confirming intermediate-duty control.
- `pwm4=0`: the fan stopped after an approximately 20-second spin-down delay.
- `pwm4=255`: the fan started reliably from rest and ran at full speed.
- `pwm4_enable=2`: EC automatic mode was restored after testing.

This proves that DCR5 and the fourth Linux PWM channel control the physical
PCIe Fan header.

After restoring the original cabling and rebooting Unraid, the driver loaded
automatically. CPU, SSD, and HDD tachometers reported approximately 2284, 2269,
and 1189 RPM, with all four PWM channels in EC automatic mode. The reinstalled
NVIDIA Tesla P4 was detected by PCIe and exposed its GPU temperature through
`nvidia-smi` (46°C in this boot-time sample).

### PCIe RPM result

The three firmware-exported tachometers did not change between PCIe stopped and
full-speed states:

```text
STOP: TACH1≈2005, TACH2=0, TACH3≈1279
FULL: TACH1≈2013, TACH2=0, TACH3≈1278
```

Read-only I2EC inspection showed:

| Register | Value | Meaning |
|---|---:|---|
| `0x1848` | `0x08` | TACH0 and TACH1 select their A inputs |
| `0x184f` | `0x02` | TACH2 selects its A input |
| `0x16f4` | `0x00` | TACH0B and TACH1B are disabled |
| `0x16fe` | `0x00` | TACH2B is disabled |

The IT5571 has three tachometer counters, each switchable between A and B pins.
The N5 firmware configures and exports only the three A inputs. The B pins are
currently ordinary GPIOs; forcing them into tachometer mode could affect other
board functions and is inappropriate for a public driver.

Final result: PCIe PWM control is fully supported, but there is no safe,
confirmed PCIe RPM feedback. The public label should be
`PCIe Fan (no RPM feedback)` and `fan4_input` should return zero.

## 6. Linux hwmon design

Recommended public module name:

```text
minisforum_n5_it5571
```

The validated prototype is currently named `minisforum_n5_it5571`; it should be renamed
consistently before the first public release.

Exposed standard nodes:

```text
temp1_input ... temp4_input
fan1_input  ... fan4_input
fan1_min    ... fan4_min
pwm1        ... pwm4
pwm1_enable ... pwm4_enable
```

`pwmN_enable` semantics:

- `0`: fail-safe full speed.
- `1`: manual Linux/userspace control.
- `2`: native EC/BIOS automatic control.

Entering manual mode starts at full speed. Removing the module restores all four
channels to EC automatic mode.

## 7. Unraid integration

The Dynamix System Temperature driver list should contain only:

```text
minisforum_n5_it5571
```

The existing `nct6775` line is a stale configuration and does not match this N5.

Dynamix Fan Auto Control discovers all four standard PWM nodes. The first three
can be correlated with real RPM. PCIe has no RPM and therefore cannot use the
plugin's RPM-based automatic detection.

An optional companion service can map a specifically selected PCIe hwmon
temperature to `pwm4`. Selection must include device identity and sensor label;
PCIe devices do not share a universal temperature-probe layout.

## 8. Safety properties

- Strict N5/F8NAA DMI match.
- Exclusive EC/PMC I/O-region ownership.
- Serialized PMC2 transactions.
- PWM writes limited to 0-255.
- Full speed before entering manual mode.
- Full-speed fail-safe if a selected PCIe temperature source disappears.
- EC auto restoration on service exit and module unload.
- Refusal to install a module built for a different kernel release.

## 9. Validation scope

Validated:

- Unraid 7.3.2 and `6.18.38-Unraid`.
- BIOS 1.04.
- Four PWM outputs.
- Three real RPM inputs.
- Four EC temperatures.
- Absence of usable PCIe RPM feedback.
- Module load/unload, manual mode, and EC-auto restoration.

Still required before public release:

- Rename the module to `minisforum_n5_it5571`.
- Build kernel-specific `.txz` assets and a `.plg` installer.
- Test install, update, uninstall, and reinstall paths.
- Publish GPL-2.0 source and reproducible build instructions.

## 10. Publication boundary

The public repository may contain independently written source code, build
scripts, test results, and register conclusions. Do not redistribute vendor EC
firmware images or the confidential IT5571 datasheet. The README may link to the
official Linux hwmon documentation:

- https://docs.kernel.org/hwmon/hwmon-kernel-api.html
- https://docs.kernel.org/hwmon/sysfs-interface.html
