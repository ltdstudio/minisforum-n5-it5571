#!/bin/bash
# restore-bios.sh — restore all N5 fan channels to EC/BIOS automatic mode.
# Safety net called by the plugin removal script before unloading minisforum_n5_it5571.
set -u

for hw in /sys/class/hwmon/hwmon*; do
  [ -d "$hw" ] || continue
  name=$(cat "$hw/name" 2>/dev/null || true)
  [ "$name" = "minisforum_n5_it5571" ] || continue

  for pwm in "$hw"/pwm[0-9]*; do
    [ -e "$pwm" ] || continue
    ch=$(basename "$pwm" | sed 's/pwm//')
    enable="${hw}/pwm${ch}_enable"
    [ -e "$enable" ] || continue

    # Clamp extreme duty cycles before handing back to EC/BIOS
    cur=$(cat "$pwm" 2>/dev/null || echo 0)
    if [ "$cur" -lt 85 ] || [ "$cur" -gt 160 ]; then
      echo 85 > "$pwm" 2>/dev/null
    fi
    echo 2 > "$enable" 2>/dev/null
  done
done

echo "minisforum_n5_it5571: fan channels restored to EC/BIOS automatic mode"
