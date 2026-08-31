#!/usr/bin/env bash
# Shared flasher for the target-specific scripts in cplus/ and cplus2/.
set -euo pipefail

if [[ $# -ne 3 ]]; then
  echo "Usage: $0 {cplus|cplus2} {firmware|spiffs|all} /path/to/build-output" >&2
  exit 2
fi

TARGET="$1"
OPERATION="$2"
ARTIFACT_DIR="$3"
ARDUINO_DATA_DIR="${ARDUINO_DATA_DIR:-$HOME/.arduino15}"
CORE_VERSION="${M5STACK_CORE_VERSION:-2.0.9}"
CORE_DIR="$ARDUINO_DATA_DIR/packages/m5stack/hardware/esp32/$CORE_VERSION"
ESPTOOL="${ESPTOOL:-$ARDUINO_DATA_DIR/packages/m5stack/tools/esptool_py/4.5.1/esptool.py}"
PORT="${PORT:-/dev/ttyACM0}"

case "$TARGET" in
  cplus)
    PARTITION="no_ota"
    ;;
  cplus2)
    PARTITION="${PLUS2_PARTITION:-default_8MB}"
    case "$PARTITION" in default_8MB|no_ota) ;; *) echo "PLUS2_PARTITION must be default_8MB or no_ota." >&2; exit 2 ;; esac
    ;;
  *)
    echo "Unknown target: $TARGET" >&2
    exit 2
    ;;
esac

case "$OPERATION" in firmware|spiffs|all) ;; *) echo "Unknown operation: $OPERATION" >&2; exit 2 ;; esac

PARTITION_FILE="$CORE_DIR/tools/partitions/$PARTITION.csv"
[[ -f "$PARTITION_FILE" ]] || { echo "Partition table not found: $PARTITION_FILE" >&2; exit 1; }
SPIFFS_OFFSET="$(awk -F, '$1 ~ /^[[:space:]]*spiffs[[:space:]]*$/ {gsub(/[[:space:]]/, "", $4); print $4; exit}' "$PARTITION_FILE")"
[[ -n "$SPIFFS_OFFSET" ]] || { echo "SPIFFS partition not found in $PARTITION_FILE" >&2; exit 1; }
[[ -f "$ESPTOOL" ]] || { echo "esptool.py not found: $ESPTOOL" >&2; exit 1; }

require_file() {
  [[ -f "$ARTIFACT_DIR/$1" ]] || { echo "Required artifact not found: $ARTIFACT_DIR/$1" >&2; exit 1; }
}

flash_firmware() {
  require_file "LNFP_M5Stick.ino.bin"
  python3 "$ESPTOOL" --chip esp32 --port "$PORT" --baud 1500000 --before default_reset --after hard_reset \
    write_flash -z 0x10000 "$ARTIFACT_DIR/LNFP_M5Stick.ino.bin"
}

flash_spiffs() {
  require_file "LNFP_M5Stick.spiffs.bin"
  python3 "$ESPTOOL" --chip esp32 --port "$PORT" --baud 1500000 --before default_reset --after hard_reset \
    write_flash -z "$SPIFFS_OFFSET" "$ARTIFACT_DIR/LNFP_M5Stick.spiffs.bin"
}

flash_all() {
  for file in boot_app0.bin LNFP_M5Stick.ino.bootloader.bin LNFP_M5Stick.ino.bin LNFP_M5Stick.ino.partitions.bin; do
    require_file "$file"
  done
  python3 "$ESPTOOL" --chip esp32 --port "$PORT" --baud 1500000 --before default_reset --after hard_reset \
    write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect \
    0xe000 "$ARTIFACT_DIR/boot_app0.bin" \
    0x1000 "$ARTIFACT_DIR/LNFP_M5Stick.ino.bootloader.bin" \
    0x10000 "$ARTIFACT_DIR/LNFP_M5Stick.ino.bin" \
    0x8000 "$ARTIFACT_DIR/LNFP_M5Stick.ino.partitions.bin"
  flash_spiffs
}

case "$OPERATION" in
  firmware)
    read -r -p "Flash firmware only to $PORT? Type FLASH FIRMWARE to continue: " confirm
    [[ "$confirm" == "FLASH FIRMWARE" ]] || { echo "Cancelled."; exit 0; }
    flash_firmware
    ;;
  spiffs)
    read -r -p "Flash SPIFFS to $PORT at $SPIFFS_OFFSET? This replaces SPIFFS configuration and web files. Type FLASH SPIFFS to continue: " confirm
    [[ "$confirm" == "FLASH SPIFFS" ]] || { echo "Cancelled."; exit 0; }
    flash_spiffs
    ;;
  all)
    read -r -p "Flash bootloader, partition table, firmware, and SPIFFS to $PORT? Type FLASH ALL to continue: " confirm
    [[ "$confirm" == "FLASH ALL" ]] || { echo "Cancelled."; exit 0; }
    flash_all
    ;;
esac
