#!/usr/bin/env bash
# Check code formatting for OBS-WebRTC-Link

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo -e "${RED}Error: clang-format is not installed${NC}"
    echo "Please install clang-format:"
    echo "  - Ubuntu/Debian: sudo apt-get install clang-format"
    echo "  - macOS: brew install clang-format"
    echo "  - Windows: Download from LLVM website"
    exit 1
fi

# Get clang-format version
CLANG_FORMAT_VERSION=$(clang-format --version | grep -oP '\d+\.\d+' | head -1 || echo "unknown")
echo "Using clang-format version: ${CLANG_FORMAT_VERSION}"

# Find all C++ source files (excluding dependencies)
echo "Checking C++ file formatting..."
CPP_FILES=$(find "${PROJECT_ROOT}" \
    -path "${PROJECT_ROOT}/deps" -prune -o \
    -path "${PROJECT_ROOT}/build" -prune -o \
    -path "${PROJECT_ROOT}/.git" -prune -o \
    \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.c" \) -type f -print)

if [ -z "$CPP_FILES" ]; then
    echo -e "${YELLOW}No C++ files found to check${NC}"
    exit 0
fi

# Check formatting
FORMATTING_ISSUES=0
FILES_CHECKED=0

for file in $CPP_FILES; do
    FILES_CHECKED=$((FILES_CHECKED + 1))

    # Check if file needs reformatting
    if ! clang-format --dry-run --Werror "$file" &> /dev/null; then
        echo -e "${RED}✗${NC} $file needs formatting"
        FORMATTING_ISSUES=$((FORMATTING_ISSUES + 1))

        # Show diff if requested
        if [ "${SHOW_DIFF:-0}" = "1" ]; then
            clang-format --dry-run "$file" | diff -u "$file" - || true
        fi
    else
        if [ "${VERBOSE:-0}" = "1" ]; then
            echo -e "${GREEN}✓${NC} $file"
        fi
    fi
done

echo ""
echo "Checked ${FILES_CHECKED} files"

if [ $FORMATTING_ISSUES -gt 0 ]; then
    echo -e "${RED}Found ${FORMATTING_ISSUES} files with formatting issues${NC}"
    echo ""
    echo "To fix formatting issues, run:"
    echo "  ./scripts/format-code.sh"
    echo ""
    echo "To see diffs, run:"
    echo "  SHOW_DIFF=1 ./scripts/check-format.sh"
    exit 1
else
    echo -e "${GREEN}All files are properly formatted!${NC}"
    exit 0
fi
