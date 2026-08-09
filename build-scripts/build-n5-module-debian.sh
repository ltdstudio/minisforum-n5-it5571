#!/bin/bash
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends \
  bc bison flex libelf-dev libssl-dev xz-utils

cd /work
if [ -d linux-6.18.38 ]; then
  rm -rf -- /work/linux-6.18.38
fi
tar -xf linux-6.18.38.tar.xz

cd /work/linux-6.18.38
cp /work/unraid.config .config
make olddefconfig
make prepare modules_prepare
make M=/work/module clean
make M=/work/module modules

modinfo /work/module/n5_ec_hwmon.ko || true
