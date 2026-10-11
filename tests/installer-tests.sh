#!/bin/bash
set -euo pipefail
TEST_ROOT=$(mktemp -d)
trap 'rm -rf "$TEST_ROOT"' EXIT
mkdir -p "$TEST_ROOT/bin"
export TEST_ROOT
export TEST_KVER="${1:-6.18.54-Unraid}"
cat > "$TEST_ROOT/bin/mock" <<'MOCK'
#!/bin/bash
case "$(basename "$0")" in
  uname) echo "$TEST_KVER" ;;
  wget)
    echo download >> "$CASE_DIR/downloads"
    [ "$SCENARIO" != offline ] || exit 1
    [ "$SCENARIO" != missing ] || exit 1
    case "${@: -1}" in
      *.sha256) if [ "$SCENARIO" = bad_download ]; then printf '%064d\n' 0 > "$3"; else cp "$TEST_ROOT/package.sha256" "$3"; fi ;;
      *) cp "$TEST_ROOT/package.txz" "$3" ;;
    esac ;;
  upgradepkg) [ "$SCENARIO" != install_fail ] ;;
  modinfo) if [ "$SCENARIO" = wrong_kernel ]; then echo '0.0.0-wrong SMP'; else echo "$TEST_KVER SMP"; fi ;;
  lsmod) echo 'Module Size Used by' ;;
  modprobe) [ "$SCENARIO" != load_fail ] ;;
  depmod|udevadm) exit 0 ;;
  *) exit 2 ;;
esac
MOCK
chmod +x "$TEST_ROOT/bin/mock"
for command in uname wget upgradepkg modinfo lsmod modprobe depmod udevadm; do ln -s mock "$TEST_ROOT/bin/$command"; done
export PATH="$TEST_ROOT/bin:$PATH"
printf 'test package\n' > "$TEST_ROOT/package.txz"
sha256sum "$TEST_ROOT/package.txz" > "$TEST_ROOT/package.sha256"
REPO_ROOT=$(cd "$(dirname "$0")/.." && pwd)
VERSION=$(php -r '$p=simplexml_load_file($argv[1]); if ($p===false) exit(1); echo (string)$p["version"];' "$REPO_ROOT/minisforum-n5-it5571.plg")
php -r '$p=simplexml_load_file($argv[1]); if ($p===false) exit(1); echo (string)$p->FILE[0]->INLINE;' "$REPO_ROOT/minisforum-n5-it5571.plg" > "$TEST_ROOT/template.sh"

for SCENARIO in offline corrupt_cache missing bad_download install_fail wrong_kernel load_fail; do
  export SCENARIO
  CASE_DIR="$TEST_ROOT/$SCENARIO"
  export CASE_DIR
  mkdir -p "$CASE_DIR/sys/class/dmi/id" "$CASE_DIR/boot/config/plugins/minisforum-n5-it5571" "$CASE_DIR/lib/modules/$TEST_KVER/extra"
  printf 'N5\n' > "$CASE_DIR/sys/class/dmi/id/product_name"
  printf 'F8NAA\n' > "$CASE_DIR/sys/class/dmi/id/board_name"
  CACHE="$CASE_DIR/boot/config/plugins/minisforum-n5-it5571/minisforum-n5-it5571-$VERSION-x86_64-$TEST_KVER.txz"
  if [ "$SCENARIO" != missing ] && [ "$SCENARIO" != bad_download ]; then
    cp "$TEST_ROOT/package.txz" "$CACHE"
    cp "$TEST_ROOT/package.sha256" "$CACHE.sha256"
  fi
  if [ "$SCENARIO" = corrupt_cache ]; then printf 'corrupt\n' > "$CACHE"; fi
  sed -e "s|/boot/|$CASE_DIR/boot/|g" -e "s|/sys/|$CASE_DIR/sys/|g" -e "s|/lib/modules/|$CASE_DIR/lib/modules/|g" "$TEST_ROOT/template.sh" > "$CASE_DIR/install.sh"
  result=0
  bash "$CASE_DIR/install.sh" > "$CASE_DIR/output" 2>&1 || result=$?
  case "$SCENARIO" in
    offline|corrupt_cache)
      [ "$result" -eq 0 ] || { cat "$CASE_DIR/output"; exit 1; }
      grep -q "v$VERSION installed" "$CASE_DIR/output"
      if [ "$SCENARIO" = offline ]; then [ ! -e "$CASE_DIR/downloads" ]; fi
      if [ "$SCENARIO" = corrupt_cache ]; then cmp "$CACHE" "$TEST_ROOT/package.txz"; fi ;;
    *)
      [ "$result" -ne 0 ] || { cat "$CASE_DIR/output"; echo "Unexpected success: $SCENARIO"; exit 1; }
      ! grep -q "v$VERSION installed" "$CASE_DIR/output" ;;
  esac
  echo "PASS $SCENARIO"
done
