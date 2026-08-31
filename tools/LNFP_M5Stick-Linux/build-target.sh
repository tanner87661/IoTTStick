#!/usr/bin/env bash
# Shared compiler for cplus/build.sh and cplus2/build.sh.
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 {cplus|cplus2} [output-directory]" >&2
  exit 2
fi

TARGET="$1"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SKETCH_DIR="$ROOT_DIR/Sketchbook/LNFP_M5Stick"
ARDUINO_DATA_DIR="${ARDUINO_DATA_DIR:-$HOME/.arduino15}"
CORE_VERSION="${M5STACK_CORE_VERSION:-2.0.9}"
CORE_DIR="$ARDUINO_DATA_DIR/packages/m5stack/hardware/esp32/$CORE_VERSION"
MKSPIFFS="${MKSPIFFS:-$ARDUINO_DATA_DIR/packages/m5stack/tools/mkspiffs/0.2.3/mkspiffs}"
WORK_DIR="$(mktemp -d /tmp/iottstick-build.XXXXXX)"

cleanup() { rm -rf "$WORK_DIR"; }
trap cleanup EXIT

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

case "$TARGET" in
  cplus)
    FQBN="m5stack:esp32:stickc_plus:PartitionScheme=no_ota"
    PARTITION="no_ota"
    DEFINE="StickPlus"
    OUTPUT_DIR="${2:-/tmp/iottstick-lnfp-cplus}"
    ;;
  cplus2)
    PARTITION="${PLUS2_PARTITION:-default_8MB}"
    case "$PARTITION" in default_8MB|no_ota) ;; *) echo "PLUS2_PARTITION must be default_8MB or no_ota." >&2; exit 2 ;; esac
    FQBN="m5stack:esp32:stickc_plus2:PartitionScheme=$PARTITION"
    DEFINE="StickPlus2"
    OUTPUT_DIR="${2:-/tmp/iottstick-lnfp-cplus2}"
    ;;
  *)
    echo "Unknown target: $TARGET" >&2
    exit 2
    ;;
esac

PARTITION_FILE="$CORE_DIR/tools/partitions/$PARTITION.csv"
BOOT_APP0="$CORE_DIR/tools/partitions/boot_app0.bin"
for required_path in "$SKETCH_DIR" "$MKSPIFFS" "$PARTITION_FILE" "$BOOT_APP0"; do
  [[ -e "$required_path" ]] || { echo "Required path not found: $required_path" >&2; exit 1; }
done

read -r SPIFFS_OFFSET SPIFFS_SIZE < <(awk -F, '$1 ~ /^[[:space:]]*spiffs[[:space:]]*$/ {gsub(/[[:space:]]/, "", $4); gsub(/[[:space:]]/, "", $5); print $4, $5; exit}' "$PARTITION_FILE")
[[ -n "${SPIFFS_OFFSET:-}" && -n "${SPIFFS_SIZE:-}" ]] || { echo "SPIFFS partition not found in $PARTITION_FILE" >&2; exit 1; }

SOURCE_COPY="$WORK_DIR/LNFP_M5Stick"
mkdir -p "$SOURCE_COPY" "$OUTPUT_DIR"
cp -a "$SKETCH_DIR/." "$SOURCE_COPY/"
if [[ "$DEFINE" == "StickPlus" ]]; then
  sed -i -E 's@^//[[:space:]]*#define[[:space:]]+StickPlus[[:space:]].*@#define StickPlus@; s@^#define[[:space:]]+StickPlus2[[:space:]].*@//#define StickPlus2@' "$SOURCE_COPY/LNFP_M5Stick.ino"
else
  sed -i -E 's@^//[[:space:]]*#define[[:space:]]+StickPlus[[:space:]].*@//#define StickPlus@; s@^#define[[:space:]]+StickPlus2[[:space:]].*@#define StickPlus2@' "$SOURCE_COPY/LNFP_M5Stick.ino"
fi

"$ARDUINO_CLI" compile --fqbn "$FQBN" --libraries "$ROOT_DIR/Sketchbook/libraries" --output-dir "$OUTPUT_DIR" "$SOURCE_COPY"
"$MKSPIFFS" -c "$SOURCE_COPY/data" -s "$SPIFFS_SIZE" -p 256 -b 4096 "$OUTPUT_DIR/LNFP_M5Stick.spiffs.bin"
cp "$BOOT_APP0" "$OUTPUT_DIR/boot_app0.bin"

echo "Built $TARGET with $PARTITION (SPIFFS $SPIFFS_OFFSET) in: $OUTPUT_DIR"
