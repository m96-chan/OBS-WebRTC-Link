# OBS-WebRTC-Link

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

Dependencies:
- OBS Studio (libobs)
- LibDataChannel (recommended)

```
git clone --recursive https://github.com/yourname/obs-webrtc-link.git
cd obs-webrtc-link
mkdir build && cd build
cmake .. -DOBS_INCLUDE_SEARCH_PATH="path/to/libobs"
cmake --build . --config Release
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
