# Minisforum N5 EC / IT5571 Driver for Unraid


English | [简体中文](README.zh-CN.md)

`minisforum_n5_it5571` is a Linux hwmon driver for the Minisforum N5 F8NAA
mainboard. It exposes four fan PWM controls, three confirmed tachometer inputs,
and four EC temperature sensors to Unraid.

## Features

- CPU Fan: PWM and RPM.
- SSD Fan: PWM and RPM.
- HDD Fan Group: PWM and RPM.
- PCIe Fan: PWM, without usable RPM feedback.
- CPU, System, Board, and Ambient temperatures.
- Recommended FanCtrl Plus 2 integration for multi-zone control from disk,
  NVMe, CPU, auxiliary hwmon, StorCLI, and NVIDIA GPU temperatures.
- Dynamix Fan Auto Control compatibility.
- Recommended Dynamix System Temperature integration for displaying the CPU,
  System, Board, and Ambient EC sensors.
- Automatic restoration of native EC/BIOS control on exit or module unload.

## Compatibility

| Item | Validated version |
|---|---|
| Device | Minisforum N5 |
| Mainboard | F8NAA |
| BIOS | 1.04 |
| Unraid | 7.3.2 |
| Kernel | 6.18.38-Unraid |

The module must match `uname -r` exactly. A new package is required after an
Unraid kernel update. The driver refuses to load on systems that do not match
the N5/F8NAA DMI identity.

Using the same IT5571 chip does not imply drop-in compatibility. Vendors may use
different PMC commands, ports, EC RAM layouts, and fan wiring. The compatibility
list currently contains only the fully validated N5/F8NAA. Owners of other
IT5571 systems may submit DMI, firmware, and hardware test data for a separate
validated board profile.

Other publicly documented IT5571 systems include the
[Avalue EMX-EHLP](https://www.avalue.com/en/product/Industrial-Embedded-Motherboard/Mini-ITX/EMX-EHLP)
industrial mainboard and the
[System76 Pangolin (pang13)](https://system76.com/tech-docs/models/pang13/README.html).
They are potential porting targets, not currently supported systems. Do not
bypass the DMI guard to force-load this driver on them.

## hwmon channels

| Node | Function |
|---|---|
| `pwm1`, `fan1_input` | CPU Fan |
| `pwm2`, `fan2_input` | SSD Fan |
| `pwm3`, `fan3_input` | HDD Fan Group |
| `pwm4`, `fan4_input` | PCIe Fan; RPM reports zero |
| `temp1_input` | CPU Temp |
| `temp2_input` | System Temp |
| `temp3_input` | Board Temp |
| `temp4_input` | Ambient Temp |

The PCIe header was physically validated: `pwm4=128` noticeably reduces speed,
`pwm4=0` stops the fan, and `pwm4=255` reliably starts it from rest and runs it
at full speed. All four PWM channels return to EC automatic mode after reboot.
The board firmware does not enable or export a safe PCIe tachometer path, so
the driver does not fabricate an RPM value.

## Installation

### Community Applications

After acceptance, search Unraid Apps for:

```text
Minisforum N5 EC / IT5571 Driver
```

### Manual plugin installation

Open **Plugins → Install Plugin** and enter:

```text
https://raw.githubusercontent.com/ltdstudio/minisforum-n5-it5571/main/minisforum-n5-it5571.plg
```

The maintainer must publish a release asset for each supported Unraid kernel.

## Recommended companion: Dynamix System Temperature

Use **Dynamix System Temperature** as the temperature-display layer. This
driver provides standard hwmon temperature nodes, while Dynamix System
Temperature selects and displays them in the Unraid WebGUI and Dashboard.

Open:

```text
Settings → System Temperature
```

Select `minisforum_n5_it5571` as the available driver, then select CPU Temp and
System Temp. Board Temp and Ambient Temp may also be displayed as desired. A
stale `nct6775` entry is not applicable to this system and should be removed
from the selected drivers during migration.

Recommended division of responsibilities:

- **Dynamix System Temperature**: sensor discovery, selection, and Dashboard display.
- **FanCtrl Plus 2**: automatic four-zone PWM control from selected temperatures.
- **Minisforum N5 EC / IT5571 Driver**: low-level PWM, RPM, and EC hwmon nodes.

## Recommended companion: FanCtrl Plus 2

[FanCtrl Plus 2](https://github.com/andrebrait/fanctrlplus) is the recommended
user-space controller for this driver. It controls multiple PWM channels
independently and supports disk, NVMe, CPU, auxiliary hwmon, StorCLI, and NVIDIA
GPU temperature sources, matching the N5's four cooling zones well.

Search Community Applications for:

```text
FanCtrl Plus 2
```

If it is not yet visible in the current CA feed, use the project's documented
URL under **Plugins → Install Plugin**:

```text
https://raw.githubusercontent.com/andrebrait/fanctrlplus/main/plugin/fanctrlplus2.plg
```

Suggested mapping:

| N5 controller | Recommended temperature source |
|---|---|
| CPU Fan / `pwm1` | CPU Temp |
| SSD Fan / `pwm2` | Corresponding NVMe temperature group |
| HDD Fan Group / `pwm3` | Array-disk temperature group |
| PCIe Fan / `pwm4` | NVIDIA GPU or target PCIe-device auxiliary temperature |

The NVIDIA Tesla P4 in the validation system exposes GPU temperature through
`nvidia-smi`, and FanCtrl Plus 2 supports NVIDIA GPU sensors. Binding that GPU
temperature to PCIe Fan is therefore the recommended setup. The PCIe header has
no RPM feedback, so a displayed value of zero RPM is expected.

FanCtrl Plus 2 cannot run alongside upstream FanCtrl Plus. Do not let Dynamix
Fan Auto Control or any other service control the same `pwmN` concurrently,
because competing control loops will overwrite each other.

### Dynamix Fan Auto Control alternative

The standard Dynamix plugin can still discover the driver's hwmon PWM nodes.
Stop FanCtrl Plus 2 first, select the N5 channels again, and do not reuse stale
`it87` paths.

## PCIe temperature sources

PCIe devices do not share one universal temperature-sensor layout. Select a
specific device and sensor, for example:

- AMD GPU / Edge Temperature / PCI address.
- NVIDIA GPU / GPU Temperature / PCI address.
- 10GbE NIC / PHY Temperature / PCI address.
- NVMe / Composite Temperature / serial number.

Connect a physical fan to the PCIe Fan header before configuring it, then verify
the sensor and PWM mapping in FanCtrl Plus 2. Different PCIe devices do not
share one fixed temperature-reading mechanism.

## Safety

- Do not force-load this module on other Minisforum models or mainboards.
- Do not install a `.ko` built for a different kernel.
- Confirm the physical fan-to-zone mapping before changing PWM values.
- Assign each `pwmN` to one fan-control plugin only.
- Module and service exit restore `pwmN_enable=2`.

## Building

Use kernel source and configuration matching the target Unraid release exactly:

```bash
make -C /path/to/linux M="$PWD" modules
modinfo minisforum_n5_it5571.ko
```

Verify that module `vermagic` matches the target `uname -r` before publishing.

## License

The driver source is intended for release under GPL-2.0-only. The repository
must not redistribute vendor EC firmware or the confidential IT5571 datasheet.

## Support

- GitHub Issues: `https://github.com/ltdstudio/minisforum-n5-it5571/issues`
- Unraid Support Thread: `https://github.com/ltdstudio/minisforum-n5-it5571/issues`

When reporting a problem, include the Unraid release, `uname -r`, BIOS version,
DMI board identity, `modinfo minisforum_n5_it5571`, and relevant `dmesg` output.
