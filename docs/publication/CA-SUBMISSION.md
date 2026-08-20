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

Hardware-specific ITE IT5571 EC/hwmon driver for the Minisforum N5 family. N5/F8NAA is validated; N5 Pro/F8NAA and N5 Air/F8NAB are opt-in experimental profiles. Exposes four PWM controls (CPU/SSD/HDD/PCIe), three confirmed RPM inputs, and four EC temperatures (CPU/System/Board/Ambient). Recommended with FanCtrl Plus 2 and Dynamix System Temperature.

## Validation

- Tested on Unraid 7.3.2 / 6.18.38-Unraid / BIOS 1.04 (Minisforum N5)
- Install, upgrade, uninstall, and reboot paths verified
- Fan channels restored to EC automatic mode on unload/removal (with duty-cycle clamping)
- DMI guard: exact N5/F8NAA, N5 PRO/F8NAA, or N5A/F8NAB pair required
- Experimental profiles load read-only unless `experimental_write=1` is explicitly set
- Package ships as GitHub Releases asset, kernel-matched (`.txz` per `uname -r`)
- WebGUI page under Settings → User Utilities with live sensor data

## Notes

- The kernel module must match the running Unraid kernel exactly; a new package is required after each Unraid kernel update.
- Only N5/F8NAA is currently validated. One MS-A2 user reports success, but MS-A2 remains community-reported and is not auto-whitelisted pending exact DMI/channel data.
- PCIe Fan has no RPM feedback on this board (firmware does not expose a safe tach path); the driver reports zero instead of fabricating a value.
