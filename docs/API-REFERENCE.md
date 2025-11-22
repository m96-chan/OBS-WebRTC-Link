# API Reference

This document provides detailed API documentation for all major components in the OBS-WebRTC-Link project.

---

## Table of Contents

1. [Core Components](#core-components)
   - [PeerConnection](#peerconnection)
   - [SignalingClient](#signalingclient)
   - [WHIPClient](#whipclient)
   - [WHEPClient](#whepclient)
   - [ReconnectionManager](#reconnectionmanager)
2. [Output Components](#output-components)
   - [WebRTCOutput](#webrtcoutput)
3. [Source Components](#source-components)
   - [WebRTCSource](#webrtcsource)
4. [Data Structures](#data-structures)
5. [Enumerations](#enumerations)

---

## Core Components

### PeerConnection

**File**: [src/core/peer-connection.hpp](../src/core/peer-connection.hpp)

WebRTC PeerConnection wrapper using libdatachannel. Provides a clean C++ interface for WebRTC peer connections, wrapping libdatachannel's C++ API. Designed to be OBS-independent and thread-safe.

#### Constructor

```cpp
explicit PeerConnection(const PeerConnectionConfig& config);
```

**Parameters**:
- `config`: Configuration for the peer connection

**Throws**: `std::runtime_error` if initialization fails

#### Methods

##### createOffer()

```cpp
void createOffer();
```

Create an SDP offer. Initiates the connection process. The generated offer will be delivered via the `localDescriptionCallback`.

**Throws**: `std::runtime_error` if offer creation fails

##### createAnswer()

```cpp
void createAnswer();
```

Create an SDP answer in response to a received offer. The generated answer will be delivered via the `localDescriptionCallback`.

**Throws**: `std::runtime_error` if answer creation fails

##### setRemoteDescription()

```cpp
void setRemoteDescription(SdpType type, const std::string& sdp);
```

Set remote description (offer or answer).

**Parameters**:
- `type`: SDP type (Offer or Answer)
- `sdp`: SDP content

**Throws**: `std::runtime_error` if setting remote description fails

##### addIceCandidate()

```cpp
void addIceCandidate(const std::string& candidate, const std::string& mid);
```

Add a remote ICE candidate.

**Parameters**:
- `candidate`: ICE candidate string
- `mid`: Media stream identification tag

**Throws**: `std::runtime_error` if adding candidate fails

##### getState()

```cpp
ConnectionState getState() const;
```

Get current connection state.

**Returns**: Current connection state

##### isConnected()

```cpp
bool isConnected() const;
```

Check if connection is established.

**Returns**: `true` if connected or completed

##### close()

```cpp
void close();
```

Close the peer connection gracefully. After calling `close()`, the PeerConnection should not be used.

##### getLocalDescription()

```cpp
std::string getLocalDescription() const;
```

Get local description (for debugging).

**Returns**: Local SDP string, or empty if not set

##### getRemoteDescription()

```cpp
std::string getRemoteDescription() const;
```

Get remote description (for debugging).

**Returns**: Remote SDP string, or empty if not set

#### Configuration Structure

```cpp
struct PeerConnectionConfig {
    std::vector<std::string> iceServers;
    LogCallback logCallback;
    StateChangeCallback stateCallback;
    IceCandidateCallback iceCandidateCallback;
    LocalDescriptionCallback localDescriptionCallback;
};
```

#### Example Usage

```cpp
PeerConnectionConfig config;
config.iceServers = {"stun:stun.l.google.com:19302"};
config.logCallback = [](LogLevel level, const std::string& msg) {
    std::cout << msg << std::endl;
};

auto pc = std::make_unique<PeerConnection>(config);
pc->createOffer();
```

---

### SignalingClient

**File**: [src/core/signaling-client.hpp](../src/core/signaling-client.hpp)

WebRTC Signaling Client for Offer/Answer exchange. Provides a clean abstraction for WebRTC signaling, handling Offer/Answer and ICE candidate exchange. Supports multiple signaling protocols through a pluggable transport layer.

#### Constructors

```cpp
explicit SignalingClient(const SignalingConfig& config);
```

```cpp
SignalingClient(const SignalingConfig& config,
                std::unique_ptr<SignalingTransport> transport);
```

**Parameters**:
- `config`: Configuration for the signaling client
- `transport`: (Optional) Custom transport implementation for testing

**Throws**: `std::invalid_argument` if config is invalid

#### Methods

##### connect()

```cpp
void connect();
```

Connect to the signaling server.

**Throws**: `std::runtime_error` if connection fails

##### disconnect()

```cpp
void disconnect();
```

Disconnect from the signaling server.

##### isConnected()

```cpp
bool isConnected() const;
```

Check if connected to signaling server.

**Returns**: `true` if connected

##### sendOffer()

```cpp
void sendOffer(const std::string& sdp);
```

Send an SDP offer to the remote peer.

**Parameters**:
- `sdp`: SDP offer string

**Throws**: `std::runtime_error` if not connected or send fails

##### sendAnswer()

```cpp
void sendAnswer(const std::string& sdp);
```

Send an SDP answer to the remote peer.

**Parameters**:
- `sdp`: SDP answer string

**Throws**: `std::runtime_error` if not connected or send fails

##### sendIceCandidate()

```cpp
void sendIceCandidate(const std::string& candidate, const std::string& mid);
```

Send an ICE candidate to the remote peer.

**Parameters**:
- `candidate`: ICE candidate string
- `mid`: Media stream identification tag

**Throws**: `std::runtime_error` if not connected or send fails

##### handleMessage()

```cpp
void handleMessage(const std::string& message);
```

Handle incoming message from signaling server. Parses the message and calls appropriate callbacks.

**Parameters**:
- `message`: JSON-encoded message

#### Configuration Structure

```cpp
struct SignalingConfig {
    std::string url;
    SignalingConnectedCallback onConnected;
    SignalingDisconnectedCallback onDisconnected;
    SignalingErrorCallback onError;
    SignalingOfferCallback onOffer;
    SignalingAnswerCallback onAnswer;
    SignalingIceCandidateCallback onIceCandidate;
};
```

#### Example Usage

```cpp
SignalingConfig config;
config.url = "wss://signaling.example.com";
config.onOffer = [](const std::string& sdp) {
    // Handle received offer
};
config.onAnswer = [](const std::string& sdp) {
    // Handle received answer
};

auto client = std::make_unique<SignalingClient>(config);
client->connect();
client->sendOffer(myOfferSdp);
```

---

### WHIPClient

**File**: [src/core/whip-client.hpp](../src/core/whip-client.hpp)

WHIP (WebRTC-HTTP Ingestion Protocol) Client for sending media. Implements the WHIP protocol (draft-ietf-wish-whip) for ingesting media into SFU servers like LiveKit.

#### Constructor

```cpp
explicit WHIPClient(const WHIPConfig& config);
```

**Parameters**:
- `config`: Configuration for the WHIP client

**Throws**: `std::invalid_argument` if config is invalid

#### Methods

##### sendOffer()

```cpp
std::string sendOffer(const std::string& sdp);
```

Send SDP offer to WHIP server and receive answer.

**Parameters**:
- `sdp`: SDP offer string

**Returns**: SDP answer string from server

**Throws**:
- `std::invalid_argument` if SDP is empty
- `std::runtime_error` if HTTP request fails

##### sendIceCandidate()

```cpp
void sendIceCandidate(const std::string& candidate, const std::string& mid);
```

Send ICE candidate to WHIP server via PATCH.

**Parameters**:
- `candidate`: ICE candidate string
- `mid`: Media stream identification tag

**Throws**: `std::runtime_error` if not connected or send fails

##### disconnect()

```cpp
void disconnect();
```

Disconnect from WHIP server. Sends HTTP DELETE to resource URL.

##### isConnected()

```cpp
bool isConnected() const;
```

Check if connected to WHIP server.

**Returns**: `true` if connected

#### Configuration Structure

```cpp
struct WHIPConfig {
    std::string url;
    std::string bearerToken;
    WHIPConnectedCallback onConnected;
    WHIPDisconnectedCallback onDisconnected;
    WHIPErrorCallback onError;
    WHIPIceCandidateCallback onIceCandidate;
};
```

#### Example Usage

```cpp
WHIPConfig config;
config.url = "https://sfu.livekit.cloud/whip";
config.bearerToken = "my-auth-token";
config.onConnected = []() {
    // Connection established
};

auto client = std::make_unique<WHIPClient>(config);
std::string answer = client->sendOffer(myOfferSdp);
client->sendIceCandidate(candidate, mid);
```

---

### WHEPClient

**File**: [src/core/whep-client.hpp](../src/core/whep-client.hpp)

WHEP (WebRTC-HTTP Egress Protocol) Client for receiving media. Implements the WHEP protocol (draft-murillo-whep) for receiving media from SFU servers like LiveKit.

#### Constructor

```cpp
explicit WHEPClient(const WHEPConfig& config);
```

**Parameters**:
- `config`: Configuration for the WHEP client

**Throws**: `std::invalid_argument` if config is invalid

#### Methods

##### sendOffer()

```cpp
std::string sendOffer(const std::string& sdp);
```

Send SDP offer to WHEP server and receive answer.

**Parameters**:
- `sdp`: SDP offer string

**Returns**: SDP answer string from server

**Throws**:
- `std::invalid_argument` if SDP is empty
- `std::runtime_error` if HTTP request fails

##### sendIceCandidate()

```cpp
void sendIceCandidate(const std::string& candidate, const std::string& mid);
```

Send ICE candidate to WHEP server via PATCH.

**Parameters**:
- `candidate`: ICE candidate string
- `mid`: Media stream identification tag

**Throws**: `std::runtime_error` if not connected or send fails

##### disconnect()

```cpp
void disconnect();
```

Disconnect from WHEP server. Sends HTTP DELETE to resource URL.

##### isConnected()

```cpp
bool isConnected() const;
```

Check if connected to WHEP server.

**Returns**: `true` if connected

#### Configuration Structure

```cpp
struct WHEPConfig {
    std::string url;
    std::string bearerToken;
    WHEPConnectedCallback onConnected;
    WHEPDisconnectedCallback onDisconnected;
    WHEPErrorCallback onError;
    WHEPIceCandidateCallback onIceCandidate;
};
```

#### Example Usage

```cpp
WHEPConfig config;
config.url = "https://sfu.livekit.cloud/whep";
config.bearerToken = "my-auth-token";
config.onConnected = []() {
    // Connection established
};

auto client = std::make_unique<WHEPClient>(config);
std::string answer = client->sendOffer(myOfferSdp);
client->sendIceCandidate(candidate, mid);
```

---

### ReconnectionManager

**File**: [src/core/reconnection-manager.hpp](../src/core/reconnection-manager.hpp)

Automatic reconnection manager with exponential backoff. Manages automatic reconnection with exponential backoff for WebRTC connections.

#### Constructor

```cpp
explicit ReconnectionManager(const ReconnectionConfig& config);
```

**Parameters**:
- `config`: Configuration for reconnection

#### Methods

##### scheduleReconnect()

```cpp
bool scheduleReconnect();
```

Schedule a reconnection attempt. Schedules reconnection after a delay calculated using exponential backoff. If max retries is reached, no reconnection is scheduled.

**Returns**: `true` if reconnection was scheduled, `false` if max retries reached

##### cancel()

```cpp
void cancel();
```

Cancel pending reconnection. Cancels any scheduled reconnection attempt but does not reset the retry count.

##### reset()

```cpp
void reset();
```

Reset reconnection state. Resets the retry count to 0 and cancels any pending reconnection.

##### onConnectionSuccess()

```cpp
void onConnectionSuccess();
```

Notify manager of successful connection. Resets the retry count to 0.

##### isReconnecting()

```cpp
bool isReconnecting() const;
```

Check if reconnection is in progress.

**Returns**: `true` if reconnection is scheduled or in progress

##### getRetryCount()

```cpp
int getRetryCount() const;
```

Get current retry count.

**Returns**: Current number of retry attempts

##### getNextDelay()

```cpp
int64_t getNextDelay() const;
```

Get next delay duration.

**Returns**: Next delay in milliseconds

#### Configuration Structure

```cpp
struct ReconnectionConfig {
    int maxRetries = 5;
    int64_t initialDelayMs = 1000;
    int64_t maxDelayMs = 30000;
    ReconnectCallback reconnectCallback;
    ReconnectionStateCallback stateCallback;
};
```

#### Example Usage

```cpp
ReconnectionConfig config;
config.maxRetries = 5;
config.initialDelayMs = 1000;
config.maxDelayMs = 30000;
config.reconnectCallback = [this]() {
    this->connect();
};

ReconnectionManager manager(config);

// On connection failure:
manager.scheduleReconnect();

// On successful reconnection:
manager.onConnectionSuccess();
```

---

## Output Components

### WebRTCOutput

**File**: [src/output/webrtc-output.hpp](../src/output/webrtc-output.hpp)

WebRTC Output implementation for OBS Studio. Provides WebRTC output functionality, allowing Program output to be sent via WebRTC using WHIP protocol.

#### Constructor

```cpp
explicit WebRTCOutput(const WebRTCOutputConfig& config);
```

**Parameters**:
- `config`: Configuration for the output

**Throws**: `std::runtime_error` if initialization fails

#### Methods

##### start()

```cpp
bool start();
```

Start the WebRTC output. Initiates the WebRTC connection using WHIP protocol.

**Returns**: `true` if started successfully, `false` otherwise

##### stop()

```cpp
void stop();
```

Stop the WebRTC output. Stops the WebRTC connection and releases resources.

##### isActive()

```cpp
bool isActive() const;
```

Check if output is active.

**Returns**: `true` if output is active and ready to send packets

##### sendPacket()

```cpp
void sendPacket(const EncodedPacket& packet);
```

Send an encoded packet.

**Parameters**:
- `packet`: Encoded video or audio packet

**Throws**: `std::runtime_error` if output is not active

##### getVideoBitrate() / setVideoBitrate()

```cpp
int getVideoBitrate() const;
void setVideoBitrate(int bitrate);
```

Get or set video bitrate.

**Parameters**:
- `bitrate`: Video bitrate in kbps

**Returns**: Video bitrate in kbps

##### getAudioBitrate() / setAudioBitrate()

```cpp
int getAudioBitrate() const;
void setAudioBitrate(int bitrate);
```

Get or set audio bitrate.

**Parameters**:
- `bitrate`: Audio bitrate in kbps

**Returns**: Audio bitrate in kbps

#### Configuration Structure

```cpp
struct WebRTCOutputConfig {
    std::string serverUrl;
    VideoCodec videoCodec = VideoCodec::H264;
    AudioCodec audioCodec = AudioCodec::Opus;
    int videoBitrate = 2500;  // kbps
    int audioBitrate = 128;   // kbps
    ErrorCallback errorCallback;
    StateCallback stateCallback;
    bool enableAutoReconnect = true;
    int maxReconnectRetries = 5;
    int reconnectInitialDelayMs = 1000;
    int reconnectMaxDelayMs = 30000;
};
```

#### Example Usage

```cpp
WebRTCOutputConfig config;
config.serverUrl = "http://localhost:8080/whip";
config.videoCodec = VideoCodec::H264;
config.audioCodec = AudioCodec::Opus;
config.errorCallback = [](const std::string& error) {
    std::cerr << "Error: " << error << std::endl;
};

WebRTCOutput output(config);
output.start();

EncodedPacket packet;
packet.type = PacketType::Video;
packet.data = encodedData;
packet.timestamp = pts;
packet.keyframe = isKeyframe;
output.sendPacket(packet);

output.stop();
```

---

## Source Components

### WebRTCSource

**File**: [src/source/webrtc-source.hpp](../src/source/webrtc-source.hpp)

WebRTC Source implementation for receiving streams. Receives video/audio from WebRTC connections using WHEP protocol.

#### Constructor

```cpp
explicit WebRTCSource(const WebRTCSourceConfig& config);
```

**Parameters**:
- `config`: Source configuration

#### Methods

##### start()

```cpp
bool start();
```

Start receiving stream.

**Returns**: `true` if started successfully, `false` otherwise

##### stop()

```cpp
void stop();
```

Stop receiving stream.

##### isActive()

```cpp
bool isActive() const;
```

Check if source is active.

**Returns**: `true` if active, `false` otherwise

##### getConnectionState()

```cpp
ConnectionState getConnectionState() const;
```

Get current connection state.

**Returns**: Current connection state

#### Configuration Structure

```cpp
struct WebRTCSourceConfig {
    std::string serverUrl;
    VideoCodec videoCodec;
    AudioCodec audioCodec;
    std::function<void(const VideoFrame&)> videoCallback;
    std::function<void(const AudioFrame&)> audioCallback;
    std::function<void(const std::string&)> errorCallback;
    std::function<void(ConnectionState)> stateCallback;
    bool enableAutoReconnect = true;
    int maxReconnectRetries = 5;
    int reconnectInitialDelayMs = 1000;
    int reconnectMaxDelayMs = 30000;
};
```

#### Example Usage

```cpp
WebRTCSourceConfig config;
config.serverUrl = "http://localhost:8080/whep";
config.videoCodec = VideoCodec::H264;
config.audioCodec = AudioCodec::Opus;
config.videoCallback = [](const VideoFrame& frame) {
    // Process video frame
};
config.audioCallback = [](const AudioFrame& frame) {
    // Process audio frame
};

WebRTCSource source(config);
source.start();
```

---

## Data Structures

### EncodedPacket

```cpp
struct EncodedPacket {
    PacketType type;
    std::vector<uint8_t> data;
    int64_t timestamp;
    bool keyframe;
};
```

### VideoFrame

```cpp
struct VideoFrame {
    std::vector<uint8_t> data;
    uint32_t width;
    uint32_t height;
    uint64_t timestamp;
    bool keyframe;
};
```

### AudioFrame

```cpp
struct AudioFrame {
    std::vector<uint8_t> data;
    uint32_t sampleRate;
    uint32_t channels;
    uint64_t timestamp;
};
```

### HTTPRequest

```cpp
struct HTTPRequest {
    std::map<std::string, std::string> headers;
    std::string body;
    std::string contentType;
};
```

### HTTPResponse

```cpp
struct HTTPResponse {
    int statusCode;
    std::map<std::string, std::string> headers;
    std::string body;
};
```

---

## Enumerations

### LogLevel

```cpp
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};
```

### ConnectionState (PeerConnection)

```cpp
enum class ConnectionState {
    New,
    Checking,
    Connected,
    Completed,
    Failed,
    Disconnected,
    Closed
};
```

### ConnectionState (Source)

```cpp
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed
};
```

### SdpType

```cpp
enum class SdpType {
    Offer,
    Answer
};
```

### VideoCodec

```cpp
enum class VideoCodec {
    H264,
    VP8,
    VP9,
    AV1
};
```

### AudioCodec (Output)

```cpp
enum class AudioCodec {
    Opus,
    AAC
};
```

### AudioCodec (Source)

```cpp
enum class AudioCodec {
    Opus,
    PCM
};
```

### PacketType

```cpp
enum class PacketType {
    Video,
    Audio
};
```

---

## Thread Safety

All core components ([PeerConnection](../src/core/peer-connection.hpp), [SignalingClient](../src/core/signaling-client.hpp), [WHIPClient](../src/core/whip-client.hpp), [WHEPClient](../src/core/whep-client.hpp), [ReconnectionManager](../src/core/reconnection-manager.hpp)) are designed to be thread-safe. Internal synchronization is handled using mutexes and atomic operations where necessary.

## Error Handling

All methods that can fail throw exceptions:
- `std::invalid_argument` - Invalid input parameters
- `std::runtime_error` - Runtime errors (connection failures, protocol errors, etc.)

Always wrap calls in try-catch blocks when error handling is required.

## Memory Management

All components use the PIMPL idiom (Pointer to Implementation) for better encapsulation and ABI stability. Components are non-copyable but support move semantics for efficient ownership transfer.

---

**See Also**:
- [Project Structure](OBS-WebRTC-Link-Project-Structure.md)
- [Testing Guide](TESTING.md)
- [Contributing Guidelines](CONTRIBUTING.md)
