# Community Applications Submission — Minisforum N5 EC / IT5571 Driver

Hello! I'd like to submit my plugin to Community Applications.

## Plugin details

| Field | Value |
|---|---|
| **Name** | Minisforum N5 EC / IT5571 Driver |
| **Category** | Drivers:System |
| **Author** | ltdstudio |
| **License** | GPL-2.0-only |
| **Min Unraid** | 7.3.2 (first validated release) |
| **Plugin URL** | `https://raw.githubusercontent.com/ltdstudio/minisforum-n5-it5571/main/minisforum-n5-it5571.plg` |
| **Source** | https://github.com/ltdstudio/minisforum-n5-it5571 |
| **Support thread** | (link to my support thread above) |
| **Icon** | https://raw.githubusercontent.com/ltdstudio/minisforum-n5-it5571/main/icons/minisforum-n5-it5571.png |

## Short description

Hardware-specific ITE IT5571 EC/hwmon driver for the Minisforum N5 F8NAA mainboard. Exposes four PWM controls (CPU/SSD/HDD/PCIe), three confirmed RPM inputs, and four EC temperatures (CPU/System/Board/Ambient). Recommended with FanCtrl Plus 2 for fan control and Dynamix System Temperature for sensor display.

## Validation

- Tested on Unraid 7.3.2 / 6.18.38-Unraid / BIOS 1.04 (Minisforum N5)
- Install, upgrade, uninstall, and reboot paths verified
- Fan channels restored to EC automatic mode on unload/removal (with duty-cycle clamping)
- DMI guard: refuses to load on non-N5/F8NAA systems
- Package ships as GitHub Releases asset, kernel-matched (`.txz` per `uname -r`)
- WebGUI page under Settings → User Utilities with live sensor data

## Notes

- The kernel module must match the running Unraid kernel exactly; a new package is required after each Unraid kernel update.
- Only N5/F8NAA is currently validated. Other IT5571 systems (Avalue EMX-EHLP, System76 Pangolin) are documented as potential porting targets only.
- PCIe Fan has no RPM feedback on this board (firmware does not expose a safe tach path); the driver reports zero instead of fabricating a value.
