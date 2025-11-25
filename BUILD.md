# Building OBS-WebRTC-Link

This document provides detailed instructions for building the OBS-WebRTC-Link plugin from source.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Quick Build (Tests Only)](#quick-build-tests-only)
- [Full Plugin Build](#full-plugin-build)
  - [Linux](#linux)
  - [Windows](#windows)
  - [macOS](#macos)
- [Build Options](#build-options)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### All Platforms

- **CMake** 3.20 or later
- **C++17 compatible compiler**:
  - Linux: GCC 9+ or Clang 10+
  - Windows: Visual Studio 2019 or later
  - macOS: Xcode 12+ (Apple Clang 12+)
- **Git** (for cloning submodules)

### Platform-Specific Dependencies

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake git pkg-config \
  libssl-dev ninja-build

# Fedora/RHEL
sudo dnf install gcc-c++ cmake git pkgconfig openssl-devel ninja-build
```

**Windows:**
- Visual Studio 2019 or later with C++ build tools
- CMake (install from https://cmake.org/)

**macOS:**
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake pkg-config openssl ninja
```

---

## Quick Build (Tests Only)

Build and run tests **without** requiring OBS Studio SDK:

```bash
# Clone the repository
git clone https://github.com/m96-chan/OBS-WebRTC-Link.git
cd OBS-WebRTC-Link

# Initialize submodules
git submodule update --init --recursive

# Configure for tests only
cmake -B build -DBUILD_TESTS_ONLY=ON -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run tests
cd build
ctest --output-on-failure --verbose
```

---

## Full Plugin Build

Building the full plugin requires the **OBS Studio SDK**. Follow the platform-specific instructions below.

### Linux

#### 1. Install OBS Studio Development Files

**Ubuntu/Debian:**
```bash
sudo apt-get install obs-studio
# Or build OBS Studio from source for development headers
```

**Fedora/RHEL:**
```bash
sudo dnf install obs-studio-devel
```

#### 2. Build Plugin

```bash
# Configure with OBS SDK paths
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DOBS_INCLUDE_SEARCH_PATH="/usr/include" \
  -DOBS_LIB_SEARCH_PATH="/usr/lib"

# Build
cmake --build build --config Release -j$(nproc)

# Install (requires sudo)
sudo cmake --install build
```

#### 3. Installation Paths

The plugin will be installed to:
- `/usr/lib/obs-plugins/` - Plugin binary
- `/usr/share/obs/obs-plugins/obs-webrtc-link/` - Plugin data

---

### Windows

#### 1. Install OBS Studio

Download and install OBS Studio from:
https://obsproject.com/download

**Note:** You need the full OBS Studio installation, not just the runtime.

#### 2. Build Plugin

Open **Developer Command Prompt for Visual Studio**:

```cmd
REM Clone repository
git clone https://github.com/m96-chan/OBS-WebRTC-Link.git
cd OBS-WebRTC-Link

REM Initialize submodules
git submodule update --init --recursive

REM Configure with OBS SDK paths (adjust paths to your OBS installation)
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
  -DOBS_INCLUDE_SEARCH_PATH="C:/Program Files/obs-studio/include" ^
  -DOBS_LIB_SEARCH_PATH="C:/Program Files/obs-studio/bin/64bit"

REM Build
cmake --build build --config Release

REM Install (run as Administrator)
cmake --install build --config Release
```

#### 3. Installation Paths

The plugin will be installed to:
- `C:\Program Files\obs-studio\obs-plugins\64bit\` - Plugin DLL
- `C:\Program Files\obs-studio\data\obs-plugins\obs-webrtc-link\` - Plugin data

---

### macOS

#### 1. Install OBS Studio

**Option A: Homebrew (Recommended)**
```bash
brew install obs
```

**Option B: Download from Official Website**
Download from https://obsproject.com/download

#### 2. Build Plugin

```bash
# Configure with OBS SDK paths
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DOBS_INCLUDE_SEARCH_PATH="/opt/homebrew/include" \
  -DOBS_LIB_SEARCH_PATH="/opt/homebrew/lib"

# Build
cmake --build build --config Release -j$(sysctl -n hw.ncpu)

# Install (requires sudo)
sudo cmake --install build --config Release
```

#### 3. Installation Paths

The plugin will be installed to:
- `/Library/Application Support/obs-studio/plugins/obs-webrtc-link/` - Plugin bundle

---

## Build Options

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TESTS_ONLY` | `OFF` | Build only tests without OBS plugin |
| `BUILD_TESTS` | `ON` | Build unit tests |
| `BUILD_BENCHMARKS` | `OFF` | Build performance benchmarks |
| `CMAKE_BUILD_TYPE` | `Release` | Build type: `Debug`, `Release`, `RelWithDebInfo` |

### Examples

**Build with Debug Symbols:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

**Build with Benchmarks:**
```bash
cmake -B build -DBUILD_BENCHMARKS=ON -DBUILD_TESTS_ONLY=ON
cmake --build build --config Release
./build/tests/benchmarks/whip_connection_benchmark
```

**Disable Tests:**
```bash
cmake -B build -DBUILD_TESTS=OFF
```

---

## Troubleshooting

### Common Issues

#### "Could not find OBS Studio"

**Solution:** Ensure `OBS_INCLUDE_SEARCH_PATH` and `OBS_LIB_SEARCH_PATH` point to valid OBS installation directories.

**Verify paths:**
```bash
# Linux
ls /usr/include/obs
ls /usr/lib/libobs.so

# macOS
ls /opt/homebrew/include/obs
ls /opt/homebrew/lib/libobs.dylib

# Windows
dir "C:\Program Files\obs-studio\include"
dir "C:\Program Files\obs-studio\bin\64bit\obs.dll"
```

#### "Submodule not initialized"

**Solution:**
```bash
git submodule update --init --recursive
```

#### Qt not found (Non-critical)

**Warning:** `Could not find Qt6` or `Qt5`

**Impact:** Plugin UI will not be built, but plugin functionality is unaffected.

**Solution (optional):**
```bash
# Linux
sudo apt-get install qt6-base-dev

# macOS
brew install qt@6

# Windows
# Download Qt from https://www.qt.io/download-open-source
```

#### Link errors on Linux

**Error:** `undefined reference to obs_*`

**Solution:** Ensure OBS libraries are in the library search path:
```bash
export LD_LIBRARY_PATH=/usr/lib:$LD_LIBRARY_PATH
```

#### Permission denied during install

**Solution:** Use `sudo` on Linux/macOS or run as Administrator on Windows:
```bash
sudo cmake --install build
```

---

## Advanced Build Configuration

### Custom Installation Prefix

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/custom/path
cmake --build build
cmake --install build
```

### Cross-Compilation

For advanced users building for different architectures, refer to the CMake cross-compilation documentation:
https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html

---

## Development Workflow

For active development:

```bash
# Configure once
cmake -B build -DBUILD_TESTS_ONLY=ON -DCMAKE_BUILD_TYPE=Debug

# Rebuild after code changes
cmake --build build --config Debug

# Run specific test
./build/tests/unit/peer_connection_test

# Run all tests
cd build && ctest --output-on-failure
```

---

## Getting Help

If you encounter build issues not covered here:

1. Check [GitHub Issues](https://github.com/m96-chan/OBS-WebRTC-Link/issues)
2. Review [CONTRIBUTING.md](CONTRIBUTING.md)
3. Open a new issue with:
   - Your platform and versions (OS, CMake, compiler, OBS)
   - Full CMake configuration output
   - Complete build error log

---

## Additional Resources

- [README.md](README.md) - General project overview
- [CONTRIBUTING.md](docs/CONTRIBUTING.md) - Contribution guidelines
- [USAGE_EXAMPLES.md](docs/USAGE_EXAMPLES.md) - Plugin usage examples
- [OBS Studio Build Instructions](https://obsproject.com/wiki/install-instructions)
