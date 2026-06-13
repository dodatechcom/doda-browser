#!/usr/bin/env bash
# AI Assistant is now a native C++ component. Use about:ai instead.
set -euo pipefail
cd "$(dirname "$0")/../firefox-source"
echo "AI Assistant is built-in. Run: ./mach run about:ai"
exec ./mach run "$@"
