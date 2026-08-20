# [Support] Minisforum N5 EC / IT5571 Driver - Fan PWM, RPM and Temperature Sensors

This thread provides support for the **Minisforum N5 EC / IT5571 Driver**, a hardware-specific hwmon driver for the Minisforum N5 family and its ITE IT5571 embedded controller.

## Supported hardware

- Minisforum N5
- F8NAA mainboard
- BIOS 1.04 (validated)
- ITE IT5571 embedded controller

Experimental profiles (read-only by default; PWM requires explicit opt-in):

- Minisforum N5 Pro / `N5 PRO` / `F8NAA`
- Minisforum N5 Air / `N5A` or `N5 AIR` / `F8NAB`

Community report: one user reports good operation on a Minisforum MS-A2. This
is not maintainer validation and MS-A2 is not automatically whitelisted until
its exact DMI and channel mapping are collected.

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

- The module matches `uname -r` exactly and requires an exact recognized DMI pair.
- N5 Pro/Air initially expose temperature/RPM only. PWM needs `experimental_write=1`.
- On unload, only channels actually modified by the module are restored to EC/BIOS automatic mode.
- PCIe Fan was physically validated: pwm4=0 stops the fan, pwm4=255 reliably starts it at full speed. The IT5571 has three tachometer counters; this board firmware exposes only three RPM inputs.
- Stop immediately if a fan stops, runs unexpectedly, maps to the wrong header,
  disappears, or temperatures become abnormal. Unload the module and reboot to
  BIOS control if necessary.

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
