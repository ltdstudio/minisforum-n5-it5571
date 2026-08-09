#!/bin/bash
# Minisforum N5 EC / IT5571 driver — legacy manual install helper.
# DISCLAIMER: independent community script, not affiliated with Minisforum.
# Contains unverified factors; use at your own risk. N5 / F8NAA only.
set -euo pipefail

plugin_dir=/boot/config/plugins/n5-ec-hwmon
kernel_release=$(uname -r)
module_source="$plugin_dir/minisforum_n5_it5571-${kernel_release}.ko"
module_dir="/lib/modules/${kernel_release}/extra"
module_target="$module_dir/minisforum_n5_it5571.ko"
drivers_file=/boot/config/plugins/dynamix.system.temp/drivers.conf

if [[ ! -f "$module_source" ]]; then
  echo "N5 driver does not support running kernel ${kernel_release}: $module_source is missing" >&2
  exit 1
fi

mkdir -p "$module_dir" "$(dirname "$drivers_file")"
install -m 0644 "$module_source" "$module_target"
depmod -a "$kernel_release"

touch "$drivers_file"
if ! grep -qxF minisforum_n5_it5571 "$drivers_file"; then
  printf '\nminisforum_n5_it5571\n' >> "$drivers_file"
  sed -i '/^$/d' "$drivers_file"
fi

if ! lsmod | awk '{print $1}' | grep -qx minisforum_n5_it5571; then
  modprobe minisforum_n5_it5571
fi

if [[ -f "$plugin_dir/pcie-autofan.conf" ]] &&
   grep -Eq '^[[:space:]]*ENABLE=1([[:space:]]*(#.*)?)?$' "$plugin_dir/pcie-autofan.conf"; then
  /bin/bash "$plugin_dir/n5-pcie-autofan" start
fi

echo "minisforum_n5_it5571 installed for ${kernel_release}"
