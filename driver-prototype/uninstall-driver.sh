#!/bin/bash
set -euo pipefail

kernel_release=$(uname -r)
module_target="/lib/modules/${kernel_release}/extra/minisforum_n5_it5571.ko"
drivers_file=/boot/config/plugins/dynamix.system.temp/drivers.conf
pcie_service=/boot/config/plugins/n5-ec-hwmon/n5-pcie-autofan

if [[ -f "$pcie_service" ]]; then
  /bin/bash "$pcie_service" stop || true
fi

if lsmod | awk '{print $1}' | grep -qx minisforum_n5_it5571; then
  modprobe -r minisforum_n5_it5571
fi

rm -f -- "$module_target"
depmod -a "$kernel_release"

if [[ -f "$drivers_file" ]]; then
  sed -i '/^minisforum_n5_it5571$/d' "$drivers_file"
fi

echo "minisforum_n5_it5571 removed from the running system"
