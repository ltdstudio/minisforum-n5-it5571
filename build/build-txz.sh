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
VERSION="0.2.2"
KVER="${1:?KVER required}"
KO="${2:?path to .ko required}"
[ -s "$KO" ] || { echo "Module missing or empty: $KO" >&2; exit 1; }
if command -v modinfo >/dev/null 2>&1; then
  [ "$(modinfo -F vermagic "$KO" | cut -d ' ' -f 1)" = "$KVER" ] || {
    echo "Module vermagic does not match $KVER" >&2; exit 1;
  }
fi
OUTDIR="${3:-$(pwd)}"
SRC="$(cd "$(dirname "$0")/.." && pwd)"   # repo root
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# Keep the output path valid after the script changes into the staging tree.
mkdir -p "$OUTDIR"
OUTDIR="$(cd "$OUTDIR" && pwd)"

# ── 1. WebGUI files ────────────────────────────────────────────────
mkdir -p "$WORK/usr/local/emhttp/plugins/$PLUGIN/scripts" \
         "$WORK/usr/local/emhttp/plugins/$PLUGIN/icons" \
         "$WORK/usr/local/emhttp/plugins/$PLUGIN/images"
cp "$SRC/src/usr/local/emhttp/plugins/$PLUGIN/minisforum-n5-it5571.page" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/"
cp "$SRC/src/usr/local/emhttp/plugins/$PLUGIN/minisforum-n5-it5571.php" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/"
cp "$SRC/src/usr/local/emhttp/plugins/$PLUGIN/scripts/restore-bios.sh" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/scripts/"
# icons/  -> sidebar menu (tab_title); images/ -> Settings panel (MainContent)
cp "$SRC/icons/minisforum-n5-it5571.png" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/icons/"
cp "$SRC/icons/minisforum-n5-it5571.png" \
   "$WORK/usr/local/emhttp/plugins/$PLUGIN/images/"

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

PKG="$OUTDIR/${PLUGIN}-${VERSION}-x86_64-${KVER}.txz"
cd "$WORK"
# COPYFILE_DISABLE=1: 防止 macOS tar 写入 AppleDouble (._*) 垃圾条目
# bsdtar's --owner/--group set names only; Slackware needs numeric root IDs too.
case "$(tar --version)" in
  *bsdtar*) TAR_OWNER=(--uid=0 --gid=0 --uname=root --gname=root) ;;
  *) TAR_OWNER=(--owner=root --group=root) ;;
esac
COPYFILE_DISABLE=1 tar --no-xattrs "${TAR_OWNER[@]}" -cJf "$PKG" .
if command -v sha256sum >/dev/null 2>&1; then
  (cd "$OUTDIR" && sha256sum "$(basename "$PKG")" > "$(basename "$PKG").sha256")
else
  (cd "$OUTDIR" && shasum -a 256 "$(basename "$PKG")" > "$(basename "$PKG").sha256")
fi
cd - > /dev/null
ls -la "$PKG"
echo "PKG_OK: $PKG"
