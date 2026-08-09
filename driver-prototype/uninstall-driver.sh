#!/bin/bash
set -euo pipefail

kernel_release=$(uname -r)
module_target="/lib/modules/${kernel_release}/extra/n5_ec_hwmon.ko"
drivers_file=/boot/config/plugins/dynamix.system.temp/drivers.conf
pcie_service=/boot/config/plugins/n5-ec-hwmon/n5-pcie-autofan

if [[ -f "$pcie_service" ]]; then
  /bin/bash "$pcie_service" stop || true
fi

if lsmod | awk '{print $1}' | grep -qx n5_ec_hwmon; then
  modprobe -r n5_ec_hwmon
fi

rm -f -- "$module_target"
depmod -a "$kernel_release"

if [[ -f "$drivers_file" ]]; then
  sed -i '/^n5_ec_hwmon$/d' "$drivers_file"
fi

echo "n5_ec_hwmon removed from the running system"
