#!/usr/bin/env bash
# Knowledge Graph is now a native C++ component compiled into the browser.
# about:knowledgegraph is available in every build.
set -euo pipefail
cd "$(dirname "$0")/../firefox-source"
echo "Knowledge Graph is built-in. Run: ./mach run about:knowledgegraph"
exec ./mach run "$@"
