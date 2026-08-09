# Minisforum N5 / F8NAA Unraid hwmon driver

This package is built and validated for Unraid 7.3.2 with kernel
`6.18.38-Unraid`. It exposes the Minisforum N5 IT5571 fan controller and EC
temperatures through the standard Linux hwmon ABI.

## Channel map

| hwmon node | N5 function | Feedback |
|---|---|---|
| `pwm1`, `fan1_input` | CPU fan | TACH1 |
| `pwm2`, `fan2_input` | SSD fan | TACH2 |
| `pwm3`, `fan3_input` | HDD fan group | TACH3 |
| `pwm4`, `fan4_input` | PCIe fan | no tach; reads 0 RPM |
| `temp1_input` | CPU Temp | EC 0x09 |
| `temp2_input` | System Temp | EC 0x04 |
| `temp3_input` | Board Temp | EC 0x05 |
| `temp4_input` | Ambient Temp | EC 0x06 |

`pwmN_enable=1` selects manual control and `pwmN_enable=2` restores the native
EC/BIOS automatic curve. Module removal also restores all four channels to EC
automatic mode.

## Unraid integration

The deployed boot script loads the driver from
`/boot/config/plugins/n5-ec-hwmon/` and adds `minisforum_n5_it5571` to the Dynamix System
Temp available-driver list. Fan Auto Control discovers all four `pwmN` nodes.
The PCIe channel deliberately exports `fan4_input=0` because the board firmware
has no fourth tachometer signal.

System Temp can select `CPU Temp`, `System Temp`, `Board Temp`, or `Ambient Temp`.
For CPU display, the existing `k10temp` sensor remains the most precise source;
the N5 EC CPU reading is also available.

## Optional PCIe temperature linkage

`pcie-autofan.conf` is installed with `ENABLE=0`, because no PCIe fan is
currently connected. After connecting one, set `TEMP_HWMON_NAME` and
`TEMP_CHANNEL` for the PCIe device probe, test the values, then set `ENABLE=1`.
The controller maps `TEMP_LOW..TEMP_HIGH` to `PWM_LOW..PWM_HIGH`; if the source
disappears it forces full speed. Stop it with:

```bash
/bin/bash /boot/config/plugins/n5-ec-hwmon/n5-pcie-autofan stop
```

## Kernel upgrades and rollback

The `.ko` is kernel-specific. After an Unraid kernel update it must be rebuilt
for the new `uname -r`; the installer intentionally refuses a mismatched module.
Rollback is:

```bash
/bin/bash /boot/config/plugins/n5-ec-hwmon/uninstall-driver.sh
```
