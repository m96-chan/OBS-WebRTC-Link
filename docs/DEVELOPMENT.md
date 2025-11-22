# Development Guide

This guide provides comprehensive instructions for setting up a development environment and contributing to the OBS-WebRTC-Link project.

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Development Environment Setup](#development-environment-setup)
3. [Building the Project](#building-the-project)
4. [Development Workflow](#development-workflow)
5. [Testing](#testing)
6. [Debugging](#debugging)
7. [Code Style](#code-style)
8. [Common Development Tasks](#common-development-tasks)
9. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required Tools

- **C++ Compiler**:
  - Windows: MSVC 2019 or later (Visual Studio 2019/2022)
  - Linux: GCC 9+ or Clang 10+
  - macOS: Xcode 12+ (Clang 10+)
- **CMake** 3.20 or later
- **Git** with submodule support
- **OBS Studio** 30.x SDK (for plugin development)

### Optional Tools

- **clang-format** (for code formatting)
- **Visual Studio Code** (recommended IDE)
- **Docker** (for local SFU testing)
- **LiveKit CLI** (for generating access tokens)
- **Postman** or **curl** (for testing HTTP endpoints)

---

## Development Environment Setup

### 1. Clone Repository

```bash
git clone --recursive https://github.com/m96-chan/OBS-WebRTC-Link.git
cd OBS-WebRTC-Link
```

If you already cloned without `--recursive`, initialize submodules:

```bash
git submodule update --init --recursive
```

### 2. Install OBS Studio SDK

#### Windows

Download and install OBS Studio from the [official website](https://obsproject.com/).

The SDK is typically installed at:
- Include: `C:\Program Files\obs-studio\include`
- Libraries: `C:\Program Files\obs-studio\bin\64bit`

#### Linux (Ubuntu/Debian)

```bash
sudo apt-get install obs-studio
sudo apt-get install libobs-dev
```

#### macOS

```bash
brew install obs
```

### 3. Install Dependencies

#### Windows

All dependencies are included as Git submodules:
- libdatachannel
- nlohmann-json
- Google Test (automatically fetched by CMake)

No additional installation required.

#### Linux (Ubuntu/Debian)

```bash
# Install build tools
sudo apt-get install build-essential cmake git

# Install OpenSSL (required by libdatachannel)
sudo apt-get install libssl-dev

# Install formatting tools (optional)
sudo apt-get install clang-format
```

#### macOS

```bash
# Install build tools
brew install cmake

# Install OpenSSL
brew install openssl
```

### 4. Setup IDE

#### Visual Studio Code (Recommended)

Install recommended extensions:
- C/C++ (Microsoft)
- CMake Tools (Microsoft)
- clang-format (xaver)
- GitLens (GitKraken)

Create [.vscode/settings.json](.vscode/settings.json):

```json
{
  "C_Cpp.clang_format_style": "file",
  "editor.formatOnSave": true,
  "cmake.configureOnOpen": true,
  "cmake.buildDirectory": "${workspaceFolder}/build"
}
```

#### Visual Studio (Windows)

1. Open Visual Studio
2. File → Open → CMake
3. Select CMakeLists.txt
4. Visual Studio will automatically configure the project

#### CLion

1. Open the project folder
2. CLion will automatically detect CMakeLists.txt
3. Configure CMake settings in Settings → Build, Execution, Deployment → CMake

---

## Building the Project

### Configure CMake

#### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -DOBS_INCLUDE_SEARCH_PATH="C:/Program Files/obs-studio/include" ^
         -DOBS_LIB_SEARCH_PATH="C:/Program Files/obs-studio/bin/64bit"
```

#### Linux

```bash
mkdir build
cd build
cmake .. -DOBS_INCLUDE_SEARCH_PATH="/usr/include" \
         -DOBS_LIB_SEARCH_PATH="/usr/lib"
```

#### macOS

```bash
mkdir build
cd build
cmake .. -DOBS_INCLUDE_SEARCH_PATH="/usr/local/include" \
         -DOBS_LIB_SEARCH_PATH="/usr/local/lib"
```

### Build Options

- `BUILD_TESTS_ONLY=ON`: Build tests without OBS SDK (useful for CI)
- `BUILD_LIBDATACHANNEL=ON`: Build libdatachannel from submodule (default)
- `CMAKE_BUILD_TYPE`: Debug, Release, RelWithDebInfo

### Build

```bash
# Debug build
cmake --build . --config Debug

# Release build
cmake --build . --config Release
```

### Install

```bash
# Install to OBS plugin directory
cmake --install . --config Release
```

On Windows, this installs to:
- `%APPDATA%\obs-studio\plugins\obs-webrtc-link\`

On Linux:
- `~/.config/obs-studio/plugins/obs-webrtc-link/`

---

## Development Workflow

### 1. Create Feature Branch

```bash
git checkout -b feature/my-new-feature
```

### 2. Make Changes

Edit source files in:
- [src/core/](../src/core/) - Core WebRTC components
- [src/output/](../src/output/) - OBS output implementation
- [src/source/](../src/source/) - OBS source implementation

### 3. Format Code

```bash
# Format all C++ files
./scripts/format-code.sh

# Check formatting without modifying files
./scripts/check-format.sh
```

### 4. Build and Test

```bash
cd build
cmake --build . --config Debug
ctest --output-on-failure
```

### 5. Commit Changes

```bash
git add .
git commit -m "Add new feature: description"
```

Follow commit message guidelines:
- Use imperative mood ("Add feature" not "Added feature")
- Keep first line under 72 characters
- Add detailed description in commit body if needed

### 6. Push and Create Pull Request

```bash
git push origin feature/my-new-feature
```

Then create a Pull Request on GitHub.

---

## Testing

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Specific Test Suite

```bash
ctest -R PeerConnectionTest --verbose
```

### Run Tests with Debugging

```bash
# Linux/macOS
gdb --args ./tests/unit/obs-webrtc-core-tests --gtest_filter=PeerConnectionTest.*

# Windows (Visual Studio)
# Set breakpoints and run with F5
```

### Write New Tests

See [TESTING.md](TESTING.md) for detailed testing guidelines.

Example test:

```cpp
/**
 * @brief Test description
 */
TEST_F(ComponentTest, MethodName_Condition_ExpectedBehavior) {
    // Arrange
    auto component = std::make_unique<Component>(config_);

    // Act
    component->doSomething();

    // Assert
    EXPECT_TRUE(component->isReady());
}
```

---

## Debugging

### Enable Verbose Logging

Add log callbacks to capture detailed information:

```cpp
config.logCallback = [](LogLevel level, const std::string& msg) {
    std::cout << "[" << static_cast<int>(level) << "] " << msg << std::endl;
};
```

### Debug Output in OBS

OBS logs are located at:
- Windows: `%APPDATA%\obs-studio\logs\`
- Linux: `~/.config/obs-studio/logs/`
- macOS: `~/Library/Application Support/obs-studio/logs/`

### Debug with GDB (Linux/macOS)

```bash
gdb --args obs
(gdb) break WHIPClient::sendOffer
(gdb) run
```

### Debug with Visual Studio (Windows)

1. Set OBS Studio as the debug target
2. Set breakpoints in plugin code
3. Start debugging (F5)

### Network Debugging

#### Capture WebRTC Traffic

Use Wireshark to capture WebRTC traffic:
- Filter: `udp && rtp`
- Look for STUN/TURN traffic on port 3478
- RTP media on dynamic ports

#### Test WHIP/WHEP Endpoints

```bash
# Test WHIP endpoint
curl -X POST http://localhost:7880/whip \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/sdp" \
  --data-binary @offer.sdp

# Test WHEP endpoint
curl -X POST http://localhost:7880/whep \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/sdp" \
  --data-binary @offer.sdp
```

---

## Code Style

### C++ Style Guide

Follow the project's code style enforced by clang-format:

- Indentation: 4 spaces (no tabs)
- Line length: 100 characters max
- Naming conventions:
  - Classes: PascalCase (`PeerConnection`)
  - Functions: camelCase (`createOffer`)
  - Variables: camelCase (`iceServers`)
  - Constants: UPPER_SNAKE_CASE (`MAX_RETRIES`)
  - Private members: trailing underscore (`impl_`)

### File Headers

All source files should include a file header:

```cpp
/**
 * @file filename.cpp
 * @brief Brief description of the file
 */
```

### Documentation

Document all public APIs with Doxygen-style comments:

```cpp
/**
 * @brief Brief description
 *
 * Detailed description of what the function does.
 *
 * @param param1 Description of param1
 * @param param2 Description of param2
 * @return Description of return value
 * @throws std::runtime_error if something fails
 */
ReturnType functionName(Type1 param1, Type2 param2);
```

### Code Organization

- Header files ([.hpp](../src/core/peer-connection.hpp)): Public interface
- Implementation files (.cpp): Private implementation
- Use PIMPL pattern for all public classes
- One class per file (with matching filename)

---

## Common Development Tasks

### Add New Core Component

1. Create header file in [src/core/](../src/core/):

```cpp
// src/core/my-component.hpp
#pragma once

namespace obswebrtc {
namespace core {

class MyComponent {
public:
    MyComponent();
    ~MyComponent();

    void doSomething();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace core
}  // namespace obswebrtc
```

2. Create implementation file:

```cpp
// src/core/my-component.cpp
#include "my-component.hpp"

namespace obswebrtc {
namespace core {

class MyComponent::Impl {
    // Private implementation
};

MyComponent::MyComponent() : impl_(std::make_unique<Impl>()) {}
MyComponent::~MyComponent() = default;

void MyComponent::doSomething() {
    // Implementation
}

}  // namespace core
}  // namespace obswebrtc
```

3. Add to CMakeLists.txt:

```cmake
set(CORE_SOURCES
    src/core/peer-connection.cpp
    src/core/my-component.cpp  # Add new file
)
```

4. Write tests in [tests/unit/](../tests/unit/):

```cpp
// tests/unit/my_component_test.cpp
#include "core/my-component.hpp"
#include <gtest/gtest.h>

class MyComponentTest : public ::testing::Test {
protected:
    // Setup and teardown
};

TEST_F(MyComponentTest, DoSomething) {
    // Test implementation
}
```

### Add New Configuration Option

1. Add to config structure:

```cpp
struct MyConfig {
    std::string url;
    int timeout = 5000;  // New option with default value
};
```

2. Update constructor to validate:

```cpp
MyComponent::MyComponent(const MyConfig& config) {
    if (config.timeout <= 0) {
        throw std::invalid_argument("Timeout must be positive");
    }
    // ...
}
```

3. Update documentation in header file

### Add New Callback

1. Define callback type:

```cpp
using MyCallback = std::function<void(const std::string& data)>;
```

2. Add to config:

```cpp
struct MyConfig {
    MyCallback onMyEvent;
};
```

3. Invoke in implementation:

```cpp
void MyComponent::Impl::handleEvent(const std::string& data) {
    if (config_.onMyEvent) {
        config_.onMyEvent(data);
    }
}
```

### Update Dependencies

```bash
# Update all submodules to latest
git submodule update --remote --merge

# Update specific submodule
cd deps/libdatachannel
git checkout master
git pull
cd ../..
git add deps/libdatachannel
git commit -m "Update libdatachannel to latest"
```

---

## Troubleshooting

### Build Issues

#### CMake cannot find OBS Studio

**Problem**: `Could not find OBS Studio`

**Solution**: Set the correct paths:
```bash
cmake .. -DOBS_INCLUDE_SEARCH_PATH="/path/to/obs/include" \
         -DOBS_LIB_SEARCH_PATH="/path/to/obs/lib"
```

#### libdatachannel build fails

**Problem**: Submodule not initialized

**Solution**:
```bash
git submodule update --init --recursive
```

#### OpenSSL not found (Linux)

**Problem**: `Could not find OpenSSL`

**Solution**:
```bash
sudo apt-get install libssl-dev
```

### Runtime Issues

#### Plugin not loading in OBS

**Problem**: Plugin doesn't appear in OBS

**Solution**:
1. Check OBS log for errors
2. Verify plugin is installed in correct directory
3. Ensure all dependencies (DLLs) are present
4. Try debug build for more detailed logging

#### Connection failures

**Problem**: WebRTC connection fails

**Solution**:
1. Check firewall settings
2. Verify STUN/TURN server is accessible
3. Check SFU endpoint is correct
4. Verify bearer token is valid
5. Enable verbose logging to see detailed errors

#### Memory leaks

**Problem**: Memory usage increases over time

**Solution**:
1. Use memory profilers (Valgrind on Linux, Visual Studio Memory Profiler on Windows)
2. Ensure all resources are properly cleaned up in destructors
3. Check for circular references in callbacks
4. Verify smart pointers are used correctly

---

## Development Tools

### Recommended VS Code Extensions

- **C/C++** (ms-vscode.cpptools): IntelliSense and debugging
- **CMake Tools** (ms-vscode.cmake-tools): CMake integration
- **clang-format** (xaver.clang-format): Code formatting
- **GitLens** (eamodio.gitlens): Git integration
- **Better Comments** (aaron-bond.better-comments): Enhanced comments

### Useful Scripts

Create [scripts/](../scripts/) directory with helper scripts:

#### format-code.sh

```bash
#!/bin/bash
find src tests -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
```

#### check-format.sh

```bash
#!/bin/bash
find src tests -name "*.cpp" -o -name "*.hpp" | \
    xargs clang-format --dry-run --Werror
```

#### run-tests.sh

```bash
#!/bin/bash
cd build
cmake --build . --config Debug
ctest --output-on-failure --verbose
```

### Docker Development Environment

Use Docker for consistent development environment:

```dockerfile
# Dockerfile.dev
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    clang-format \
    gdb

WORKDIR /workspace
```

Build and run:

```bash
docker build -f Dockerfile.dev -t obs-webrtc-dev .
docker run -it -v $(pwd):/workspace obs-webrtc-dev bash
```

---

## Contributing Checklist

Before submitting a Pull Request, ensure:

- [ ] Code builds without warnings
- [ ] All existing tests pass
- [ ] New tests added for new functionality
- [ ] Code formatted with clang-format
- [ ] Documentation updated (API docs, README, etc.)
- [ ] Commit messages follow guidelines
- [ ] No debug code or commented-out code
- [ ] Thread safety considered
- [ ] Memory leaks checked
- [ ] Error handling implemented

---

**See Also**:
- [API Reference](API-REFERENCE.md)
- [Architecture Overview](ARCHITECTURE.md)
- [Testing Guide](TESTING.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [Project Structure](OBS-WebRTC-Link-Project-Structure.md)
