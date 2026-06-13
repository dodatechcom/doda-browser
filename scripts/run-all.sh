#!/usr/bin/env bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXTENSIONS="$SCRIPT_DIR/../components/programmable/extension"
cd "$SCRIPT_DIR/../firefox-source"
exec ./mach run --load-extension "$EXTENSIONS" "$@"
