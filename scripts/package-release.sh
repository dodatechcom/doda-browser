#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
FIREFOX_DIR="$PROJECT_DIR/firefox-source"
VERSION="${1:-$(date +%Y%m%d)}"
OUTPUT_DIR="$PROJECT_DIR/releases/$VERSION"

if [ ! -f "$FIREFOX_DIR/obj-release/dist/bin/firefox" ] && [ ! -f "$FIREFOX_DIR/obj-release/dist/bin/firefox.exe" ]; then
    echo "ERROR: No built binary found at obj-release/dist/bin/"
    echo "Make sure the build completed successfully first."
    exit 1
fi

mkdir -p "$OUTPUT_DIR"
cd "$FIREFOX_DIR/obj-release/dist/bin"

echo "Packaging Doda Browser $VERSION"
echo "Output: $OUTPUT_DIR"

# Determine platform
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Linux)
        ARTIFACT="doda-browser-linux-${ARCH}.tar.bz2"
        echo "Creating $ARTIFACT ..."
        # Exclude large dev files
        tar cjf "$OUTPUT_DIR/$ARTIFACT" \
            --exclude="*.o" --exclude="*.a" \
            --exclude="*.la" --exclude="*.debug" \
            -C "$FIREFOX_DIR/obj-release/dist/bin" .
        ;;
    Darwin)
        ARTIFACT="doda-browser-macos-universal.tar.bz2"
        echo "Creating $ARTIFACT ..."
        tar cjf "$OUTPUT_DIR/$ARTIFACT" \
            --exclude="*.o" --exclude="*.a" \
            -C "$FIREFOX_DIR/obj-release/dist/bin" .
        ;;
    MINGW*|MSYS*)
        ARTIFACT="doda-browser-windows-${ARCH}.zip"
        echo "Creating $ARTIFACT ..."
        7z a "$OUTPUT_DIR/$ARTIFACT" . -x!*.o -x!*.a -x!*.lib
        ;;
    *)
        echo "Unknown OS: $OS"
        exit 1
        ;;
esac

# Checksum
echo "Generating checksums..."
cd "$OUTPUT_DIR"
if command -v sha256sum &>/dev/null; then
    sha256sum "$ARTIFACT" > "$ARTIFACT.sha256"
elif command -v shasum &>/dev/null; then
    shasum -a 256 "$ARTIFACT" > "$ARTIFACT.sha256"
fi

echo ""
echo "=== Package Complete ==="
echo "  $OUTPUT_DIR/$ARTIFACT"
echo "  $OUTPUT_DIR/$ARTIFACT.sha256"
echo "Size: $(du -h "$OUTPUT_DIR/$ARTIFACT" | cut -f1)"
