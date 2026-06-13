#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/../firefox-source"
exec ./mach run --load-extension "$SCRIPT_DIR/../components/programmable/extension" "$@"
