#!/usr/bin/env bash
# Build Windows, macOS, and Linux installer archives matching the established release layout.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SKETCH_DIR="$ROOT_DIR/Sketchbook/LNFP_M5Stick"
TEMPLATE_DIR="$ROOT_DIR/tools/LNFP_M5Stick-Linux/package-template"
ARDUINO_DATA_DIR="${ARDUINO_DATA_DIR:-$HOME/.arduino15}"
CORE_VERSION="${M5STACK_CORE_VERSION:-2.0.9}"
CORE_DIR="$ARDUINO_DATA_DIR/packages/m5stack/hardware/esp32/$CORE_VERSION"
MKSPIFFS="$ARDUINO_DATA_DIR/packages/m5stack/tools/mkspiffs/0.2.3/mkspiffs"
PLUS2_PARTITION="${PLUS2_PARTITION:-default_8MB}"
VERSION="${VERSION:-$(sed -n 's/^String BBVersion = "\([^"]*\)";.*/\1/p' "$SKETCH_DIR/LNFP_M5Stick.ino")}"
DIST_DIR="${1:-/tmp/IoTTStickM5-V$VERSION}"
INSTALL_DIR="$DIST_DIR/InstallFile"
WORK_DIR="$(mktemp -d /tmp/iottstick-package.XXXXXX)"

cleanup() { rm -rf "$WORK_DIR"; }
trap cleanup EXIT

if [[ -z "$VERSION" ]]; then
  echo "Could not determine firmware version from LNFP_M5Stick.ino" >&2
  exit 1
fi

if [[ -n "${ARDUINO_CLI:-}" ]]; then
  ARDUINO_CLI="$ARDUINO_CLI"
elif command -v arduino-cli >/dev/null 2>&1; then
  ARDUINO_CLI="arduino-cli"
else
  ARDUINO_CLI="$HOME/bin/arduino-cli"
fi

if [[ "$ARDUINO_CLI" == */* ]]; then
  [[ -x "$ARDUINO_CLI" ]] || { echo "Arduino CLI not found: $ARDUINO_CLI" >&2; exit 1; }
else
  command -v "$ARDUINO_CLI" >/dev/null 2>&1 || { echo "Arduino CLI command not found: $ARDUINO_CLI" >&2; exit 1; }
fi

for required_path in "$CORE_DIR" "$MKSPIFFS" "$TEMPLATE_DIR/windows/esptool.exe" "$TEMPLATE_DIR/macos/esptool.py"; do
  [[ -e "$required_path" ]] || { echo "Required path not found: $required_path" >&2; exit 1; }
done

case "$PLUS2_PARTITION" in
  default_8MB|no_ota) ;;
  *) echo "PLUS2_PARTITION must be default_8MB (historical V1.6.7) or no_ota (verified Plus2 configuration)." >&2; exit 2 ;;
esac

mkdir -p "$INSTALL_DIR"

make_zip() {
  local stage="$1" archive="$2"
  (cd "$stage" && python3 -m zipfile -c "$archive" ./* >/dev/null)
}

write_windows_updater() {
  local destination="$1" spiffs_offset="$2"
  printf '%s\n' \
    'esptool --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 LNFP_M5Stick.ino.bootloader.bin 0x10000 LNFP_M5Stick.ino.bin 0x8000 LNFP_M5Stick.ino.partitions.bin' \
    "esptool write_flash $spiffs_offset LNFP_M5Stick.spiffs.bin" > "$destination"
}

write_posix_updater() {
  local destination="$1" spiffs_offset="$2" port_line="$3"
  cat > "$destination" <<EOF
#!/usr/bin/env sh
set -eu
$port_line
./esptool.py \${PORT_ARG:-} --chip esp32 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 boot_app0.bin 0x1000 LNFP_M5Stick.ino.bootloader.bin 0x10000 LNFP_M5Stick.ino.bin 0x8000 LNFP_M5Stick.ino.partitions.bin
./esptool.py \${PORT_ARG:-} write_flash $spiffs_offset LNFP_M5Stick.spiffs.bin
EOF
  chmod 755 "$destination"
}

build_target() {
  local target="$1" fqbn="$2" source_define="$3" partition="$4"
  local source_copy="$WORK_DIR/$target/LNFP_M5Stick" build_dir="$WORK_DIR/$target/build"
  local partition_file="$CORE_DIR/tools/partitions/$partition.csv"
  local spiffs_offset spiffs_size

  [[ -f "$partition_file" ]] || { echo "Partition table not found: $partition_file" >&2; exit 1; }
  read -r spiffs_offset spiffs_size < <(awk -F, '$1 ~ /^[[:space:]]*spiffs[[:space:]]*$/ {gsub(/[[:space:]]/, "", $4); gsub(/[[:space:]]/, "", $5); print $4, $5; exit}' "$partition_file")
  [[ -n "${spiffs_offset:-}" && -n "${spiffs_size:-}" ]] || { echo "SPIFFS partition not found in $partition_file" >&2; exit 1; }

  mkdir -p "$source_copy" "$build_dir"
  cp -a "$SKETCH_DIR/." "$source_copy/"
  if [[ "$source_define" == "StickPlus" ]]; then
    sed -i -E 's@^//[[:space:]]*#define[[:space:]]+StickPlus[[:space:]].*@#define StickPlus@; s@^#define[[:space:]]+StickPlus2[[:space:]].*@//#define StickPlus2@' "$source_copy/LNFP_M5Stick.ino"
  else
    sed -i -E 's@^//[[:space:]]*#define[[:space:]]+StickPlus[[:space:]].*@//#define StickPlus@; s@^#define[[:space:]]+StickPlus2[[:space:]].*@#define StickPlus2@' "$source_copy/LNFP_M5Stick.ino"
  fi

  "$ARDUINO_CLI" compile --fqbn "$fqbn" --libraries "$ROOT_DIR/Sketchbook/libraries" --output-dir "$build_dir" "$source_copy" >&2
  "$MKSPIFFS" -c "$source_copy/data" -s "$spiffs_size" -p 256 -b 4096 "$build_dir/LNFP_M5Stick.spiffs.bin" >&2
  cp "$CORE_DIR/tools/partitions/boot_app0.bin" "$build_dir/boot_app0.bin"
  printf '%s %s\n' "$build_dir" "$spiffs_offset"
}

read -r PLUS_BUILD PLUS_SPIFFS_OFFSET < <(build_target "plus" "m5stack:esp32:stickc_plus:PartitionScheme=no_ota" "StickPlus" "no_ota")
read -r PLUS2_BUILD PLUS2_SPIFFS_OFFSET < <(build_target "plus2" "m5stack:esp32:stickc_plus2:PartitionScheme=$PLUS2_PARTITION" "StickPlus2" "$PLUS2_PARTITION")

package_target() {
  local label="$1" build_dir="$2" spiffs_offset="$3" windows_zip="$4" mac_zip="$5" linux_zip="$6"
  local windows_stage="$WORK_DIR/$label/windows" mac_stage="$WORK_DIR/$label/macos" linux_stage="$WORK_DIR/$label/linux"
  local artifact
  mkdir -p "$windows_stage" "$mac_stage" "$linux_stage"
  for artifact in boot_app0.bin LNFP_M5Stick.ino.bootloader.bin LNFP_M5Stick.ino.bin LNFP_M5Stick.ino.partitions.bin LNFP_M5Stick.spiffs.bin; do
    cp "$build_dir/$artifact" "$windows_stage/"
    cp "$build_dir/$artifact" "$mac_stage/"
    cp "$build_dir/$artifact" "$linux_stage/"
  done
  cp "$TEMPLATE_DIR/windows/esptool.exe" "$windows_stage/esptool.exe"
  cp "$TEMPLATE_DIR/macos/esptool.py" "$mac_stage/esptool.py"
  cp "$TEMPLATE_DIR/macos/esptool.py" "$linux_stage/esptool.py"
  write_windows_updater "$windows_stage/update.bat" "$spiffs_offset"
  write_posix_updater "$mac_stage/update.mac" "$spiffs_offset" ''
  write_posix_updater "$linux_stage/update.sh" "$spiffs_offset" 'PORT_ARG="--port ${PORT:-/dev/ttyACM0}"'
  make_zip "$windows_stage" "$INSTALL_DIR/$windows_zip"
  make_zip "$mac_stage" "$INSTALL_DIR/$mac_zip"
  make_zip "$linux_stage" "$INSTALL_DIR/$linux_zip"
}

package_target "plus" "$PLUS_BUILD" "$PLUS_SPIFFS_OFFSET" "M5Update.zip" "M5UpdateMacOs.zip" "M5UpdateLinux.zip"
package_target "plus2" "$PLUS2_BUILD" "$PLUS2_SPIFFS_OFFSET" "M5UpdatePlus2.zip" "M5UpdatePlus2MacOs.zip" "M5UpdatePlus2Linux.zip"

cat > "$INSTALL_DIR/Readme.txt" <<EOF
IoTT Stick software version $VERSION

Windows:
  M5Update.zip is for IoTT Stick Plus.
  M5UpdatePlus2.zip is for IoTT Stick Plus2.
  Extract the archive and run update.bat. Edit update.bat to add --port COMx if automatic port detection fails.

Linux:
  M5UpdateLinux.zip is for IoTT Stick Plus.
  M5UpdatePlus2Linux.zip is for IoTT Stick Plus2.
  Extract the archive, then run: PORT=/dev/ttyACM0 ./update.sh

Each updater flashes firmware and SPIFFS and overwrites SPIFFS configuration files. Back up configuration before updating.
EOF

cat > "$INSTALL_DIR/MacReadMe.txt" <<EOF
IoTT Stick software version $VERSION

Use M5UpdateMacOs.zip for IoTT Stick Plus or M5UpdatePlus2MacOs.zip for IoTT Stick Plus2.
Extract the archive, ensure Python 3 is available, then run: sh update.mac

The archive contains esptool.py and all required binary images. The updater overwrites SPIFFS configuration files; back up configuration before updating.
EOF

cat > "$INSTALL_DIR/BUILD-MANIFEST.txt" <<EOF
Generated by tools/LNFP_M5Stick-Linux/package-release.sh
Firmware version: $VERSION
M5Stack core: $CORE_VERSION
Plus partition: no_ota (SPIFFS at $PLUS_SPIFFS_OFFSET)
Plus2 partition: $PLUS2_PARTITION (SPIFFS at $PLUS2_SPIFFS_OFFSET)
EOF

echo "Distribution written to: $INSTALL_DIR"
