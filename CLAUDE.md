# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

OBS-WebRTC-Link is a WebRTC plugin for OBS Studio that enables universal WebRTC input and output. It supports two connection modes:
- **SFU Relay (WHIP/WHEP)**: Server-mediated connections through compliant SFUs (e.g., LiveKit)
- **Direct P2P**: Low-latency direct peer-to-peer connections

The codebase is designed with a **clean separation between WebRTC core logic and OBS plugin integration**, enabling independent testing and development.

## Build Commands

### Quick Build (Tests Only - No OBS SDK Required)

```bash
# Initialize submodules (first time only)
git submodule update --init --recursive

# Configure and build tests
cmake -B build -DBUILD_TESTS_ONLY=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run all tests
cd build
ctest --output-on-failure --verbose
```

### Full Plugin Build (Requires OBS SDK)

**Linux:**
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DOBS_INCLUDE_SEARCH_PATH="/usr/include" \
  -DOBS_LIB_SEARCH_PATH="/usr/lib"
cmake --build build -j$(nproc)
sudo cmake --install build
```

**Windows:**
```bash
cmake -B build -G "Visual Studio 17 2022" -A x64 ^
  -DOBS_INCLUDE_SEARCH_PATH="C:/Program Files/obs-studio/include" ^
  -DOBS_LIB_SEARCH_PATH="C:/Program Files/obs-studio/bin/64bit"
cmake --build build --config Release
cmake --install build --config Release
```

**macOS:**
```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DOBS_INCLUDE_SEARCH_PATH="/opt/homebrew/include" \
  -DOBS_LIB_SEARCH_PATH="/opt/homebrew/lib"
cmake --build build --config Release
sudo cmake --install build --config Release
```

### Running Specific Tests

```bash
# Run a single test
./build/tests/unit/peer_connection_test

# Run with verbose output
./build/tests/unit/peer_connection_test --gtest_verbose

# Run specific test case
./build/tests/unit/peer_connection_test --gtest_filter=PeerConnectionTest.ConstructionSucceeds
```

### Performance Benchmarks

```bash
cmake -B build -DBUILD_BENCHMARKS=ON -DBUILD_TESTS_ONLY=ON
cmake --build build --config Release
./build/tests/benchmarks/whip_connection_benchmark
```

## Architecture

### Layer Separation

The codebase follows a **3-layer architecture**:

1. **Core Layer** (`src/core/`)
   - **OBS-independent WebRTC logic**
   - All core classes are pure C++ with no OBS dependencies
   - Thread-safe and designed for reusability
   - Can be tested without OBS SDK

2. **Integration Layer** (`src/output/`, `src/source/`)
   - Bridges core WebRTC logic with OBS plugin API
   - Handles OBS-specific concerns (properties, data flow, lifecycle)
   - Two implementations:
     - `webrtc-output.cpp`: Sends OBS program output via WebRTC
     - `webrtc-source.cpp`: Receives WebRTC streams as OBS source

3. **UI Layer** (`src/ui/`)
   - Qt-based settings dialogs and UI components
   - Optional (plugin builds without Qt if unavailable)

### Core Components

**PeerConnection** (`peer-connection.hpp/cpp`)
- WebRTC peer connection wrapper using libdatachannel
- Handles offer/answer generation, ICE candidates, and connection states
- Thread-safe operations with callback-based event notification
- **Critical for multi-connection tests**: Use independent callback state to avoid race conditions

**SignalingClient** (`signaling-client.hpp/cpp`)
- Generic WebSocket-based signaling for exchanging SDP and ICE candidates
- Used by both WHIP/WHEP and P2P modes
- Supports JSON-based message protocol

**WHIP/WHEP Clients** (`whip-client.hpp/cpp`, `whep-client.hpp/cpp`)
- WHIP (WebRTC-HTTP Ingestion Protocol): Publishes streams to SFU servers
- WHEP (WebRTC-HTTP Egress Protocol): Subscribes to streams from SFU servers
- HTTP-based signaling with bearer token authentication

**P2PConnection** (`p2p-connection.hpp/cpp`)
- Direct peer-to-peer connection management
- Session ID-based pairing
- Supports STUN/TURN server configuration

**ReconnectionManager** (`reconnection-manager.hpp/cpp`)
- Automatic reconnection with exponential backoff
- Configurable retry limits and delays

**AudioOnlyConfig** (`audio-only-config.hpp/cpp`)
- Audio-only mode configuration with quality presets (high/medium/low)
- Opus codec settings and echo cancellation control

## Testing Strategy

### Test Organization

- **Unit Tests** (`tests/unit/`): Test individual core components in isolation
- **Integration Tests** (`tests/integration/`): Test component interactions (WHIP, WHEP, P2P)
- **Benchmarks** (`tests/benchmarks/`): Performance testing

### Critical Testing Pattern: Independent Callback State

**Problem**: Multiple `PeerConnection` instances sharing test fixture callback state causes race conditions and memory corruption.

**Solution**: Use the `CallbackState` helper struct for tests with multiple connections:

```cpp
TEST_F(PeerConnectionTest, MultiplePeerConnectionsCoexist) {
    // Create separate state for each connection
    CallbackState state1, state2;

    auto config1 = createTestConfigWithState(state1);
    auto config2 = createTestConfigWithState(state2);

    auto pc1 = std::make_unique<PeerConnection>(config1);
    auto pc2 = std::make_unique<PeerConnection>(config2);

    // Each connection has isolated callback storage
}
```

This pattern is **mandatory** for any test creating multiple `PeerConnection` instances.

## Dependency Management

All dependencies are included as Git submodules:
- `deps/libdatachannel`: WebRTC implementation
- `deps/nlohmann-json`: JSON parsing
- `deps/googletest`: Unit testing
- `deps/benchmark`: Performance benchmarks

**To update dependencies:**
```bash
git submodule update --remote
```

## CI/CD Pipeline

The GitHub Actions workflow (`.github/workflows/build.yml`) validates:
- ✅ Cross-platform builds (Linux, Windows, macOS)
- ✅ Full plugin compilation (Linux with OBS SDK)
- ✅ Test-only builds (Windows/macOS without OBS SDK)
- ✅ All unit tests across platforms

**Important**: The CI automatically builds and tests on every PR. Ensure all tests pass locally before pushing.

## Common Pitfalls

### Race Conditions in Tests
**Issue**: Tests with multiple `PeerConnection` instances crash with "double free" or "Abort trap: 6"
**Fix**: Always use `createTestConfigWithState()` with independent `CallbackState` instances

### Submodule Issues
**Issue**: Build fails with "submodule not found"
**Fix**: Run `git submodule update --init --recursive`

### OBS SDK Not Found
**Issue**: CMake error "Could not find OBS Studio"
**Fix**: Either:
- Set `-DOBS_INCLUDE_SEARCH_PATH` and `-DOBS_LIB_SEARCH_PATH` correctly
- Use `-DBUILD_TESTS_ONLY=ON` to build without OBS SDK

### Qt Not Found (Non-Critical)
**Issue**: CMake warning "Qt not found"
**Fix**: This is non-critical. Plugin builds without UI if Qt is unavailable.

## Development Workflow

1. **Make changes** in `src/core/` for WebRTC logic
2. **Write tests** in `tests/unit/` following TDD principles
3. **Run tests locally**:
   ```bash
   cmake --build build --config Release --target <test_name>
   ./build/tests/unit/<test_name>
   ```
4. **Verify all tests pass**:
   ```bash
   ctest --output-on-failure --verbose
   ```
5. **Commit with descriptive messages** following the project's commit conventions
6. **Push and verify CI** passes on GitHub Actions

## Key Design Principles

- **OBS Independence**: Core WebRTC logic must not depend on OBS
- **Thread Safety**: All core components support concurrent access
- **Callback-Based**: Async operations use callbacks for event notification
- **Testability**: Every core component has comprehensive unit tests
- **Clean Separation**: Clear boundaries between core, integration, and UI layers
