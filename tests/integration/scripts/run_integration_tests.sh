#!/bin/bash
# Integration test runner script for Linux/macOS
#
# This script:
# 1. Checks for Docker availability
# 2. Starts LiveKit container via Docker Compose
# 3. Runs integration tests
# 4. Stops LiveKit container
# 5. Reports results

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../../../build"
DOCKER_DIR="${SCRIPT_DIR}/../docker"

echo "===================="
echo "Integration Test Runner"
echo "===================="
echo

# Check Docker
echo "Checking Docker availability..."
if ! command -v docker &> /dev/null; then
    echo "ERROR: Docker is not installed or not in PATH"
    echo "Integration tests require Docker to run LiveKit server"
    exit 1
fi

if ! docker info &> /dev/null; then
    echo "ERROR: Docker is not running"
    echo "Please start Docker and try again"
    exit 1
fi

echo "Docker is available"
echo

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "ERROR: Build directory not found: $BUILD_DIR"
    echo "Please build the project first"
    exit 1
fi

# Navigate to build directory
cd "$BUILD_DIR"

echo "Running integration tests..."
echo

# Run integration tests with CTest
# -R Integration: Run only integration tests
# -V: Verbose output
# --output-on-failure: Show output only for failed tests
ctest -R Integration -V --output-on-failure

TEST_RESULT=$?

echo
echo "===================="
if [ $TEST_RESULT -eq 0 ]; then
    echo "All integration tests PASSED"
else
    echo "Some integration tests FAILED"
fi
echo "===================="

exit $TEST_RESULT
