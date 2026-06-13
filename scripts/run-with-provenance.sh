#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/../firefox-source"
echo "Provenance is now a native C++ component (compiled into libxul.so)"
echo "Open about:provenance to use it."
exec ./mach run "$@"
