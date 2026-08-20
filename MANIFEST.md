# Archive manifest

## `driver-prototype/`

- `minisforum_n5_it5571.c`: Linux hwmon driver source; N5 validated, N5 Pro/Air experimental.
- `Makefile`: external kernel-module build file.
- `install-driver.sh`: prototype Unraid installation script.
- `uninstall-driver.sh`: prototype rollback script.
- `minisforum_n5_it5571-6.18.38-Unraid.ko`: x86-64 v0.2.0 module for exactly
  `6.18.38-Unraid`.
- `README.md`: original prototype package notes.

The public module and hwmon name is `minisforum_n5_it5571`.

## `research-tools/`

- `n5_fan.c`: PMC/EC fan telemetry reader.
- `n5_pwm_diag.c`: PWM mapping and manual/automatic-mode diagnostic.
- `n5_i2ec_read.c`: read-only IT5571 I2EC register diagnostic.
- `ec_probe.c`: ACPI-style EC RAM read/write diagnostic; writes are dangerous.
- `sio_probe.c`: IT5571 Super I/O configuration-space diagnostic.

Only source is included. Temporary standalone binaries are omitted because they
can be rebuilt and may contain build-environment debug paths.

## `build-scripts/`

- `build-n5-module.sh`: Alpine-based historical build procedure.
- `build-n5-module-debian.sh`: Debian-based historical build procedure.

These scripts document the prototype build process and expect build inputs
under `/work`. Release v0.2.0 was rebuilt against the prepared
`6.18.38-Unraid` kernel tree with GCC 14.2.0.

## `legacy-pcie-autofan/`

- `n5-pcie-autofan`: early standalone PCIe temperature/PWM controller.
- `pcie-autofan.conf`: disabled-by-default example configuration.

This is retained for development history. The public documentation recommends
FanCtrl Plus 2 for fan control instead.

## `docs/publication/`

Bilingual GitHub README drafts, reverse-engineering results, Unraid Community
Applications/support-thread copy, and the publication index.

## `docs/research/`

- `N5-EC-PROBE-NOTES.zh-CN.md`: EC discovery notes.
- `N5-FAN-RESEARCH.zh-CN.md`: early fan-interface research report.
- `N5-PWM-RPM-REVERSE-ENGINEERING.zh-CN.md`: detailed PWM/RPM reverse-engineering log.

Host-specific SSH details have been replaced with placeholders.

## `LICENSES/`

GPL-2.0 license material applicable to the prototype driver source. Individual
files retain their SPDX identifiers.

## Intentionally excluded

- Minisforum BIOS images and EC firmware binaries.
- Firmware disassemblies and other derived firmware dumps.
- The confidential ITE IT5571 datasheet and rendered pages.
- Linux source archives and extracted kernel trees.
- Zig/compiler caches, object files, temporary binaries, logs, and SSH records.
- Passwords, IP addresses, and local user paths.
- Third-party MCS51 disassembler copies; they are not required to build or use
  the driver and are not part of this project's authored source.
- Duplicate/stale publication-draft directories.
