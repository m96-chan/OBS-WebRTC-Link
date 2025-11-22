# Architecture Overview

This document provides a comprehensive overview of the OBS-WebRTC-Link architecture, design principles, and component relationships.

---

## Table of Contents

1. [High-Level Architecture](#high-level-architecture)
2. [Design Principles](#design-principles)
3. [Component Overview](#component-overview)
4. [Data Flow](#data-flow)
5. [Protocol Support](#protocol-support)
6. [Threading Model](#threading-model)
7. [Error Handling](#error-handling)
8. [Extensibility](#extensibility)

---

## High-Level Architecture

The OBS-WebRTC-Link plugin follows a layered architecture that separates concerns and promotes maintainability:

```
┌─────────────────────────────────────────────────────────────┐
│                    OBS Studio Interface                      │
│  (obs_module_load, obs_register_output, obs_register_source) │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                   Plugin Layer (OBS API)                     │
│                                                               │
│  ┌────────────────────┐          ┌────────────────────┐    │
│  │  WebRTC Output     │          │  WebRTC Source     │    │
│  │  (obs_output_info) │          │  (obs_source_info) │    │
│  └────────────────────┘          └────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│              Application Layer (WebRTC Logic)                │
│                                                               │
│  ┌─────────────┐  ┌─────────────┐  ┌──────────────────┐   │
│  │WebRTCOutput │  │WebRTCSource │  │ReconnectionMgr   │   │
│  │             │  │             │  │                  │   │
│  └─────────────┘  └─────────────┘  └──────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                Core Layer (Protocol Logic)                   │
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │PeerConnection│  │WHIPClient    │  │WHEPClient    │     │
│  │              │  │              │  │              │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│                                                               │
│  ┌──────────────┐                                           │
│  │SignalingClient│                                          │
│  │              │                                           │
│  └──────────────┘                                           │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│              Transport Layer (Network I/O)                   │
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │libdatachannel│  │HTTP Client   │  │WebSocket     │     │
│  │(WebRTC)      │  │(WHIP/WHEP)   │  │(Signaling)   │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

---

## Design Principles

### 1. Separation of Concerns

The architecture strictly separates:
- **OBS Integration** ([src/output/](../src/output/), [src/source/](../src/source/)): Handles OBS-specific APIs
- **WebRTC Logic** ([src/core/](../src/core/)): Pure WebRTC implementation, independent of OBS
- **Protocol Handling**: WHIP/WHEP/P2P protocols isolated in dedicated components

This separation allows:
- Easy testing of core WebRTC logic without OBS
- Reusability of core components in other projects
- Clear boundaries and responsibilities

### 2. Dependency Inversion

Core components depend on abstractions, not concrete implementations:

```cpp
// Abstract transport interface
class SignalingTransport {
public:
    virtual void connect(const std::string& url) = 0;
    virtual void disconnect() = 0;
    virtual void sendMessage(const std::string& message) = 0;
    virtual bool isConnected() const = 0;
};

// Concrete implementations can be injected
SignalingClient(const SignalingConfig& config,
                std::unique_ptr<SignalingTransport> transport);
```

This enables:
- Mock implementations for testing
- Multiple transport backends (WebSocket, HTTP, custom)
- Runtime protocol selection

### 3. PIMPL Pattern

All public classes use the Pointer to Implementation (PIMPL) idiom:

```cpp
class WHIPClient {
public:
    explicit WHIPClient(const WHIPConfig& config);
    // Public interface...

private:
    class Impl;  // Forward declaration
    std::unique_ptr<Impl> impl_;  // Opaque pointer
};
```

Benefits:
- ABI stability (implementation changes don't break binary compatibility)
- Faster compilation (internal dependencies hidden)
- Better encapsulation (implementation details completely hidden)

### 4. Resource Management

All resources follow RAII (Resource Acquisition Is Initialization):
- Automatic cleanup in destructors
- Move semantics for efficient ownership transfer
- No manual resource management required

```cpp
// Resources automatically cleaned up when object goes out of scope
{
    auto client = std::make_unique<WHIPClient>(config);
    client->start();
    // ... use client ...
} // Destructor automatically disconnects and cleans up
```

### 5. Thread Safety

All core components are thread-safe:
- Internal synchronization using mutexes
- Callback execution on dedicated threads
- Atomic operations for state management

---

## Component Overview

### Core Layer

#### PeerConnection ([src/core/peer-connection.hpp](../src/core/peer-connection.hpp))

**Purpose**: Low-level WebRTC peer connection management

**Responsibilities**:
- SDP offer/answer generation
- ICE candidate collection and exchange
- Connection state management
- Media stream handling

**Dependencies**:
- libdatachannel (WebRTC implementation)

**Key Features**:
- Support for STUN/TURN servers
- ICE trickling
- Connection state callbacks
- Thread-safe operations

#### SignalingClient ([src/core/signaling-client.hpp](../src/core/signaling-client.hpp))

**Purpose**: WebSocket-based signaling for P2P connections

**Responsibilities**:
- WebSocket connection management
- JSON message serialization/deserialization
- Offer/Answer relay between peers
- ICE candidate relay

**Dependencies**:
- SignalingTransport (abstraction)
- nlohmann-json (JSON parsing)

**Key Features**:
- Pluggable transport layer
- Automatic JSON encoding/decoding
- Error handling and retry logic

#### WHIPClient ([src/core/whip-client.hpp](../src/core/whip-client.hpp))

**Purpose**: WHIP (WebRTC-HTTP Ingestion Protocol) implementation for sending media

**Responsibilities**:
- HTTP POST for SDP offer/answer exchange
- HTTP PATCH for ICE candidate trickle
- HTTP DELETE for session termination
- Bearer token authentication

**Dependencies**:
- HTTP client library

**Key Features**:
- WHIP protocol compliance
- Token-based authentication
- Location header parsing
- Error response handling

#### WHEPClient ([src/core/whep-client.hpp](../src/core/whep-client.hpp))

**Purpose**: WHEP (WebRTC-HTTP Egress Protocol) implementation for receiving media

**Responsibilities**:
- HTTP POST for SDP offer/answer exchange
- HTTP PATCH for ICE candidate trickle
- HTTP DELETE for session termination
- Bearer token authentication

**Dependencies**:
- HTTP client library

**Key Features**:
- WHEP protocol compliance
- Token-based authentication
- Resource URL management
- Error response handling

#### ReconnectionManager ([src/core/reconnection-manager.hpp](../src/core/reconnection-manager.hpp))

**Purpose**: Automatic reconnection with exponential backoff

**Responsibilities**:
- Reconnection scheduling
- Exponential backoff calculation
- Retry limit enforcement
- State management

**Dependencies**: None (pure logic)

**Key Features**:
- Exponential backoff: delay = initialDelay × 2^retryCount
- Configurable max delay cap
- Cancellation support
- Thread-safe operations

### Application Layer

#### WebRTCOutput ([src/output/webrtc-output.hpp](../src/output/webrtc-output.hpp))

**Purpose**: OBS output implementation for sending streams via WebRTC

**Responsibilities**:
- Receive encoded frames from OBS
- Send frames via WebRTC using WHIP
- Manage connection lifecycle
- Handle errors and reconnection

**Dependencies**:
- WHIPClient or PeerConnection (depending on mode)
- ReconnectionManager

**Key Features**:
- Multiple codec support (H.264, VP8, VP9, AV1)
- Hardware encoder support (NVENC, AMF, QuickSync)
- Bitrate control
- Automatic reconnection

#### WebRTCSource ([src/source/webrtc-source.hpp](../src/source/webrtc-source.hpp))

**Purpose**: OBS source implementation for receiving streams via WebRTC

**Responsibilities**:
- Receive media frames via WebRTC using WHEP
- Decode frames and provide to OBS
- Manage connection lifecycle
- Handle errors and reconnection

**Dependencies**:
- WHEPClient or PeerConnection (depending on mode)
- ReconnectionManager

**Key Features**:
- Multiple codec support
- Hardware decoder support
- Texture rendering for OBS
- Automatic reconnection

---

## Data Flow

### Output Flow (Sending Stream)

```
OBS Program Output
       ↓
 [Encoder Thread]
       ↓
 Encoded Packets (H.264/VP8/VP9/AV1)
       ↓
 WebRTCOutput::sendPacket()
       ↓
 WHIPClient::sendOffer() → SFU Server
       ↓
 PeerConnection::addTrack()
       ↓
 libdatachannel (WebRTC)
       ↓
 Network (RTP/SRTP)
```

### Source Flow (Receiving Stream)

```
Network (RTP/SRTP)
       ↓
 libdatachannel (WebRTC)
       ↓
 PeerConnection::onTrack()
       ↓
 WHEPClient::sendOffer() ← SFU Server
       ↓
 WebRTCSource::onFrame()
       ↓
 [Decoder Thread]
       ↓
 Decoded Frames (YUV/RGB)
       ↓
 OBS Source Render
```

---

## Protocol Support

### SFU Mode (WHIP/WHEP)

**Best for**: Internet streaming, multi-viewer scenarios, complex networks

```
┌─────────┐   WHIP    ┌─────────┐   WHEP   ┌─────────┐
│   OBS   │ ────────→ │   SFU   │ ←─────── │   OBS   │
│ Output  │           │LiveKit/ │          │ Source  │
│         │           │  SRS    │          │         │
└─────────┘           └─────────┘          └─────────┘
```

**Flow**:
1. Output sends SDP offer via HTTP POST to WHIP endpoint
2. SFU responds with SDP answer
3. ICE candidates exchanged via HTTP PATCH (trickle ICE)
4. Media flows via WebRTC (RTP/SRTP)
5. Source sends SDP offer via HTTP POST to WHEP endpoint
6. SFU responds with SDP answer
7. ICE candidates exchanged via HTTP PATCH
8. Media received via WebRTC

**Advantages**:
- Stable connections through NAT/firewalls
- Multiple viewers supported
- Server-side recording/processing
- Scalability

### P2P Mode (Direct Connection)

**Best for**: LAN scenarios, 1-on-1 connections, lowest latency

```
┌─────────┐           ┌─────────┐
│   OBS   │ ←──P2P──→ │   OBS   │
│  Host   │           │ Client  │
│         │           │         │
└─────────┘           └─────────┘
```

**Flow**:
1. Host creates offer and shares session ID
2. Client connects with session ID
3. Signaling server relays SDP and ICE candidates
4. Direct peer-to-peer media connection established
5. Media flows directly between peers

**Advantages**:
- Ultra-low latency
- No server bandwidth costs
- Direct connection
- Simple setup for LAN

---

## Threading Model

### OBS Threads

- **Main Thread**: OBS UI and plugin initialization
- **Video Thread**: Video frame encoding and output
- **Audio Thread**: Audio frame processing

### Plugin Threads

- **Network Thread**: HTTP requests, WebSocket communication
- **WebRTC Thread**: libdatachannel internal processing
- **Callback Thread**: User callbacks execution

### Synchronization

All public APIs are thread-safe:

```cpp
class WHIPClient::Impl {
private:
    mutable std::mutex mutex_;

public:
    void sendOffer(const std::string& sdp) {
        std::lock_guard<std::mutex> lock(mutex_);
        // Thread-safe implementation
    }
};
```

### Callback Execution

Callbacks may be executed on different threads:
- User code should be thread-safe
- Use queues for cross-thread communication
- Minimize work in callbacks

```cpp
config.onError = [this](const std::string& error) {
    // This may be called from network thread
    // Queue error for main thread processing
    errorQueue_.push(error);
};
```

---

## Error Handling

### Error Categories

1. **Configuration Errors**: Invalid parameters, missing required fields
   - Thrown as `std::invalid_argument`
   - Detected at construction time

2. **Network Errors**: Connection failures, timeouts, DNS errors
   - Thrown as `std::runtime_error`
   - Reported via error callbacks
   - Trigger reconnection if enabled

3. **Protocol Errors**: Invalid SDP, malformed messages, HTTP errors
   - Thrown as `std::runtime_error`
   - Logged for debugging
   - May be recoverable

4. **Resource Errors**: Memory allocation failures, system limits
   - Thrown as `std::bad_alloc` or `std::runtime_error`
   - Usually non-recoverable

### Error Propagation

```cpp
// Synchronous errors thrown as exceptions
try {
    client->sendOffer(sdp);
} catch (const std::runtime_error& e) {
    // Handle error
}

// Asynchronous errors via callbacks
config.onError = [](const std::string& error) {
    // Handle error
};
```

### Reconnection Strategy

```
Connection Failed
       ↓
 ReconnectionManager::scheduleReconnect()
       ↓
 Wait (initialDelay × 2^retryCount)
       ↓
 Reconnect Callback
       ↓
 Success? → Reset retry count
       ↓
 Failure? → Retry (up to maxRetries)
       ↓
 Max Retries? → Give up, notify error
```

---

## Extensibility

### Adding New Signaling Protocols

Implement the `SignalingTransport` interface:

```cpp
class CustomSignalingTransport : public SignalingTransport {
public:
    void connect(const std::string& url) override {
        // Custom connection logic
    }

    void disconnect() override {
        // Custom disconnection logic
    }

    void sendMessage(const std::string& message) override {
        // Custom message sending logic
    }

    bool isConnected() const override {
        // Return connection status
    }
};

// Use custom transport
auto transport = std::make_unique<CustomSignalingTransport>();
SignalingClient client(config, std::move(transport));
```

### Adding New Codecs

Extend the codec enumerations:

```cpp
enum class VideoCodec {
    H264,
    VP8,
    VP9,
    AV1,
    H265  // New codec
};
```

Update encoder/decoder selection logic in implementation.

### Adding New SFU Backends

WHIP/WHEP clients are protocol-compliant and should work with any WHIP/WHEP server:
- LiveKit (tested)
- SRS (compatible)
- Janus (compatible)
- Custom implementations

No code changes required if the server is WHIP/WHEP compliant.

---

## Component Dependencies

```
WebRTCOutput
    ├── WHIPClient
    │   └── HTTP Client
    ├── PeerConnection
    │   └── libdatachannel
    └── ReconnectionManager

WebRTCSource
    ├── WHEPClient
    │   └── HTTP Client
    ├── PeerConnection
    │   └── libdatachannel
    └── ReconnectionManager

SignalingClient
    ├── SignalingTransport
    │   └── WebSocket Client
    └── nlohmann-json

PeerConnection
    └── libdatachannel
        ├── OpenSSL (DTLS/SRTP)
        ├── libsrtp (SRTP)
        └── usrsctp (SCTP/DataChannels)
```

---

## Build System Architecture

### CMake Structure

```
CMakeLists.txt (root)
    ├── Core library (obs-webrtc-core)
    ├── OBS plugin (obs-webrtc-link)
    └── Tests (obs-webrtc-core-tests)
```

### Dependency Management

- **Submodules**: libdatachannel, nlohmann-json
- **CMake FetchContent**: Google Test (tests only)
- **System Libraries**: OBS Studio (via find_package)

### Build Options

- `BUILD_TESTS_ONLY`: Build only tests without OBS SDK
- `BUILD_LIBDATACHANNEL`: Build libdatachannel from source
- `OBS_INCLUDE_SEARCH_PATH`: Path to OBS include directory
- `OBS_LIB_SEARCH_PATH`: Path to OBS library directory

---

**See Also**:
- [API Reference](API-REFERENCE.md)
- [Testing Guide](TESTING.md)
- [Project Structure](OBS-WebRTC-Link-Project-Structure.md)
- [Contributing Guidelines](CONTRIBUTING.md)
