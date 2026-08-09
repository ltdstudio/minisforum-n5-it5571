# Unraid Community Applications English Publication Copy

> Replace every bracketed placeholder and complete final package validation
> before publishing.

## Application name

```text
Minisforum N5 EC / IT5571 Driver
```

## Suggested category

```text
Drivers:System
```

## Short description

```text
Hardware-specific IT5571 EC/hwmon driver for the Minisforum N5 F8NAA mainboard. Exposes four PWM controls, three confirmed RPM inputs, and four EC temperatures. Recommended with FanCtrl Plus 2 for fan control and Dynamix System Temperature for sensor display.
```

## Full description

```text
This plugin installs a dedicated Linux hwmon driver for the Minisforum N5 F8NAA mainboard and its ITE IT5571 embedded controller.

Features:
• Four PWM outputs: CPU, SSD, HDD, and PCIe;
• Three confirmed RPM inputs: CPU, SSD, and HDD;
• Four temperatures: CPU, System, Board, and Ambient;
• Recommended FanCtrl Plus 2 integration for independent control from disk, NVMe, CPU, NVIDIA GPU, and other auxiliary temperatures;
• Recommended Dynamix System Temperature integration for CPU, System, Board, and Ambient display in the Unraid WebGUI/Dashboard;
• Dynamix Fan Auto Control compatibility.

PCIe PWM was physically validated with fan stop and full-speed tests. The board firmware does not provide a safe PCIe RPM feedback path, so the PCIe channel reports zero RPM instead of fabricating a value.

Recommended mapping: CPU temperature → CPU Fan; NVMe group → SSD Fan; array-disk group → HDD Fan Group; NVIDIA GPU or target PCIe auxiliary temperature → PCIe Fan. The NVIDIA Tesla P4 in the validation system exposes a usable GPU temperature. FanCtrl Plus 2 is installed separately. It must not run beside upstream FanCtrl Plus, and no two controllers should write the same PWM channel.

Recommended software roles: Minisforum N5 EC / IT5571 Driver provides the low-level hwmon nodes, FanCtrl Plus 2 performs automatic fan control, and Dynamix System Temperature handles temperature selection and WebGUI/Dashboard display.

Only DMI-matched Minisforum N5/F8NAA systems are supported. Publicly documented IT5571 systems such as the Avalue EMX-EHLP and System76 Pangolin (pang13) are potential porting targets, not currently compatible systems; each requires separate firmware-protocol and board-wiring validation. The kernel module must match the running Unraid kernel exactly; unsupported hardware or kernels are rejected.
```

## Installation warning

```text
This plugin directly controls embedded-controller fan outputs and is only for the Minisforum N5/F8NAA. Do not force-load it on other systems. Verify that a driver package exists for the running Unraid kernel. PCIe Fan has no RPM feedback; connect a physical fan before enabling PCIe temperature-linked control. Assign each PWM to only one of FanCtrl Plus 2, Dynamix Fan Auto Control, or another control service.
```

## Initial release changelog

```text
### 0.1.0

- Initial public release for Minisforum N5/F8NAA.
- Added four PWM channels: CPU, SSD, HDD, and PCIe.
- Added CPU, SSD, and HDD RPM monitoring.
- Added CPU, System, Board, and Ambient temperatures.
- Added EC automatic-mode restoration on unload.
- Documented recommended FanCtrl Plus 2 integration and four-zone mapping.
- Documented recommended Dynamix System Temperature integration.
- PCIe PWM validated on hardware; PCIe RPM is not available.
- Validated on Unraid 7.3.2 / 6.18.38-Unraid / BIOS 1.04.
```

## Support-thread title

```text
[Support] Minisforum N5 EC / IT5571 Driver - Fan PWM, RPM and Temperature Sensors
```

## Support-thread body

```text
This thread provides support for the Minisforum N5 EC / IT5571 Driver.

Supported hardware:
• Minisforum N5
• F8NAA mainboard
• BIOS 1.04 (validated)
• ITE IT5571 embedded controller

Features:
• CPU Fan: PWM + RPM
• SSD Fan: PWM + RPM
• HDD Fan Group: PWM + RPM
• PCIe Fan: PWM, no RPM feedback
• CPU/System/Board/Ambient temperatures

Recommended controller:
• FanCtrl Plus 2: https://github.com/andrebrait/fanctrlplus
• CPU Temp → CPU Fan
• NVMe temperature group → SSD Fan
• Array-disk temperature group → HDD Fan Group
• NVIDIA GPU/target PCIe-device temperature → PCIe Fan
• Do not run FanCtrl Plus, FanCtrl Plus 2, or another configuration against the same PWM concurrently

Recommended temperature display plugin:
• Dynamix System Temperature
• Select CPU Temp and System Temp; optionally display Board Temp and Ambient Temp
• `it87` and `nct6775` are unnecessary; select the EC temperature nodes supplied by this driver

PCIe Fan note:
A physical PWM fan was verified to stop at pwm4=0 and run at full speed at pwm4=255. The IT5571 has three tachometer counters with selectable A/B inputs, but this board firmware configures and exports only three A inputs. No safe host-accessible PCIe RPM path is available.

When requesting support, include:
1. Unraid release
2. uname -r
3. BIOS version
4. dmidecode -t system -t baseboard
5. modinfo minisforum_n5_it5571
6. sensors
7. dmesg | grep -i minisforum_n5

Source: https://github.com/ltdstudio/minisforum-n5-it5571
Issue tracker: https://github.com/ltdstudio/minisforum-n5-it5571/issues
```

## Suggested Community Applications metadata

| Field | Suggested value |
|---|---|
| Name | Minisforum N5 EC / IT5571 Driver |
| Repository | `https://raw.githubusercontent.com/ltdstudio/minisforum-n5-it5571/main/minisforum-n5-it5571.plg` |
| Support | `https://github.com/ltdstudio/minisforum-n5-it5571/issues` |
| Project | `https://github.com/ltdstudio/minisforum-n5-it5571` |
| Category | Drivers:System |
| License | GPL-2.0-only |
| Min Unraid | 7.3.2 for the first validated release |

## Manual publication checklist

- [ ] Rename all public module references to `minisforum_n5_it5571`.
- [ ] Remove passwords, IP addresses, SSH logs, and local filesystem paths.
- [ ] Do not publish EC firmware or the confidential IT5571 PDF.
- [ ] Add `LICENSE`, bilingual READMEs, source, and build instructions.
- [ ] Publish a separate driver package for every supported `uname -r`.
- [ ] Make the `.plg` verify downloaded assets with SHA-256 or MD5.
- [ ] Reject mismatched DMI identities and kernel releases.
- [ ] Test install, reboot, upgrade, uninstall, and reinstall on a clean setup.
- [ ] Restore EC automatic fan mode on every failure and uninstall path.
- [ ] Verify the raw `.plg` URL through Unraid's manual plugin installer.
- [ ] Create the Unraid support thread and add its URL to the `.plg`.
- [ ] Submit to Community Applications only after all checks pass.
