# OBS-WebRTC-Link

[![Build Status](https://github.com/m96-chan/OBS-WebRTC-Link/workflows/Build/badge.svg)](https://github.com/m96-chan/OBS-WebRTC-Link/actions)
[![License](https://img.shields.io/badge/license-GPLv2-blue.svg)](LICENSE)

**English | 日本語**

A versatile WebRTC plugin for OBS Studio that provides **Universal WebRTC Input & Output**.

While perfect for OBS-to-OBS relay, it also enables you to receive streams from browsers, mobile devices, and other WHIP clients directly into OBS as a source.

---

## 🔗 Architecture

This plugin supports both **SFU Relay (WHIP/WHEP)** for stability and **Direct P2P** for low latency.

### Mode A: SFU Relay (Recommended)

Best for: Internet streaming, receiving from multiple sources (Browsers, Mobiles), and complex networks.

(Compatible with any WHIP/WHEP compliant SFU like LiveKit, SRS, Janus)

```
graph LR
    Source[OBS / Browser / Mobile] -- WHIP --> SFU[SFU Server (e.g. LiveKit)]
    SFU -- WHEP --> Receiver[OBS (This Plugin)]
    SFU -- WHEP --> Browser[Browser Viewer]
```

---

### Mode B: Direct P2P (Advanced)

Best for: Local Area Networks (LAN) or 1-on-1 direct connections.

```
graph LR
    Peer[OBS / Browser Peer] -- P2P / Direct --> Receiver[OBS (This Plugin)]
```

---

## 🚀 Features

### Universal WebRTC Source:
- Receive video/audio from other OBS instances.
- Receive streams from web browsers (via WebRTC).
- Receive from mobile apps or any WHIP-compatible publisher.

### Hybrid Connection Modes:
- **SFU Relay Support:** Stable connections through WHIP/WHEP compliant servers (LiveKit tested).
- **Direct P2P:** Ultra-low latency direct connections.

### Bidirectional:
- **Output:** Send OBS Program output via WebRTC.
- **Source:** Add a “WebRTC Link Source” to receive streams.

### Additional:
- Automatic reconnection
- Hardware accelerated encoding/decoding (NVENC/AMF/QuickSync)

---

## 📦 Installation (Windows)

1. Download the installer or ZIP from Releases.
2. Install to your OBS plugin directory.

---

## ⚙️ Usage Guide

### Scenario 1: Receiving from LiveKit / SFU (Recommended)

**Receiver (Your OBS):**
- Add `WebRTC Link Source`
- Mode: `SFU (WHEP)`
- URL: `https://your-sfu-endpoint/whep`
- Token: subscriber token

**Sender:**
- OBS: select WebRTC Output
- Browser/Mobile: publish via WHIP or LiveKit SDK

---

### Scenario 2: Direct P2P

**Sender:**
- Choose "P2P Host"
- Copy the Session ID

**Receiver:**
- Add `WebRTC Link Source`
- Select `P2P Client`
- Paste Session ID

**For detailed LiveKit setup instructions, see [LiveKit Setup Guide](docs/LIVEKIT_SETUP.md).**

---

## 🐳 Development Environment (Docker)

For easy local testing with LiveKit SFU, you can use the included Docker environment:

### Quick Start

**1. Navigate to the docker directory:**
```bash
cd docker/livekit
```

**2. Copy the example environment file:**
```bash
cp .env.example .env
```

**3. Generate API credentials:**
```bash
# Generate API Key
openssl rand -base64 32

# Generate API Secret
openssl rand -base64 32
```

**4. Edit `.env` and set your credentials:**
```env
LIVEKIT_API_KEY=your-generated-api-key
LIVEKIT_API_SECRET=your-generated-api-secret
```

**5. Start LiveKit:**
```bash
docker-compose up -d
```

### LiveKit Endpoints

Once running, LiveKit will be available at:
- **WebRTC API**: `http://localhost:7880`
- **WHIP Endpoint**: `http://localhost:7880/whip`
- **WHEP Endpoint**: `http://localhost:7880/whep`

### Generating Access Tokens

To connect to LiveKit, you need to generate access tokens. You can use the [LiveKit CLI](https://github.com/livekit/livekit-cli) or generate tokens programmatically.

**Using LiveKit CLI:**
```bash
# Install LiveKit CLI
go install github.com/livekit/livekit-cli/cmd/livekit-cli@latest

# Generate a publisher token (for WHIP)
livekit-cli create-token \
  --api-key <LIVEKIT_API_KEY> \
  --api-secret <LIVEKIT_API_SECRET> \
  --join --room my-room --identity publisher \
  --valid-for 24h

# Generate a subscriber token (for WHEP)
livekit-cli create-token \
  --api-key <LIVEKIT_API_KEY> \
  --api-secret <LIVEKIT_API_SECRET> \
  --join --room my-room --identity subscriber \
  --valid-for 24h
```

### Stopping LiveKit

```bash
docker-compose down
```

To remove all data:
```bash
docker-compose down -v
```

---

## 🛠️ Build from Source

### Prerequisites

Before building the plugin, ensure you have the following requirements:

**Required Tools:**
- **CMake**: 3.20 or later ([Download](https://cmake.org/download/))
- **Git**: For cloning the repository and managing submodules
- **C++17 Compatible Compiler:**
  - **Windows**: Visual Studio 2019 or later (MSVC 14.2+)
  - **Linux**: GCC 9+ or Clang 10+
  - **macOS**: Xcode 12+ (Apple Clang 12+)

**Required Libraries:**
- **OBS Studio SDK**: Version 30.x or later
  - You need the compiled OBS Studio with development headers
  - See [Getting OBS Studio](#getting-obs-studio) below

**Optional (Included as Submodules):**
- [libdatachannel](https://github.com/paullouisageneau/libdatachannel) - WebRTC implementation
- [nlohmann-json](https://github.com/nlohmann/json) - JSON library for C++
- [Google Test](https://github.com/google/googletest) - For unit tests
- [Google Benchmark](https://github.com/google/benchmark) - For performance benchmarks

### Getting OBS Studio

You have several options to obtain OBS Studio development files:

#### Option 1: Use Pre-built OBS Studio (Recommended for Windows)

**Windows:**
1. Download the latest OBS Studio installer from [obsproject.com](https://obsproject.com/)
2. Install OBS Studio to the default location (e.g., `C:\Program Files\obs-studio`)
3. The include files are typically located at:
   - Headers: `C:\Program Files\obs-studio\include`
   - Libraries: `C:\Program Files\obs-studio\bin\64bit`

**macOS:**
1. Download the OBS Studio DMG from [obsproject.com](https://obsproject.com/)
2. Extract the app and locate the development headers inside the bundle
3. Alternatively, install via Homebrew:
   ```bash
   brew install obs
   ```

**Linux:**
Install OBS Studio development packages:
```bash
# Ubuntu/Debian
sudo apt install obs-studio libobs-dev

# Fedora
sudo dnf install obs-studio obs-studio-devel

# Arch Linux
sudo pacman -S obs-studio
```

#### Option 2: Build OBS Studio from Source

If you need a specific version or want to contribute to OBS itself:

1. Clone the OBS Studio repository:
   ```bash
   git clone --recursive https://github.com/obsproject/obs-studio.git
   cd obs-studio
   ```

2. Follow the build instructions for your platform:
   - [Windows Build Guide](https://github.com/obsproject/obs-studio/wiki/Build-Instructions-For-Windows)
   - [macOS Build Guide](https://github.com/obsproject/obs-studio/wiki/Build-Instructions-For-macOS)
   - [Linux Build Guide](https://github.com/obsproject/obs-studio/wiki/Build-Instructions-For-Linux)

3. After building, note the paths to:
   - Include directory: `<obs-build-dir>/include`
   - Library directory: `<obs-build-dir>/build/libobs` or `<obs-build-dir>/build/Release/bin/64bit`

### Platform-Specific Build Instructions

#### Windows (Visual Studio)

**1. Clone the repository with submodules:**
```bash
git clone --recursive https://github.com/m96-chan/OBS-WebRTC-Link.git
cd OBS-WebRTC-Link
```

If you already cloned without `--recursive`, initialize submodules:
```bash
git submodule update --init --recursive
```

**2. Configure with CMake:**

Open a command prompt or PowerShell and run:
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 ^
  -DOBS_INCLUDE_SEARCH_PATH="C:/Program Files/obs-studio/include" ^
  -DOBS_LIB_SEARCH_PATH="C:/Program Files/obs-studio/bin/64bit"
```

Replace `"Visual Studio 17 2022"` with your installed version:
- Visual Studio 2022: `"Visual Studio 17 2022"`
- Visual Studio 2019: `"Visual Studio 16 2019"`

**3. Build:**
```bash
cmake --build . --config Release
```

Or open `obs-webrtc-link.sln` in Visual Studio and build from the IDE.

**4. Install:**
```bash
cmake --install . --config Release
```

This will install the plugin to:
- Plugin: `C:\Program Files\obs-studio\obs-plugins\64bit\`
- Data: `C:\Program Files\obs-studio\data\obs-plugins\obs-webrtc-link\`

**Note:** You may need administrator privileges to install to `Program Files`.

#### macOS (Xcode or Command Line)

**1. Clone the repository with submodules:**
```bash
git clone --recursive https://github.com/m96-chan/OBS-WebRTC-Link.git
cd OBS-WebRTC-Link
git submodule update --init --recursive
```

**2. Install dependencies:**
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install OBS Studio and dependencies
brew install obs cmake
```

**3. Configure with CMake:**

For command-line build:
```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DOBS_INCLUDE_SEARCH_PATH="/opt/homebrew/include" \
  -DOBS_LIB_SEARCH_PATH="/opt/homebrew/lib"
```

For Xcode:
```bash
mkdir build && cd build
cmake .. -G Xcode \
  -DOBS_INCLUDE_SEARCH_PATH="/opt/homebrew/include" \
  -DOBS_LIB_SEARCH_PATH="/opt/homebrew/lib"
```

**4. Build:**

Command-line:
```bash
cmake --build . --config Release
```

Or open the generated Xcode project and build from the IDE.

**5. Install:**
```bash
sudo cmake --install . --config Release
```

This will install the plugin to:
- Plugin: `/Library/Application Support/obs-studio/plugins/obs-webrtc-link.so`
- Data: `/Library/Application Support/obs-studio/plugins/obs-webrtc-link/data/`

#### Linux (GCC/Clang)

**1. Install dependencies:**

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake git \
  libobs-dev obs-studio \
  libssl-dev pkg-config
```

**Fedora:**
```bash
sudo dnf install gcc-c++ cmake git \
  obs-studio-devel \
  openssl-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake git obs-studio openssl
```

**2. Clone the repository with submodules:**
```bash
git clone --recursive https://github.com/m96-chan/OBS-WebRTC-Link.git
cd OBS-WebRTC-Link
git submodule update --init --recursive
```

**3. Configure with CMake:**
```bash
mkdir build && cd build
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DOBS_INCLUDE_SEARCH_PATH="/usr/include" \
  -DOBS_LIB_SEARCH_PATH="/usr/lib"
```

**4. Build:**
```bash
cmake --build . -j$(nproc)
```

The `-j$(nproc)` flag uses all available CPU cores for faster compilation.

**5. Install:**
```bash
sudo cmake --install .
```

This will install the plugin to:
- Plugin: `/usr/lib/obs-plugins/obs-webrtc-link.so`
- Data: `/usr/share/obs/obs-plugins/obs-webrtc-link/`

### CMake Configuration Options

The following CMake options are available to customize the build:

| Option | Default | Description |
|--------|---------|-------------|
| `OBS_INCLUDE_SEARCH_PATH` | - | Path to OBS Studio include directory (required) |
| `OBS_LIB_SEARCH_PATH` | - | Path to OBS Studio library directory (required) |
| `BUILD_LIBDATACHANNEL` | `ON` | Build libdatachannel from source (submodule) |
| `LIBDATACHANNEL_DIR` | - | Custom path to libdatachannel installation |
| `BUILD_TESTING` | `ON` | Build unit tests (requires Google Test) |
| `BUILD_BENCHMARKS` | `ON` | Build performance benchmarks (requires Google Benchmark) |
| `BUILD_TESTS_ONLY` | `OFF` | Build only tests without OBS plugin (useful for CI) |

**Example: Build without tests and benchmarks:**
```bash
cmake .. \
  -DOBS_INCLUDE_SEARCH_PATH="/path/to/obs/include" \
  -DOBS_LIB_SEARCH_PATH="/path/to/obs/lib" \
  -DBUILD_TESTING=OFF \
  -DBUILD_BENCHMARKS=OFF
```

**Example: Use system-installed libdatachannel:**
```bash
cmake .. \
  -DOBS_INCLUDE_SEARCH_PATH="/path/to/obs/include" \
  -DOBS_LIB_SEARCH_PATH="/path/to/obs/lib" \
  -DBUILD_LIBDATACHANNEL=OFF
```

### Building Tests Only (Without OBS SDK)

If you want to build and run tests without installing OBS Studio:

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTS_ONLY=ON
cmake --build . --config Release
ctest --output-on-failure
```

This is useful for continuous integration (CI) environments.

### Dependency Management

**Included as Git Submodules (Automatically Built):**
- libdatachannel: WebRTC implementation
- nlohmann-json: JSON library for C++
- Google Test: Unit testing framework
- Google Benchmark: Performance benchmarking

The project automatically builds these dependencies from submodules. To update them:
```bash
git submodule update --remote
```

**Using System Libraries (Advanced):**

If you have libraries installed system-wide and want to use them instead:

```bash
cmake .. \
  -DBUILD_LIBDATACHANNEL=OFF \
  -DOBS_INCLUDE_SEARCH_PATH="/path/to/obs/include" \
  -DOBS_LIB_SEARCH_PATH="/path/to/obs/lib"
```

The project includes custom CMake Find modules that automatically locate system libraries. See [cmake/README.md](cmake/README.md) for details.

### IDE Development Setup

#### Visual Studio (Windows)

1. Open Visual Studio
2. Select **File → Open → CMake** and choose the root `CMakeLists.txt`
3. Visual Studio will automatically configure the project
4. Edit CMake settings in `CMakeSettings.json` to specify OBS paths:
   ```json
   {
     "configurations": [
       {
         "name": "x64-Release",
         "generator": "Ninja",
         "configurationType": "Release",
         "buildRoot": "${projectDir}\\build",
         "cmakeCommandArgs": "-DOBS_INCLUDE_SEARCH_PATH=\"C:/Program Files/obs-studio/include\" -DOBS_LIB_SEARCH_PATH=\"C:/Program Files/obs-studio/bin/64bit\""
       }
     ]
   }
   ```
5. Build the project using **Build → Build All**

#### Xcode (macOS)

1. Generate Xcode project:
   ```bash
   mkdir build && cd build
   cmake .. -G Xcode \
     -DOBS_INCLUDE_SEARCH_PATH="/opt/homebrew/include" \
     -DOBS_LIB_SEARCH_PATH="/opt/homebrew/lib"
   ```
2. Open `obs-webrtc-link.xcodeproj` in Xcode
3. Select the target and build configuration (Debug/Release)
4. Build using **Product → Build** (⌘B)

#### CLion (Cross-Platform)

1. Open CLion and select **Open** → Choose the project root directory
2. CLion will automatically detect `CMakeLists.txt`
3. Configure CMake options in **Settings → Build, Execution, Deployment → CMake**:
   - Add CMake options:
     ```
     -DOBS_INCLUDE_SEARCH_PATH=/path/to/obs/include
     -DOBS_LIB_SEARCH_PATH=/path/to/obs/lib
     ```
4. Build the project using **Build → Build Project**

#### Visual Studio Code (Cross-Platform)

1. Install the **CMake Tools** extension
2. Open the project folder in VS Code
3. Create or edit `.vscode/settings.json`:
   ```json
   {
     "cmake.configureArgs": [
       "-DOBS_INCLUDE_SEARCH_PATH=/path/to/obs/include",
       "-DOBS_LIB_SEARCH_PATH=/path/to/obs/lib"
     ]
   }
   ```
4. Press **Ctrl+Shift+P** (or **Cmd+Shift+P** on macOS) and run **CMake: Configure**
5. Build using **CMake: Build** or press **F7**

### Troubleshooting

#### Common Build Errors

**Error: "Could not find OBS Studio"**
- **Solution**: Ensure `OBS_INCLUDE_SEARCH_PATH` and `OBS_LIB_SEARCH_PATH` are correctly set
- Verify that OBS Studio is installed and the paths contain `obs-module.h` and `obs.lib`/`libobs.so`

**Error: "Submodule not found"**
- **Solution**: Initialize submodules:
  ```bash
  git submodule update --init --recursive
  ```

**Error: "Qt not found" (Windows)**
- **Solution**: Qt is optional for UI components. The plugin will build without Qt, but without UI features.
- To enable Qt, install Qt 5.15+ or Qt 6.x and ensure it's in your PATH, or disable UI:
  ```bash
  cmake .. -DQT_FOUND=OFF
  ```

**Error: "CMake version too old"**
- **Solution**: Update CMake to version 3.20 or later
- Download from [cmake.org](https://cmake.org/download/)

**Error: "Compiler not found" (Windows)**
- **Solution**: Install Visual Studio 2019 or later with C++ development tools
- Ensure you open "Developer Command Prompt for VS" or run `vcvarsall.bat`

**Error: "libobs.so not found" (Linux)**
- **Solution**: Install OBS Studio development package:
  ```bash
  sudo apt install libobs-dev  # Ubuntu/Debian
  sudo dnf install obs-studio-devel  # Fedora
  ```

**Error: "Permission denied" during install**
- **Solution**: Use `sudo` on Linux/macOS:
  ```bash
  sudo cmake --install . --config Release
  ```
- On Windows, run Command Prompt as Administrator

#### Debug vs. Release Builds

**Debug Build (for development):**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --config Debug
```

Debug builds include:
- Debug symbols for debugging
- No optimizations
- Slower performance but easier to debug

**Release Build (for production):**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Release builds include:
- Full compiler optimizations
- Smaller binary size
- Better performance

**Important**: Match your build configuration with OBS Studio's configuration. If OBS was built in Release mode, build the plugin in Release mode as well to avoid potential issues.

#### Running Tests

After building with `BUILD_TESTING=ON`:

```bash
cd build
ctest --output-on-failure --verbose
```

Or run specific tests:
```bash
./tests/unit/sample_test
./tests/unit/peer_connection_test
```

#### Running Benchmarks

After building with `BUILD_BENCHMARKS=ON`:

```bash
cd build
./tests/benchmarks/whip_connection_benchmark
```

### Verifying Installation

After installation, verify the plugin is loaded:

1. Launch OBS Studio
2. Go to **Tools → Scripts** or check the log file
3. Look for "obs-webrtc-link" in the loaded plugins list
4. Add a **WebRTC Link Source** to verify the plugin is working

### Getting Help

If you encounter issues:
- Check the [Issues](https://github.com/m96-chan/OBS-WebRTC-Link/issues) page
- Review [OBS Plugin Development Guide](https://obsproject.com/docs/plugins.html)
- Ask questions in the [Discussions](https://github.com/m96-chan/OBS-WebRTC-Link/discussions) section

---

## 📝 License

Licensed under **GPLv2**.  
See LICENSE for full details.

---

# 🇯🇵 日本語概要

OBSでWebRTC映像を送受信するための汎用プラグインです。  
OBS同士のリレーはもちろん、ブラウザ・スマホ・他の配信アプリからの映像をOBSソースとして受信できます。

---

## 主な機能

### WebRTC入力ソース:
- ブラウザやスマホからの映像をOBSへ取り込み可能
- LiveKitなどのSFUと接続し、安定した遠隔映像受信が可能

### 2つの接続モード:
- **SFUリレー（推奨）**：サーバー経由で安定接続
- **Direct P2P**：LAN向け直接接続

---

## 想定ユースケース
- OBSリレー：自宅↔スタジオ間の伝送
- ゲスト参加：ブラウザ経由で映像を送信
- スマホカメラ：WebRTCを使ったワイヤレスカメラ化

---

## 🐳 開発環境（Docker）

ローカルでのテストを簡単に行うため、LiveKit SFUのDocker環境を用意しています。

### クイックスタート

**1. dockerディレクトリに移動:**
```bash
cd docker/livekit
```

**2. 環境変数ファイルをコピー:**
```bash
cp .env.example .env
```

**3. API認証情報を生成:**
```bash
# API Keyを生成
openssl rand -base64 32

# API Secretを生成
openssl rand -base64 32
```

**4. `.env` ファイルを編集して認証情報を設定:**
```env
LIVEKIT_API_KEY=生成したAPIキー
LIVEKIT_API_SECRET=生成したAPIシークレット
```

**5. LiveKitを起動:**
```bash
docker-compose up -d
```

### LiveKitエンドポイント

起動後、以下のエンドポイントが利用可能になります：
- **WebRTC API**: `http://localhost:7880`
- **WHIP エンドポイント**: `http://localhost:7880/whip`
- **WHEP エンドポイント**: `http://localhost:7880/whep`

### アクセストークンの生成

LiveKitに接続するには、アクセストークンが必要です。[LiveKit CLI](https://github.com/livekit/livekit-cli)を使用するか、プログラムで生成できます。

**LiveKit CLIを使用:**
```bash
# LiveKit CLIをインストール
go install github.com/livekit/livekit-cli/cmd/livekit-cli@latest

# パブリッシャートークンを生成（WHIP用）
livekit-cli create-token \
  --api-key <LIVEKIT_API_KEY> \
  --api-secret <LIVEKIT_API_SECRET> \
  --join --room my-room --identity publisher \
  --valid-for 24h

# サブスクライバートークンを生成（WHEP用）
livekit-cli create-token \
  --api-key <LIVEKIT_API_KEY> \
  --api-secret <LIVEKIT_API_SECRET> \
  --join --room my-room --identity subscriber \
  --valid-for 24h
```

### LiveKitを停止

```bash
docker-compose down
```

すべてのデータを削除する場合：
```bash
docker-compose down -v
```
