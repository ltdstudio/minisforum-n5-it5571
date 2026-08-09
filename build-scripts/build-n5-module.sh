#!/bin/sh
set -eu

apk add --no-cache \
  bash bc binutils bison build-base elfutils-dev flex openssl-dev perl wget xz

cd /work
if [ ! -f linux-6.18.38.tar.xz ]; then
  wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.18.38.tar.xz
fi
if [ ! -d linux-6.18.38 ]; then
  tar -xf linux-6.18.38.tar.xz
fi

cd /work/linux-6.18.38
cp /work/unraid.config .config
make olddefconfig
make prepare modules_prepare
make M=/work/module modules

modinfo /work/module/minisforum_n5_it5571.ko || true
