# Usage Examples

This document provides detailed, step-by-step examples for common use cases of the OBS-WebRTC-Link plugin.

## Table of Contents

- [Use Case 1: OBS-to-OBS Relay via LiveKit](#use-case-1-obs-to-obs-relay-via-livekit)
- [Use Case 2: Browser to OBS (Guest Input)](#use-case-2-browser-to-obs-guest-input)
- [Use Case 3: Direct P2P Connection](#use-case-3-direct-p2p-connection)
- [Use Case 4: Mobile Device as Wireless Camera](#use-case-4-mobile-device-as-wireless-camera)
- [Use Case 5: Audio-Only Mode for Podcasts](#use-case-5-audio-only-mode-for-podcasts)
- [Troubleshooting](#troubleshooting)

---

## Use Case 1: OBS-to-OBS Relay via LiveKit

### Scenario
Stream from a remote OBS instance (e.g., at home) to your main production OBS (e.g., at a studio) using LiveKit as an SFU relay.

### Prerequisites
- LiveKit server running (see [LiveKit Setup Guide](LIVEKIT_SETUP.md))
- LiveKit API credentials
- Both OBS instances have OBS-WebRTC-Link plugin installed

### Steps

#### On the Sender OBS (Remote Location)

1. **Open OBS Output Settings**
   - Go to `Settings` → `Output`
   - Or use `Tools` → `WebRTC Output Settings`

2. **Configure WHIP Output**
   - **Mode**: Select `SFU (WHIP)`
   - **Server URL**: `https://your-livekit-server.com/whip`
   - **Room Name**: `my-relay-room`
   - **Bearer Token**: Generate a publisher token using LiveKit CLI:
     ```bash
     livekit-cli create-token \
       --api-key <YOUR_API_KEY> \
       --api-secret <YOUR_API_SECRET> \
       --join --room my-relay-room --identity sender \
       --valid-for 24h
     ```
   - **Video Codec**: H.264 (recommended for compatibility)
   - **Video Bitrate**: 2500 kbps
   - **Audio Codec**: Opus
   - **Audio Bitrate**: 128 kbps

3. **Start Output**
   - Click `Start Streaming` or enable WebRTC Output
   - Verify connection status

![WHIP Output Settings](images/screenshots/obs-output-settings-whip.png)
*Screenshot will be added in future release*

#### On the Receiver OBS (Studio)

1. **Add WebRTC Link Source**
   - Click `+` in Sources panel
   - Select `WebRTC Link Source`
   - Name it "Remote OBS Feed"

2. **Configure WHEP Source**
   - **Mode**: Select `SFU (WHEP)`
   - **Server URL**: `https://your-livekit-server.com/whep`
   - **Room Name**: `my-relay-room` (same as sender)
   - **Bearer Token**: Generate a subscriber token:
     ```bash
     livekit-cli create-token \
       --api-key <YOUR_API_KEY> \
       --api-secret <YOUR_API_SECRET> \
       --join --room my-relay-room --identity receiver \
       --valid-for 24h
     ```

3. **Connect**
   - Click `Connect`
   - Video should appear within 2-3 seconds

![WHEP Source Settings](images/screenshots/obs-source-settings-whep.png)
*Screenshot will be added in future release*

### Expected Results
- Low latency: ~1-3 seconds
- Stable connection through firewalls
- Automatic reconnection on network issues

---

## Use Case 2: Browser to OBS (Guest Input)

### Scenario
Allow remote guests to send video from their web browser directly into your OBS scene.

### Prerequisites
- LiveKit server or any WHIP-compatible SFU
- Guest has a modern web browser (Chrome, Firefox, Edge, Safari)

### Steps

#### Setup OBS to Receive

1. **Add WebRTC Link Source**
   - Sources → `+` → `WebRTC Link Source`
   - Name: "Guest Camera"

2. **Configure for WHEP**
   - Mode: `SFU (WHEP)`
   - Server URL: Your SFU's WHEP endpoint
   - Room Name: `guest-room-001`
   - Token: Subscriber token

#### Guest Browser Setup

Provide the guest with a simple HTML page or use LiveKit's web SDK:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Send to OBS</title>
</head>
<body>
    <h1>Send Your Camera to OBS</h1>
    <video id="localVideo" autoplay muted width="640"></video>
    <button id="startBtn">Start Sending</button>

    <script>
        const startBtn = document.getElementById('startBtn');
        const localVideo = document.getElementById('localVideo');

        startBtn.addEventListener('click', async () => {
            // Get user media
            const stream = await navigator.mediaDevices.getUserMedia({
                video: true,
                audio: true
            });
            localVideo.srcObject = stream;

            // WHIP publish logic
            const pc = new RTCPeerConnection({
                iceServers: [{ urls: 'stun:stun.l.google.com:19302' }]
            });

            stream.getTracks().forEach(track => pc.addTrack(track, stream));

            const offer = await pc.createOffer();
            await pc.setLocalDescription(offer);

            // Send WHIP request to server
            const response = await fetch('https://your-sfu.com/whip', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/sdp',
                    'Authorization': 'Bearer <PUBLISHER_TOKEN>'
                },
                body: offer.sdp
            });

            const answer = await response.text();
            await pc.setRemoteDescription({
                type: 'answer',
                sdp: answer
            });

            startBtn.textContent = 'Sending...';
            startBtn.disabled = true;
        });
    </script>
</body>
</html>
```

### Expected Results
- Guest sees their own camera preview
- OBS receives the stream with minimal latency
- Audio and video synchronized

![Browser to OBS Example](images/examples/browser-to-obs.png)
*Diagram will be added in future release*

---

## Use Case 3: Direct P2P Connection

### Scenario
Connect two OBS instances directly without a server (ideal for LAN or point-to-point connections).

### Prerequisites
- Both OBS instances on the same network or with direct connectivity
- Signaling server for initial handshake (can be simple WebSocket server)

### Steps

#### OBS A (Host)

1. **Configure as P2P Host**
   - Add WebRTC Output
   - Mode: `P2P Host`
   - Generate Session ID (automatically created)
   - Copy the Session ID

2. **Share Session ID**
   - Send the Session ID to OBS B via any method (email, chat, etc.)

#### OBS B (Client)

1. **Add WebRTC Link Source**
   - Sources → `+` → `WebRTC Link Source`
   - Name: "P2P Feed"

2. **Configure as P2P Client**
   - Mode: `P2P Client`
   - Paste Session ID from OBS A
   - Click `Connect`

### Expected Results
- Direct peer-to-peer connection
- Ultra-low latency: ~100-500ms
- No server costs
- Requires good network connectivity between peers

![P2P Connection Settings](images/screenshots/obs-source-settings-p2p.png)
*Screenshot will be added in future release*

---

## Use Case 4: Mobile Device as Wireless Camera

### Scenario
Use your smartphone as a wireless camera input to OBS via WebRTC.

### Prerequisites
- Mobile device with camera and browser
- LiveKit or WHIP-compatible server
- OBS-WebRTC-Link plugin installed

### Steps

#### Setup OBS

1. **Add WebRTC Link Source**
   - Name: "Mobile Camera"
   - Mode: `SFU (WHEP)`
   - Configure with server details

#### Mobile Device

1. **Open Browser**
   - Navigate to LiveKit web app or custom WHIP client
   - Or use a simple web page:

```html
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Mobile Camera to OBS</title>
    <style>
        body { margin: 0; padding: 20px; font-family: sans-serif; }
        video { width: 100%; max-width: 640px; }
        button { width: 100%; padding: 15px; font-size: 18px; margin: 10px 0; }
    </style>
</head>
<body>
    <h1>Mobile Camera</h1>
    <video id="preview" autoplay muted playsinline></video>
    <button id="startBtn">Start Sending to OBS</button>
    <button id="flipBtn">Flip Camera</button>

    <script>
        let stream;
        let facingMode = 'user'; // Start with front camera

        const preview = document.getElementById('preview');
        const startBtn = document.getElementById('startBtn');
        const flipBtn = document.getElementById('flipBtn');

        flipBtn.addEventListener('click', async () => {
            facingMode = facingMode === 'user' ? 'environment' : 'user';
            if (stream) {
                stream.getTracks().forEach(track => track.stop());
            }
            await startCamera();
        });

        async function startCamera() {
            stream = await navigator.mediaDevices.getUserMedia({
                video: { facingMode },
                audio: true
            });
            preview.srcObject = stream;
        }

        startBtn.addEventListener('click', async () => {
            if (!stream) await startCamera();

            // WHIP publish logic here
            // ... (similar to browser example)
        });
    </script>
</body>
</html>
```

2. **Grant Camera Permissions**
3. **Click "Start Sending to OBS"**

### Expected Results
- Mobile camera appears in OBS
- Can switch between front/rear cameras
- Low latency suitable for live streaming

![Mobile Camera Example](images/examples/mobile-camera.png)
*Diagram will be added in future release*

---

## Use Case 5: Audio-Only Mode for Podcasts

### Scenario
Receive high-quality audio from remote podcast guests without video.

### Prerequisites
- LiveKit or WHIP-compatible server
- Guest with microphone and browser

### Steps

#### OBS Configuration

1. **Add WebRTC Link Source**
   - Name: "Guest Audio"
   - Mode: `SFU (WHEP)`
   - **Enable Audio-Only Mode**: ✓
   - **Audio Quality**: High
     - Bitrate: 128 kbps
     - Sample Rate: 48 kHz
     - Channels: Stereo
   - **Echo Cancellation**: Enabled

2. **Audio Routing**
   - In Audio Mixer, ensure "Guest Audio" is visible
   - Adjust volume levels as needed

#### Guest Setup

Provide guest with a simple audio-only interface:

```html
<!DOCTYPE html>
<html>
<head>
    <title>Podcast Guest</title>
</head>
<body>
    <h1>Podcast Recording</h1>
    <p>Status: <span id="status">Not connected</span></p>
    <button id="startBtn">Join Podcast</button>

    <script>
        startBtn.addEventListener('click', async () => {
            const stream = await navigator.mediaDevices.getUserMedia({
                video: false,  // Audio only
                audio: {
                    echoCancellation: true,
                    noiseSuppression: true,
                    sampleRate: 48000
                }
            });

            // WHIP publish (audio only)
            // ...

            document.getElementById('status').textContent = 'Connected';
        });
    </script>
</body>
</html>
```

### Expected Results
- Crystal clear audio with no video overhead
- Lower bandwidth usage
- Echo cancellation for clean audio
- Suitable for long podcast recordings

---

## Troubleshooting

### Connection Fails

**Symptom**: Source shows "Connecting..." but never connects

**Solutions**:
1. Verify server URL is correct and accessible
2. Check bearer token is valid and not expired
3. Ensure firewall allows WebRTC traffic (UDP ports)
4. Test server with `curl`:
   ```bash
   curl -X OPTIONS https://your-server.com/whep \
     -H "Authorization: Bearer <TOKEN>"
   ```

### Poor Video Quality

**Symptom**: Video is pixelated or stuttering

**Solutions**:
1. Increase video bitrate in sender settings
2. Check network bandwidth with speed test
3. Use H.264 codec instead of VP8/VP9
4. Reduce resolution if bandwidth is limited

### Audio/Video Out of Sync

**Symptom**: Lip sync issues

**Solutions**:
1. Add audio delay in OBS (Advanced Audio Properties)
2. Typical delay: 100-300ms
3. Test with different codecs

### Firewall/NAT Issues

**Symptom**: Connection works on some networks but not others

**Solutions**:
1. Configure TURN server in ICE servers
2. Verify TURN server credentials
3. Use SFU mode instead of P2P
4. Check corporate firewall rules

### Token Expired

**Symptom**: Connection was working, now fails with 401/403

**Solutions**:
1. Generate new token with LiveKit CLI
2. Update token in OBS settings
3. Increase token validity period

---

## Advanced Configuration

### Custom ICE Servers

For better connectivity, configure custom STUN/TURN servers:

```
STUN Server: stun:stun.l.google.com:19302
TURN Server: turn:turn.example.com:3478
TURN Username: your-username
TURN Credential: your-password
```

### Network Optimization

For best performance:
- **Wired Connection**: Always prefer Ethernet over WiFi
- **Bandwidth**: Ensure at least 5 Mbps upload for HD streaming
- **Latency**: Test with `ping` to server (should be <100ms)
- **Jitter**: Use QoS settings if available on router

### Multi-Source Setup

To receive multiple remote sources:
1. Create separate WebRTC Link Sources for each
2. Use different room names or session IDs
3. Manage in OBS Scenes like normal sources

---

## Getting Help

If you encounter issues not covered here:

1. Check [GitHub Issues](https://github.com/m96-chan/OBS-WebRTC-Link/issues)
2. Review [LiveKit Setup Guide](LIVEKIT_SETUP.md)
3. Consult [API Reference](API-REFERENCE.md)
4. Ask in [GitHub Discussions](https://github.com/m96-chan/OBS-WebRTC-Link/discussions)
