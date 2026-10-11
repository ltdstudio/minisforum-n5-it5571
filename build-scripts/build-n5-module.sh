#!/bin/sh
set -eu

KVER="${1:?Usage: build-n5-module.sh KVER (inputs in /work)}"
LINUX_VERSION="${KVER%-Unraid}"
[ "${KVER}" != "${LINUX_VERSION}" ] || { echo 'Expected an Unraid kernel release'; exit 1; }

apk add --no-cache \
  bash bc binutils bison build-base elfutils-dev flex gawk kmod linux-headers openssl-dev perl wget xz

cd /work
[ -s "linux-${LINUX_VERSION}.tar.xz" ] || { echo 'Stage the verified kernel source archive in /work first'; exit 1; }
[ -s unraid.config ] || { echo 'Stage the target Unraid kernel config in /work first'; exit 1; }
[ -d "linux-${LINUX_VERSION}" ] || tar -xf "linux-${LINUX_VERSION}.tar.xz"

cd "/work/linux-${LINUX_VERSION}"
cp /work/unraid.config .config
make olddefconfig
make prepare modules_prepare
[ "$(make -s kernelrelease)" = "${KVER}" ] || { echo 'Kernel release mismatch'; exit 1; }
# Unraid does not ship Module.symvers; this is valid only with MODVERSIONS off.
if grep -q '^CONFIG_MODVERSIONS=y' .config && [ ! -s Module.symvers ]; then
  echo 'CONFIG_MODVERSIONS requires the matching full-build Module.symvers'; exit 1
fi
make M=/work/module clean
make KBUILD_MODPOST_WARN=1 M=/work/module modules

modinfo /work/module/minisforum_n5_it5571.ko
[ "$(modinfo -F vermagic /work/module/minisforum_n5_it5571.ko | cut -d ' ' -f 1)" = "${KVER}" ]
