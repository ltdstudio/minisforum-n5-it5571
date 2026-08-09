#!/bin/bash
# build-txz.sh — assemble the Slackware package (.txz) for the plugin.
# .txz == tar.xz archive with the standard Slackware layout:
#   usr/local/emhttp/plugins/<plugin>/   WebGUI (page, php, scripts, icons)
#   lib/modules/<kver>/extra/            kernel module
#   install/slack-desc                   package description
# Built with plain tar+xz (no Slackware tools required on the build host).
#
# Usage: build-txz.sh <KVER> <KO_PATH> [OUTPUT_DIR]
set -euo pipefail

PLUGIN="minisforum-n5-it5571"
VERSION="0.1.0"
KVER="${1:?KVER required}"
KO="${2:?path to .ko required}"
OUTDIR="${3:-$(pwd)}"
SRC="$(cd "$(dirname "$0")/.." && pwd)"   # repo root
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# ── 1. WebGUI files ────────────────────────────────────────────────
mkdir -p "$WORK/usr/local/emhttp/plugins/$PLUGIN/scripts" \
         "$WORK/usr/local/emhttp/plugins/$PLUGIN/icons"
cp "$SRC/src/usr/local/emhttp/plugins/$PLUGIN/minisforum-n5-it5571.page" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/"
cp "$SRC/src/usr/local/emhttp/plugins/$PLUGIN/scripts/restore-bios.sh" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/scripts/"
cp "$SRC/icons/minisforum-n5-it5571.png" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/icons/"

# ── 2. Kernel module ───────────────────────────────────────────────
mkdir -p "$WORK/lib/modules/$KVER/extra"
install -m 0644 "$KO" "$WORK/lib/modules/$KVER/extra/minisforum_n5_it5571.ko"

# ── 3. Package metadata ────────────────────────────────────────────
mkdir -p "$WORK/install"
cp "$SRC/src/install/slack-desc" "$WORK/install/"

# ── 4. Permissions & assemble (tar.xz) ─────────────────────────────
chmod 755 "$WORK/usr/local/emhttp/plugins/$PLUGIN/scripts/restore-bios.sh"
find "$WORK" -type d -exec chmod 755 {} \;
find "$WORK" -type f -exec chmod 644 {} \;
chmod 755 "$WORK/usr/local/emhttp/plugins/$PLUGIN/scripts/restore-bios.sh"

mkdir -p "$OUTDIR"
PKG="$OUTDIR/${PLUGIN}-${VERSION}-x86_64-${KVER}.txz"
cd "$WORK"
tar --owner=root --group=root -cJf "$PKG" .
cd - > /dev/null
ls -la "$PKG"
echo "PKG_OK: $PKG"
