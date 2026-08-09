# [Support] Minisforum N5 EC / IT5571 Driver - Fan PWM, RPM and Temperature Sensors

This thread provides support for the **Minisforum N5 EC / IT5571 Driver**, a hardware-specific hwmon driver for the Minisforum N5 (F8NAA mainboard, ITE IT5571 embedded controller).

## Supported hardware

- Minisforum N5
- F8NAA mainboard
- BIOS 1.04 (validated)
- ITE IT5571 embedded controller

## Features

- CPU Fan: PWM + RPM
- SSD Fan: PWM + RPM
- HDD Fan Group: PWM + RPM
- PCIe Fan: PWM (no RPM feedback on this board — zero is expected)
- CPU / System / Board / Ambient temperatures

## Installation

**Plugins → Install Plugin** and enter:

```
https://raw.githubusercontent.com/ltdstudio/minisforum-n5-it5571/main/minisforum-n5-it5571.plg
```

The plugin downloads the kernel-matched package from GitHub Releases (currently Unraid 7.3.2 / 6.18.38-Unraid), installs it, and loads the module. A WebGUI page under **Settings → User Utilities** shows live fan/temperature data (refreshes every 3 seconds).

## Recommended companion plugins

- **FanCtrl Plus 2** (fan control): CPU Temp → CPU Fan, NVMe group → SSD Fan, array-disk group → HDD Fan Group, NVIDIA GPU / PCIe auxiliary temp → PCIe Fan. Do not run FanCtrl Plus, FanCtrl Plus 2, or another controller against the same PWM concurrently.
- **Dynamix System Temperature** (temperature display): select CPU Temp and System Temp; optionally Board Temp and Ambient Temp.

## Notes

- The module matches `uname -r` exactly and refuses to load on non-N5/F8NAA systems (DMI check).
- On unload/removal, all fan channels are restored to EC/BIOS automatic mode (with duty-cycle clamping for safety).
- PCIe Fan was physically validated: pwm4=0 stops the fan, pwm4=255 reliably starts it at full speed. The IT5571 has three tachometer counters; this board firmware exposes only three RPM inputs.

## When requesting support, include

1. Unraid release
2. `uname -r`
3. BIOS version
4. `dmidecode -t system -t baseboard` (or `/sys/class/dmi/id/board_name`)
5. `modinfo minisforum_n5_it5571`
6. `sensors`
7. `dmesg | grep -i minisforum`

Source: https://github.com/ltdstudio/minisforum-n5-it5571
Issue tracker: https://github.com/ltdstudio/minisforum-n5-it5571/issues
