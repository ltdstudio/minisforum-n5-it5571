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
Hardware-specific IT5571 EC/hwmon driver for the Minisforum N5 family. N5/F8NAA is validated; N5 Pro/F8NAA and N5 Air/F8NAB are experimental and read-only by default. Exposes four PWM controls, three confirmed RPM inputs, and four EC temperatures. Recommended with FanCtrl Plus 2 and Dynamix System Temperature.
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

Recognized profiles are N5/F8NAA (validated), N5 PRO/F8NAA (experimental), and N5A or N5 AIR/F8NAB (experimental). Experimental profiles expose temperature/RPM only until `experimental_write=1` is explicitly enabled. One user reports good operation on an MS-A2, but that model is not auto-whitelisted until exact DMI and channel-mapping data are collected. The kernel module must match the running Unraid kernel exactly.
```

## Installation warning

```text
This plugin directly controls embedded-controller fan outputs. N5 Pro and N5 Air support is experimental. Stop immediately if a fan stops, runs unexpectedly, controls the wrong header, disappears, or temperatures become abnormal; unload the module and return to BIOS control. Never leave an experimental test unattended. Verify the kernel package and assign each PWM to only one controller.
```

## Initial release changelog

```text
### 0.2.0

- Added experimental N5 Pro/F8NAA and N5 Air/F8NAB profiles.
- Experimental profiles start read-only; PWM writes require explicit opt-in.
- Added immediate-stop guidance and touched-channel-only restore.
- Documented positive MS-A2 community feedback without auto-whitelisting it.

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
