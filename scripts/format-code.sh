#!/usr/bin/env bash
# Format code for OBS-WebRTC-Link

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

# Color output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
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
echo "Formatting C++ files..."
CPP_FILES=$(find "${PROJECT_ROOT}" \
    -path "${PROJECT_ROOT}/deps" -prune -o \
    -path "${PROJECT_ROOT}/build" -prune -o \
    -path "${PROJECT_ROOT}/.git" -prune -o \
    \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "*.c" \) -type f -print)

if [ -z "$CPP_FILES" ]; then
    echo -e "${YELLOW}No C++ files found to format${NC}"
    exit 0
fi

# Format files
FILES_FORMATTED=0
for file in $CPP_FILES; do
    echo -e "${GREEN}Formatting:${NC} $file"
    clang-format -i "$file"
    FILES_FORMATTED=$((FILES_FORMATTED + 1))
done

echo ""
echo -e "${GREEN}Successfully formatted ${FILES_FORMATTED} files!${NC}"
