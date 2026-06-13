#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
FIREFOX_DIR="$PROJECT_DIR/firefox-source"
OVERLAY_DIR="$PROJECT_DIR/source"
MOZCONFIG="$PROJECT_DIR/mozconfig"

echo "=== Doda Bootstrap ==="

# 1. Clone or update mozilla-central
if [ ! -d "$FIREFOX_DIR/.hg" ]; then
    echo "Cloning mozilla-central (stream)..."
    hg clone --stream https://hg.mozilla.org/mozilla-central "$FIREFOX_DIR"
    echo "Checking out central..."
    cd "$FIREFOX_DIR"
    hg checkout central
else
    echo "mozilla-central already cloned at $FIREFOX_DIR"
    cd "$FIREFOX_DIR"
    echo "Pulling latest..."
    hg pull --stream
    hg update central --clean
fi

# 2. Copy Doda overlay on top of mozilla-central
echo "Applying Doda source overlay..."
cd "$FIREFOX_DIR"
cp -a "$OVERLAY_DIR"/* .

# 3. Copy mozconfig
echo "Setting up mozconfig..."
cp "$MOZCONFIG" "$FIREFOX_DIR/mozconfig"

# 4. Install build dependencies
echo "Running mach bootstrap..."
./mach bootstrap

# 5. Build
echo "Building Doda..."
./mach build

echo ""
echo "=== Done ==="
echo "Run: cd $FIREFOX_DIR && ./mach run"
echo "For help: AGENTS.md"
