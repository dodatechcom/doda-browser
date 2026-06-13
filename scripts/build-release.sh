#!/usr/bin/env bash
# Doda Release Script
# Usage: ./scripts/build-release.sh [version]
# Example: ./scripts/build-release.sh v1.0.0

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
FIREFOX_DIR="$PROJECT_DIR/firefox-source"
OUTPUT_DIR="$PROJECT_DIR/releases"
VERSION="${1:-v1.0.0}"
TIMESTAMP=$(date +%Y%m%d-%H%M%S)

echo "=== Doda Release $VERSION ==="
echo ""

# --- 1. Verify we're on the right branch ---
cd "$PROJECT_DIR"
BRANCH=$(git symbolic-ref --short HEAD 2>/dev/null || echo "unknown")
echo "Branch: $BRANCH"

# --- 2. Build Doda ---
echo ""
echo "--- Building Doda ---"
cd "$FIREFOX_DIR"
if [ -f obj-x86_64-pc-linux-gnu/dist/bin/firefox ]; then
    echo "Running incremental build..."
    ./mach build 2>&1 | tail -20
else
    echo "No existing build found. Running fresh build..."
    echo "WARNING: This will take 1-4 hours."
    ./mach build 2>&1
fi

# Verify binary
if [ ! -f obj-x86_64-pc-linux-gnu/dist/bin/firefox ]; then
    echo "ERROR: Build failed — no binary found."
    exit 1
fi
echo "Build OK: $(obj-x86_64-pc-linux-gnu/dist/bin/firefox --version 2>&1)"

# --- 3. Package extensions as .xpi ---
echo ""
echo "--- Packaging Extensions ---"
mkdir -p "$OUTPUT_DIR/$VERSION"

package_extension() {
    local name="$1"
    local dir="$2"
    local xpi="$OUTPUT_DIR/$VERSION/${name}-${VERSION}.xpi"

    echo "  Packaging $name..."
    cd "$dir"
    if [ -f manifest.json ]; then
        zip -rq "$xpi" . -x "*.git*" -x "*.DS_Store"
        echo "    → $xpi ($(du -h "$xpi" | cut -f1))"
    else
        echo "    WARNING: No manifest.json in $dir"
    fi
}

package_extension "programmable" "$PROJECT_DIR/components/programmable/extension"

# --- 4. Generate release manifest ---
echo ""
echo "--- Release Manifest ---"
MANIFEST="$OUTPUT_DIR/$VERSION/MANIFEST.txt"
cat > "$MANIFEST" <<MANIFEST_EOF
Doda Browser $VERSION
Built: $(date -u +"%Y-%m-%dT%H:%M:%SZ")
Firefox binary: $(obj-x86_64-pc-linux-gnu/dist/bin/firefox --version 2>&1)
Extensions:
  - programmable-$VERSION.xpi

Native components (built into binary):
  - AI Engine (about:ai)
  - Identity Wallet (about:identity)
  - Knowledge Graph (about:knowledgegraph)
  - Provenance (about:provenance)
MANIFEST_EOF
echo "  → $MANIFEST"

# --- 5. Create version tag ---
echo ""
echo "--- Tagging Release ---"
if git rev-parse "$VERSION" >/dev/null 2>&1; then
    echo "  Tag $VERSION already exists. Skipping."
else
    git tag -a "$VERSION" -m "Doda Browser $VERSION"
    echo "  Created tag: $VERSION"
fi

echo ""
echo "=== Release $VERSION Complete ==="
echo "  Output: $OUTPUT_DIR/$VERSION/"
echo ""
echo "To run this release locally:"
echo "  $FIREFOX_DIR/obj-x86_64-pc-linux-gnu/dist/bin/firefox --no-remote --new-instance"
echo ""
echo "To run with all extensions:"
echo "  $SCRIPT_DIR/run-all.sh"
