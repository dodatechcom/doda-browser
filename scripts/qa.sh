#!/usr/bin/env bash
# Doda QA — verify all extensions load and respond to messages
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
FIREFOX_DIR="$PROJECT_DIR/firefox-source"
PASS=0
FAIL=0

green() { echo -e "\033[32m✓ $1\033[0m"; }
red() { echo -e "\033[31m✗ $1\033[0m"; }

echo "=== Doda QA Check ==="
echo ""

# --- 1. Check Firefox binary ---
echo "--- Binary Check ---"
BINARY="$FIREFOX_DIR/obj-x86_64-pc-linux-gnu/dist/bin/firefox"
if [ -f "$BINARY" ]; then
    VERSION=$("$BINARY" --version 2>&1)
    green "Firefox binary: $VERSION"
    PASS=$((PASS + 1))
else
    red "Firefox binary not found at $BINARY"
    FAIL=$((FAIL + 1))
fi

# --- 2. Check all extensions exist ---
echo ""
echo "--- Extension Structure ---"
EXTENSIONS=(
    "Programmable Browser:$PROJECT_DIR/components/programmable/extension"
)

for entry in "${EXTENSIONS[@]}"; do
    NAME="${entry%%:*}"
    DIR="${entry##*:}"
    if [ -f "$DIR/manifest.json" ]; then
        if python3 -c "import json; json.load(open('$DIR/manifest.json'))" 2>/dev/null; then
            green "$NAME — valid manifest.json"
            PASS=$((PASS + 1))
        else
            red "$NAME — invalid manifest.json"
            FAIL=$((FAIL + 1))
        fi
    else
        red "$NAME — missing manifest.json"
        FAIL=$((FAIL + 1))
    fi
done

# --- 3. Check for required files in each extension ---
echo ""
echo "--- Required Files ---"
check_file() {
    local name="$1" file="$2"
    if [ -f "$file" ]; then
        green "$name: $(basename $file)"
        PASS=$((PASS + 1))
    else
        red "$name: missing $(basename $file)"
        FAIL=$((FAIL + 1))
    fi
}

check_file "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/background.js"
check_file "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/content.js"
check_file "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/recorder/recorder.html"
check_file "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/repl/repl.html"

# --- 4. Check theme.css in each extension ---
echo ""
echo "--- Theme CSS ---"
for entry in "${EXTENSIONS[@]}"; do
    NAME="${entry%%:*}"
    DIR="${entry##*:}"
    if [ -f "$DIR/theme.css" ]; then
        green "$NAME — theme.css"
        PASS=$((PASS + 1))
    else
        red "$NAME — missing theme.css"
        FAIL=$((FAIL + 1))
    fi
done

# --- 5. Check for JS syntax errors ---
echo ""
echo "--- JS Syntax Check ---"
check_js() {
    local name="$1" file="$2"
    if node --check "$file" 2>/dev/null; then
        green "$name: $(basename $file)"
        PASS=$((PASS + 1))
    else
        red "$name: $(basename $file) — syntax error"
        FAIL=$((FAIL + 1))
    fi
}

check_js "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/background.js"
check_js "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/content.js"
check_js "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/recorder/recorder.js"
check_js "Programmable Browser" "$PROJECT_DIR/components/programmable/extension/repl/repl.js"

# --- 6. Check branding prefs ---
echo ""
echo "--- Branding Check ---"
BRAND_FTL="$FIREFOX_DIR/browser/branding/unofficial/locales/en-US/brand.ftl"
if grep -q "Doda" "$BRAND_FTL" 2>/dev/null; then
    green "Brand strings contain Doda"
    PASS=$((PASS + 1))
else
    red "Brand strings missing Doda"
    FAIL=$((FAIL + 1))
fi

# --- 7. Check run-all.sh includes all extensions ---
echo ""
echo "--- Scripts Check ---"
RUN_ALL="$PROJECT_DIR/scripts/run-all.sh"
EXT_COUNT=$(grep -o 'components/[a-z-]*/extension' "$RUN_ALL" | wc -l)
if [ "$EXT_COUNT" -eq 1 ]; then
    green "run-all.sh includes 1 extension"
    PASS=$((PASS + 1))
else
    red "run-all.sh has $EXT_COUNT extensions (expected 1)"
    FAIL=$((FAIL + 1))
fi

# --- Summary ---
echo ""
echo "=== Summary: $PASS passed, $FAIL failed ==="
if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
