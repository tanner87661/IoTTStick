#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$SCRIPT_DIR/flash-target.sh" cplus all "${1:-/tmp/iottstick-lnfp-cplus}"
