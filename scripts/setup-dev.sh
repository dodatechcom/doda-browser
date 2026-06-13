#!/usr/bin/env bash
set -euo pipefail

echo "=== Doda Dev Setup ==="
echo ""

# Check prerequisites
command -v rustc >/dev/null 2>&1 || { echo "Need rustc"; exit 1; }
command -v cargo  >/dev/null 2>&1 || { echo "Need cargo"; exit 1; }
command -v hg     >/dev/null 2>&1 || { echo "Need mercurial"; exit 1; }

echo "✓ rustc $(rustc --version | cut -d' ' -f2)"
echo "✓ cargo $(cargo --version | cut -d' ' -f2)"
echo "✓ hg $(hg --version | head -1 | cut -d' ' -f3)"

echo ""
echo "Waiting for mozilla-central clone to finish..."
while [ ! -f "$HOME/projects/Doda/firefox-source/.hg/requires" ]; do
    sleep 10
done
echo "Repository store present."

# Wait for working directory checkout
while [ ! -f "$HOME/projects/Doda/firefox-source/mach" ]; do
    sleep 30
    echo "Still cloning... ($(du -sh $HOME/projects/Doda/firefox-source/.hg/ | cut -f1))"
done

echo "Clone complete!"
echo ""
echo "Next steps:"
echo "  1. cd $HOME/projects/Doda/firefox-source"
echo "  2. ./mach bootstrap"
echo "  3. ./mach build"
echo "  4. ./mach run"
