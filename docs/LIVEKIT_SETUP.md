# LiveKit Setup Guide

This guide provides detailed instructions for setting up LiveKit SFU (Selective Forwarding Unit) to use with the OBS-WebRTC-Link plugin.

LiveKit is the recommended SFU for this plugin, offering stable WebRTC connections via WHIP (WebRTC-HTTP Ingestion Protocol) and WHEP (WebRTC-HTTP Egress Protocol).

---

## Table of Contents

- [Overview](#overview)
- [Option 1: LiveKit Cloud (Recommended for Production)](#option-1-livekit-cloud-recommended-for-production)
- [Option 2: Self-Hosted LiveKit with Docker (Development)](#option-2-self-hosted-livekit-with-docker-development)
- [Option 3: Self-Hosted LiveKit (Production)](#option-3-self-hosted-livekit-production)
- [Generating Access Tokens](#generating-access-tokens)
- [Configuring OBS WebRTC Link](#configuring-obs-webrtc-link)
- [Troubleshooting](#troubleshooting)
- [Security Best Practices](#security-best-practices)

---

## Overview

LiveKit is an open-source, scalable WebRTC SFU that supports:
- WHIP/WHEP protocols for standard WebRTC streaming
- Multiple publishers and subscribers in the same room
- Automatic reconnection and error recovery
- Hardware-accelerated video encoding/decoding
- Cloud-native deployment

**Tested LiveKit Versions:**
- LiveKit Server: v1.5.0 or later
- LiveKit CLI: v1.4.0 or later

---

## Option 1: LiveKit Cloud (Recommended for Production)

LiveKit Cloud is a fully managed service that eliminates infrastructure management.

### Step 1: Create a LiveKit Cloud Account

1. Go to [LiveKit Cloud](https://cloud.livekit.io/)
2. Sign up for a free account
3. Verify your email address

### Step 2: Create a Project

1. Log in to the LiveKit Cloud dashboard
2. Click **Create Project**
3. Enter a project name (e.g., "OBS-WebRTC-Link")
4. Select your region (choose closest to your location for lower latency)
5. Click **Create**

### Step 3: Get API Credentials

1. Navigate to your project dashboard
2. Go to **Settings** → **API Keys**
3. Copy your **API Key** and **API Secret**
4. Store these securely (you'll need them for token generation)

### Step 4: Note Your WebSocket URL

Your WebSocket URL will be in the format:
```
wss://<your-project>.livekit.cloud
```

This URL is used for the LiveKit SDK. For WHIP/WHEP, you'll use:
```
https://<your-project>.livekit.cloud/whip
https://<your-project>.livekit.cloud/whep
```

---

## Option 2: Self-Hosted LiveKit with Docker (Development)

For local development and testing, you can run LiveKit using Docker.

### Prerequisites

- Docker Desktop (Windows/macOS) or Docker Engine (Linux)
- Docker Compose
- OpenSSL (for generating credentials)

### Step 1: Navigate to the Docker Directory

The project includes a pre-configured Docker setup:

```bash
cd docker/livekit
```

### Step 2: Create Environment File

Copy the example environment file:

```bash
cp .env.example .env
```

### Step 3: Generate API Credentials

Generate secure API credentials using OpenSSL:

```bash
# Generate API Key (32 bytes, base64 encoded)
openssl rand -base64 32

# Generate API Secret (32 bytes, base64 encoded)
openssl rand -base64 32
```

### Step 4: Configure Environment Variables

Edit the `.env` file and set your generated credentials:

```env
# LiveKit Configuration
LIVEKIT_API_KEY=your-generated-api-key-here
LIVEKIT_API_SECRET=your-generated-api-secret-here

# Port Configuration (default: 7880)
LIVEKIT_PORT=7880

# Domain (for local development, use localhost)
LIVEKIT_DOMAIN=localhost
```

### Step 5: Start LiveKit

Start the LiveKit server using Docker Compose:

```bash
docker-compose up -d
```

To view logs:
```bash
docker-compose logs -f
```

### Step 6: Verify LiveKit is Running

LiveKit will be available at the following endpoints:

- **WebRTC API**: `http://localhost:7880`
- **WHIP Endpoint**: `http://localhost:7880/whip`
- **WHEP Endpoint**: `http://localhost:7880/whep`

Test the health endpoint:
```bash
curl http://localhost:7880/health
```

Expected response:
```json
{"status":"ok"}
```

### Step 7: Stopping LiveKit

Stop the LiveKit server:
```bash
docker-compose down
```

To remove all data:
```bash
docker-compose down -v
```

---

## Option 3: Self-Hosted LiveKit (Production)

For production deployments, follow the official LiveKit deployment guides:

### Cloud Providers

- **AWS**: [Deploy on AWS](https://docs.livekit.io/deploy/aws/)
- **Google Cloud**: [Deploy on GCP](https://docs.livekit.io/deploy/gcp/)
- **Azure**: [Deploy on Azure](https://docs.livekit.io/deploy/azure/)
- **DigitalOcean**: [Deploy on DigitalOcean](https://docs.livekit.io/deploy/digitalocean/)

### Kubernetes

For Kubernetes deployments:
```bash
# Add LiveKit Helm repository
helm repo add livekit https://helm.livekit.io

# Install LiveKit
helm install livekit livekit/livekit-server \
  --set livekit.apiKey=<your-api-key> \
  --set livekit.apiSecret=<your-api-secret>
```

See [LiveKit Kubernetes Guide](https://docs.livekit.io/deploy/kubernetes/) for details.

### Firewall Configuration

Ensure the following ports are open:

| Port | Protocol | Purpose |
|------|----------|---------|
| 7880 | TCP | HTTP API and WHIP/WHEP |
| 7881 | TCP | WebSocket (alternative) |
| 50000-60000 | UDP | WebRTC media (configurable) |

---

## Generating Access Tokens

LiveKit uses JWT (JSON Web Token) for authentication. You need to generate tokens for publishers (WHIP) and subscribers (WHEP).

### Method 1: Using LiveKit CLI (Recommended)

#### Install LiveKit CLI

**macOS/Linux:**
```bash
# Using Homebrew
brew install livekit

# Or using Go
go install github.com/livekit/livekit-cli/cmd/livekit-cli@latest
```

**Windows:**
```powershell
# Using Go
go install github.com/livekit/livekit-cli/cmd/livekit-cli@latest
```

Or download pre-built binaries from [LiveKit CLI Releases](https://github.com/livekit/livekit-cli/releases).

#### Generate Publisher Token (for WHIP)

```bash
livekit-cli create-token \
  --api-key <LIVEKIT_API_KEY> \
  --api-secret <LIVEKIT_API_SECRET> \
  --join --room my-room --identity publisher \
  --valid-for 24h
```

**Token Permissions:**
- `--join`: Allow joining the room
- `--room`: Specify room name
- `--identity`: Set participant identity
- `--valid-for`: Token expiration time

**Example:**
```bash
livekit-cli create-token \
  --api-key "myapikey123" \
  --api-secret "mysecret456" \
  --join --room obs-stream --identity obs-publisher \
  --valid-for 24h
```

Output:
```
eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

#### Generate Subscriber Token (for WHEP)

```bash
livekit-cli create-token \
  --api-key <LIVEKIT_API_KEY> \
  --api-secret <LIVEKIT_API_SECRET> \
  --join --room my-room --identity subscriber \
  --valid-for 24h
```

### Method 2: Using Web-Based Token Generator

LiveKit provides a web-based token generator:

1. Go to [LiveKit Token Generator](https://token.livekit.io/)
2. Enter your API Key and API Secret
3. Configure permissions:
   - Room Name: e.g., "obs-stream"
   - Participant Identity: e.g., "publisher" or "subscriber"
   - Permissions: Select appropriate permissions
4. Click **Generate Token**
5. Copy the generated token

### Method 3: Programmatic Token Generation

For automated workflows, generate tokens programmatically using LiveKit SDKs:

**Node.js:**
```javascript
const { AccessToken } = require('livekit-server-sdk');

const token = new AccessToken('api-key', 'api-secret', {
  identity: 'publisher',
  name: 'OBS Publisher',
});

token.addGrant({
  room: 'obs-stream',
  roomJoin: true,
  canPublish: true,
  canSubscribe: false,
});

const jwt = token.toJwt();
console.log(jwt);
```

**Python:**
```python
from livekit import api

token = api.AccessToken(
    api_key='api-key',
    api_secret='api-secret'
).with_identity('publisher').with_grants(
    api.VideoGrants(
        room='obs-stream',
        room_join=True,
        can_publish=True,
        can_subscribe=False
    )
).to_jwt()

print(token)
```

See [LiveKit Token Generation Guide](https://docs.livekit.io/guides/access-tokens/) for more languages.

---

## Configuring OBS WebRTC Link

### For Publishing (Sending Video from OBS)

1. Open OBS Studio
2. Go to **Settings** → **Output**
3. Select **WebRTC Output**
4. Configure:
   - **Mode**: SFU (WHIP)
   - **URL**: `https://<your-livekit-server>/whip` or `http://localhost:7880/whip` (local)
   - **Token**: Paste the **publisher token** generated above
   - **Room**: e.g., "obs-stream" (must match token room)

5. Click **Start Streaming**

### For Receiving (Receiving Video in OBS)

1. Open OBS Studio
2. Add a new **WebRTC Link Source**:
   - **Sources** panel → **+** → **WebRTC Link Source**
3. Configure:
   - **Mode**: SFU (WHEP)
   - **URL**: `https://<your-livekit-server>/whep` or `http://localhost:7880/whep` (local)
   - **Token**: Paste the **subscriber token** generated above
   - **Room**: e.g., "obs-stream" (must match token room)

4. The video should appear in your scene

### Example: OBS-to-OBS Relay

**Sender (OBS Instance 1):**
- Mode: SFU (WHIP)
- URL: `http://localhost:7880/whip`
- Token: Publisher token for room "obs-relay"

**Receiver (OBS Instance 2):**
- Mode: SFU (WHEP)
- URL: `http://localhost:7880/whep`
- Token: Subscriber token for room "obs-relay"

---

## Troubleshooting

### Connection Failed

**Symptoms:**
- OBS cannot connect to LiveKit
- Error: "Connection failed" or "Unauthorized"

**Solutions:**

1. **Check LiveKit is running:**
   ```bash
   curl http://localhost:7880/health
   ```
   Should return: `{"status":"ok"}`

2. **Verify API credentials:**
   - Ensure `LIVEKIT_API_KEY` and `LIVEKIT_API_SECRET` are set correctly in `.env`
   - Restart LiveKit after changing credentials:
     ```bash
     docker-compose restart
     ```

3. **Check token validity:**
   - Tokens expire after the specified time (`--valid-for`)
   - Generate a new token with longer validity

4. **Verify URL format:**
   - WHIP: `http://localhost:7880/whip` (not `/whip/`)
   - WHEP: `http://localhost:7880/whep` (not `/whep/`)

5. **Check firewall settings:**
   - Ensure port 7880 (TCP) is open
   - UDP ports 50000-60000 should be accessible for WebRTC media

### Token Errors

**Error: "Invalid token" or "Token expired"**

**Solutions:**

1. **Regenerate token:**
   ```bash
   livekit-cli create-token \
     --api-key <key> \
     --api-secret <secret> \
     --join --room <room> --identity <identity> \
     --valid-for 48h
   ```

2. **Verify token matches room:**
   - The `--room` parameter must match the room you're joining in OBS

3. **Check API key/secret:**
   - Ensure they match the LiveKit server configuration

### Video Not Appearing

**Symptoms:**
- Connection succeeds but no video appears
- Black screen in WebRTC Link Source

**Solutions:**

1. **Check room name:**
   - Publisher and subscriber must use the **same room name**

2. **Verify publisher is streaming:**
   - Ensure the sender OBS instance is actively streaming

3. **Check LiveKit logs:**
   ```bash
   docker-compose logs -f livekit
   ```
   Look for errors related to room joining or media negotiation

4. **Try different room:**
   - Sometimes rooms get into a bad state
   - Generate tokens for a new room name and test

### High Latency

**Symptoms:**
- Noticeable delay between sender and receiver
- Latency > 1 second

**Solutions:**

1. **Use LiveKit Cloud:**
   - Local Docker setup may have higher latency
   - LiveKit Cloud is optimized for low latency

2. **Check network conditions:**
   - Use `ping` to test latency to LiveKit server
   - Ensure stable internet connection

3. **Optimize OBS settings:**
   - Lower resolution (720p instead of 1080p)
   - Use hardware encoding (NVENC, QuickSync, AMF)

4. **Choose nearest region:**
   - If using LiveKit Cloud, select region closest to you

### Docker Issues

**Error: "Cannot start service livekit"**

**Solutions:**

1. **Check Docker is running:**
   ```bash
   docker ps
   ```

2. **Check port conflicts:**
   - Ensure port 7880 is not in use:
     ```bash
     # Windows
     netstat -ano | findstr :7880

     # Linux/macOS
     lsof -i :7880
     ```

3. **View detailed logs:**
   ```bash
   docker-compose logs livekit
   ```

4. **Rebuild containers:**
   ```bash
   docker-compose down -v
   docker-compose up -d --build
   ```

---

## Security Best Practices

### API Key and Secret Management

1. **Never commit credentials to Git:**
   - Add `.env` to `.gitignore`
   - Use environment variables in production

2. **Use strong credentials:**
   - Generate 32+ byte random strings
   - Use `openssl rand -base64 32` or similar

3. **Rotate credentials regularly:**
   - Change API keys/secrets every 90 days
   - Update all dependent services

4. **Limit API key permissions:**
   - Use separate keys for different environments (dev, staging, prod)
   - Restrict keys to specific rooms if possible

### Token Security

1. **Use short-lived tokens:**
   - Default: 24 hours
   - For production: 1-2 hours
   - Generate new tokens programmatically as needed

2. **Validate token permissions:**
   - Publishers should NOT have `canSubscribe`
   - Subscribers should NOT have `canPublish`

3. **Use HTTPS in production:**
   - Never use HTTP for public LiveKit servers
   - Use valid SSL certificates

### Network Security

1. **Firewall configuration:**
   - Only open necessary ports (7880 TCP, 50000-60000 UDP)
   - Use security groups/firewalls to restrict access

2. **Use VPN for local testing:**
   - Don't expose local LiveKit to the internet
   - Use VPN or SSH tunnels for remote testing

3. **Monitor access logs:**
   - Check LiveKit logs for unauthorized access attempts
   - Set up alerts for suspicious activity

### WHIP/WHEP Security

1. **Token in Authorization header:**
   - WHIP/WHEP use Bearer tokens in HTTP Authorization header
   - Tokens are not visible in URLs

2. **Use unique tokens per client:**
   - Generate separate tokens for each OBS instance
   - Revoke tokens when no longer needed

3. **Implement token refresh:**
   - For long-running streams, implement token refresh mechanism
   - Generate new token before current expires

---

## Additional Resources

- [LiveKit Documentation](https://docs.livekit.io/)
- [LiveKit GitHub](https://github.com/livekit/livekit)
- [WHIP Specification](https://datatracker.ietf.org/doc/html/draft-ietf-wish-whip)
- [WHEP Specification](https://datatracker.ietf.org/doc/html/draft-murillo-whep)
- [LiveKit Cloud](https://cloud.livekit.io/)
- [LiveKit Community Forum](https://github.com/livekit/livekit/discussions)

---

## Need Help?

If you encounter issues not covered in this guide:

1. Check the [LiveKit Troubleshooting Guide](https://docs.livekit.io/guides/troubleshooting/)
2. Search [LiveKit Discussions](https://github.com/livekit/livekit/discussions)
3. Open an issue on [OBS-WebRTC-Link Issues](https://github.com/m96-chan/OBS-WebRTC-Link/issues)
4. Join the [LiveKit Community Slack](https://livekit.io/community)
