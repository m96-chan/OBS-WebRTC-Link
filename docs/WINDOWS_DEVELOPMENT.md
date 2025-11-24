# Windows Development Guide

This guide explains how to set up a TDD (Test-Driven Development) environment for OBS-WebRTC-Link on Windows.

## Prerequisites

### Required Software

1. **Visual Studio 2022** (Community Edition or higher)
   - Install with "Desktop development with C++" workload
   - Includes CMake, C++ compiler, and debugging tools
   - Download: https://visualstudio.microsoft.com/downloads/

2. **Git for Windows**
   - With Git Bash support
   - Download: https://git-scm.com/download/win

3. **Git Submodules**
   - Initialize after cloning:
   ```bash
   git submodule update --init --recursive
   ```

### Optional (for full plugin development)

- **OBS Studio SDK** - Required only for building the actual plugin
- **Qt 6.x or Qt 5.15+** - Required for UI components

**Note**: You can develop and test without OBS SDK using the test-only build mode!

## Quick Start - TDD Workflow

### 1. Initial Setup

Open PowerShell in the project root and run:

```powershell
.\scripts\setup-windows-dev.ps1 -Clean -RunTests
```

This will:
- Configure CMake for test-only builds
- Build libdatachannel dependency
- Build Google Test framework
- Build all unit tests
- Run all tests

### 2. Running Tests

After initial setup, run tests anytime:

```powershell
# Run all tests
.\scripts\test.bat

# Clean build and run tests
.\scripts\test.bat clean

# Or use PowerShell directly
.\scripts\setup-windows-dev.ps1 -RunTests
```

### 3. TDD Watch Mode (Recommended!)

For TDD, use watch mode to automatically rebuild and test on file changes:

```powershell
# Watch all tests
.\scripts\tdd-watch.ps1

# Watch specific test
.\scripts\tdd-watch.ps1 -TestName "settings_dialog"

# Watch with custom interval (default: 2 seconds)
.\scripts\tdd-watch.ps1 -Interval 5
```

Watch mode will:
- Monitor all `.cpp`, `.hpp`, `.h`, `.c` files in `src/` and `tests/`
- Automatically rebuild when changes are detected
- Run tests immediately after successful build
- Play a sound when tests pass/fail
- Show clear pass/fail indicators

**Workflow**:
1. Start watch mode: `.\scripts\tdd-watch.ps1`
2. Edit test file: `tests/unit/my_feature_test.cpp`
3. Save → automatic rebuild and test
4. Edit implementation: `src/core/my_feature.cpp`
5. Save → automatic rebuild and test
6. Repeat until all tests pass ✓

Press `Ctrl+C` to stop watch mode.

## Manual Development

### CMake Configuration

```powershell
$CMAKE = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

# Configure for tests only (no OBS SDK required)
& $CMAKE -B build `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_LIBDATACHANNEL=ON `
    -DBUILD_TESTING=ON `
    -DBUILD_TESTS_ONLY=ON
```

### Building

```powershell
# Build all
& $CMAKE --build build --config Release

# Build specific test
& $CMAKE --build build --config Release --target sample_test
```

### Running Tests Manually

```powershell
cd build\tests\unit\Release

# Run all tests
Get-ChildItem -Filter "*_test.exe" | ForEach-Object { & $_.FullName }

# Run specific test
.\sample_test.exe

# Run with verbose output
.\sample_test.exe --gtest_color=yes

# Run specific test cases
.\peer_connection_test.exe --gtest_filter="PeerConnectionTest.*"

# List all tests
.\sample_test.exe --gtest_list_tests
```

## TDD Best Practices

### 1. Red-Green-Refactor Cycle

```powershell
# Start watch mode
.\scripts\tdd-watch.ps1 -TestName "my_feature"

# Step 1: Write failing test (RED)
# Edit: tests/unit/my_feature_test.cpp
# Save → Watch mode runs test → Test fails ✗

# Step 2: Implement minimum code to pass (GREEN)
# Edit: src/core/my_feature.cpp
# Save → Watch mode runs test → Test passes ✓

# Step 3: Refactor (keep tests GREEN)
# Edit: src/core/my_feature.cpp
# Save → Watch mode runs test → Test still passes ✓
```

### 2. Test Organization

```cpp
// tests/unit/my_feature_test.cpp
#include <gtest/gtest.h>
#include "core/my_feature.hpp"

// Use test fixtures for setup/teardown
class MyFeatureTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test data
    }

    void TearDown() override {
        // Cleanup
    }

    // Shared test data
    MyFeature feature;
};

// Write descriptive test names
TEST_F(MyFeatureTest, ShouldReturnTrueForValidInput) {
    // Arrange
    int input = 42;

    // Act
    bool result = feature.process(input);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(MyFeatureTest, ShouldThrowExceptionForInvalidInput) {
    EXPECT_THROW(feature.process(-1), std::invalid_argument);
}
```

### 3. Test Coverage Goals

- **Unit tests**: Test individual functions/classes in isolation
- **Edge cases**: Empty inputs, nulls, boundary values
- **Error handling**: Invalid inputs, exceptions
- **State changes**: Before/after comparisons

## Available Test Executables

After building, you'll have these test executables:

- `sample_test.exe` - Framework verification
- `peer_connection_test.exe` - Peer connection tests
- `signaling_client_test.exe` - Signaling tests
- `whip_client_test.exe` - WHIP protocol tests
- `whep_client_test.exe` - WHEP protocol tests
- `p2p_connection_test.exe` - P2P connection tests
- `webrtc_output_test.exe` - Output plugin tests
- `webrtc_source_test.exe` - Source plugin tests
- `reconnection_manager_test.exe` - Reconnection tests
- `settings_dialog_test.exe` - Settings dialog UI tests (requires Qt)

## Debugging Tests

### Visual Studio Debugger

1. Open solution: `build\obs-webrtc-link.sln`
2. Right-click test project → "Set as Startup Project"
3. Set breakpoints in test or implementation code
4. Press F5 to debug

### Command Line Debugging

```powershell
# Run test with debugger
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" `
    build\obs-webrtc-link.sln /debugexe build\tests\unit\Release\sample_test.exe
```

## Troubleshooting

### CMake not found

**Problem**: `cmake: command not found`

**Solution**: Use the full path to CMake or add it to PATH:
```powershell
$env:Path += ";C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
```

### Submodules not initialized

**Problem**: Build fails with missing dependencies

**Solution**:
```bash
git submodule update --init --recursive
```

### PowerShell execution policy error

**Problem**: `cannot be loaded because running scripts is disabled`

**Solution**: Allow script execution (run as Administrator):
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Or run with bypass:
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\setup-windows-dev.ps1
```

### Build fails with "cannot open file"

**Problem**: Permission denied or file in use

**Solution**:
- Close Visual Studio
- Close any running test executables
- Clean build: `.\scripts\setup-windows-dev.ps1 -Clean`

## Project Structure

```
OBS-WebRTC-Link/
├── src/
│   ├── core/           # Core WebRTC logic (test these!)
│   ├── output/         # OBS output plugin
│   ├── source/         # OBS source plugin
│   └── ui/             # Qt UI components
├── tests/
│   └── unit/           # Unit tests (add your tests here!)
├── scripts/
│   ├── setup-windows-dev.ps1  # Main setup script
│   ├── test.bat               # Quick test runner
│   └── tdd-watch.ps1          # TDD watch mode
└── build/              # Build artifacts (generated)
    └── tests/unit/Release/    # Test executables
```

## Continuous Integration

Tests are automatically run on GitHub Actions for all platforms:
- Linux: Full build with OBS SDK + Qt
- Windows: Test-only build
- macOS: Test-only build

Check CI status: https://github.com/m96-chan/OBS-WebRTC-Link/actions

## Advanced Topics

### Building Full Plugin (with OBS SDK)

If you have OBS Studio SDK installed:

```powershell
& $CMAKE -B build `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_LIBDATACHANNEL=ON `
    -DBUILD_TESTING=ON `
    -DOBS_INCLUDE_SEARCH_PATH="C:\path\to\obs-sdk\include" `
    -DOBS_LIB_SEARCH_PATH="C:\path\to\obs-sdk\lib"

& $CMAKE --build build --config Release --target obs-webrtc-link
```

### Adding Qt UI Tests

Install Qt 6.x, then CMake will automatically enable UI test compilation:

```powershell
# Qt will be detected automatically
.\scripts\setup-windows-dev.ps1 -Clean -RunTests

# Run UI tests
.\scripts\tdd-watch.ps1 -TestName "settings_dialog"
```

## Resources

- [Google Test Documentation](https://google.github.io/googletest/)
- [CMake Documentation](https://cmake.org/documentation/)
- [OBS Studio Plugin Guide](https://obsproject.com/docs/plugins.html)
- [TDD by Example](https://www.goodreads.com/book/show/387190.Test_Driven_Development)

## Getting Help

- Check existing tests in `tests/unit/` for examples
- Read test output carefully - GTest provides detailed failure messages
- Use watch mode for faster iteration
- Run tests frequently (every few minutes)
- Write tests before implementation (TDD!)

Happy Testing! 🧪
