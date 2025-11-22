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
- Choose “P2P Host”
- Copy the Session ID

**Receiver:**  
- Add `WebRTC Link Source`
- Select `P2P Client`
- Paste Session ID

---

## 🛠️ Build from Source

### Dependencies

**Required:**
- [OBS Studio](https://obsproject.com/) 30.x or later (libobs)
- [CMake](https://cmake.org/) 3.20 or later
- C++17 compatible compiler (MSVC 2019+, GCC 9+, Clang 10+)

**Included as Git Submodules:**
- [libdatachannel](https://github.com/paullouisageneau/libdatachannel) - WebRTC implementation
- [nlohmann-json](https://github.com/nlohmann/json) - JSON library for C++

### Build Instructions

**1. Clone with submodules:**
```bash
git clone --recursive https://github.com/m96-chan/OBS-WebRTC-Link.git
cd OBS-WebRTC-Link
```

If you already cloned without `--recursive`, initialize submodules:
```bash
git submodule update --init --recursive
```

**2. Configure with CMake:**
```bash
mkdir build && cd build
cmake .. -DOBS_INCLUDE_SEARCH_PATH="path/to/obs-studio/include" \
         -DOBS_LIB_SEARCH_PATH="path/to/obs-studio/lib"
```

**3. Build:**
```bash
cmake --build . --config Release
```

**4. Install:**
```bash
cmake --install . --config Release
```

### CMake Options

- `OBS_INCLUDE_SEARCH_PATH` - Path to OBS Studio include directory
- `OBS_LIB_SEARCH_PATH` - Path to OBS Studio library directory
- `BUILD_LIBDATACHANNEL` - Build libdatachannel from source (default: ON)
- `LIBDATACHANNEL_DIR` - Custom path to libdatachannel installation (optional)

### Advanced: Using System Libraries

If you have libdatachannel installed system-wide, you can use it instead of submodules:

```bash
cmake .. -DBUILD_LIBDATACHANNEL=OFF \
         -DOBS_INCLUDE_SEARCH_PATH="path/to/obs-studio/include" \
         -DOBS_LIB_SEARCH_PATH="path/to/obs-studio/lib"
```

The project includes a custom CMake Find module (`cmake/FindLibDataChannel.cmake`) that automatically locates the library. See [cmake/README.md](cmake/README.md) for details.

### Dependency Versions

The following dependency versions are locked via Git submodules:
- libdatachannel: Latest stable
- nlohmann-json: Latest stable

To update dependencies:
```bash
git submodule update --remote
```

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
