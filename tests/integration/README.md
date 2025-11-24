# Integration Tests

This directory contains integration tests for the OBS WebRTC Link plugin. These tests verify end-to-end functionality using real WebRTC connections and services.

## Overview

Integration tests verify:
- **LiveKit Integration**: Real SFU connections using WHIP/WHEP protocols
- **P2P Connections**: Direct peer-to-peer WebRTC connections
- **End-to-End Flows**: Complete publish and subscribe workflows
- **Performance**: Connection times, memory usage, resource management
- **Stability**: Long-duration connections, repeated connect/disconnect cycles
- **Memory Leaks**: Resource cleanup validation using AddressSanitizer

## Prerequisites

### Required Software

1. **Docker**: Required for running LiveKit server
   - Linux/macOS: Install Docker Engine
   - Windows: Install Docker Desktop
   - Verify: `docker --version` and `docker info`

2. **CMake**: Build system (3.20 or later)

3. **C++ Compiler**:
   - Linux: GCC 9+ or Clang 10+
   - macOS: Xcode Command Line Tools
   - Windows: Visual Studio 2019+ or MSVC

### Optional Tools

- **AddressSanitizer**: For memory leak detection (available on GCC/Clang)
- **Valgrind**: Alternative memory leak detection (Linux only)

## Building

```bash
# Configure with integration tests enabled (default)
cmake -B build -DBUILD_TESTING=ON

# Build integration tests
cmake --build build --target livekit_integration_test
cmake --build build --target p2p_integration_test
cmake --build build --target e2e_flow_test
cmake --build build --target performance_test
cmake --build build --target stability_test
cmake --build build --target memory_leak_test

# Or build all targets
cmake --build build
```

## Running Tests

### All Integration Tests

```bash
# Linux/macOS
./tests/integration/scripts/run_integration_tests.sh

# Windows PowerShell
.\tests\integration\scripts\run_integration_tests.ps1
```

### Using CTest

```bash
# Run all integration tests
cd build
ctest -R Integration -V

# Run specific test suite
ctest -R LiveKitIntegration -V
ctest -R P2PIntegration -V
ctest -R E2EFlow -V
ctest -R Performance -V
ctest -R Stability -V
ctest -R MemoryLeak -V

# Run with specific configuration (Windows)
ctest -C Debug -R Integration -V
ctest -C Release -R Integration -V
```

### Manual Execution

```bash
# Linux/macOS
./build/tests/integration/livekit_integration_test

# Windows (Debug)
.\build\tests\integration\Debug\livekit_integration_test.exe

# Windows (Release)
.\build\tests\integration\Release\livekit_integration_test.exe
```

## Test Suites

### LiveKit Integration Tests

Tests WHIP/WHEP connections with real LiveKit SFU server.

**Tests:**
- Server startup and health checks
- WHIP publisher connection
- WHEP subscriber connection
- Authentication handling
- Multiple concurrent publishers
- Resource leak detection

**Requirements:**
- Docker must be running
- Ports 7880, 7881, 50000-50100 must be available

### P2P Integration Tests

Tests direct peer-to-peer WebRTC connections.

**Tests:**
- Session ID generation
- Host/client connection establishment
- Signaling message exchange
- Error handling
- Multiple sequential connections

**Requirements:**
- STUN server access (uses public Google STUN server)

### End-to-End Flow Tests

Tests complete publish/subscribe workflows.

**Tests:**
- Single publisher with single subscriber
- Multiple publishers and subscribers
- Full WebRTC flow validation

**Requirements:**
- Docker must be running
- LiveKit server

### Performance Tests

Measures connection performance and resource usage.

**Tests:**
- Connection establishment time
- Memory usage stability
- Concurrent connection scalability

**Acceptance Criteria:**
- Connection < 5 seconds
- Memory increase < 20MB for 20 connections
- 5 concurrent connections < 15 seconds

### Stability Tests

Validates long-duration operation and reliability.

**Tests:**
- 30-second continuous connection
- 10 connect/disconnect cycles
- State consistency verification

### Memory Leak Tests

Detects resource leaks using instrumentation.

**Tests:**
- PeerConnection creation/deletion (100 iterations)
- WHIP client connect/disconnect (50 iterations)

**Tools:**
- AddressSanitizer (enabled in Debug/RelWithDebInfo builds)
- Manual memory tracking

## Troubleshooting

### Docker Issues

**Error: "Docker is not available"**
```bash
# Linux: Start Docker service
sudo systemctl start docker

# macOS/Windows: Start Docker Desktop application
```

**Error: "Port already in use"**
```bash
# Find process using port 7880
netstat -ano | findstr 7880  # Windows
lsof -i :7880  # Linux/macOS

# Stop existing LiveKit container
docker stop obs-webrtc-test-livekit
docker rm obs-webrtc-test-livekit
```

### Build Issues

**Error: "LibDataChannel not found"**
```bash
# Initialize submodules
git submodule update --init --recursive
```

**Error: "GoogleTest not found"**
```bash
# Initialize submodules
git submodule update --init --recursive
```

### Test Failures

**LiveKit tests skip**
- Ensure Docker is running
- Check Docker logs: `docker logs obs-webrtc-test-livekit`
- Verify ports are available

**Timeout errors**
- Increase timeout in test configuration
- Check network connectivity
- Verify STUN server accessibility

**Memory leak false positives**
- Run in Debug mode with AddressSanitizer
- Some leaks may be in third-party libraries
- Check threshold settings in tests

## CI/CD Integration

Integration tests can be run in CI environments with Docker support:

```yaml
# GitHub Actions example
- name: Run Integration Tests
  run: |
    docker info  # Verify Docker is available
    cmake -B build -DBUILD_TESTING=ON
    cmake --build build
    cd build && ctest -R Integration --output-on-failure
```

## Contributing

When adding new integration tests:

1. Follow the TDD (Test-Driven Development) approach
2. Write failing tests first (RED)
3. Implement minimum code to pass (GREEN)
4. Refactor and improve (REFACTOR)
5. Document test purpose and acceptance criteria
6. Ensure tests are isolated and repeatable
7. Clean up resources in teardown methods

## Architecture

```
tests/integration/
├── helpers/               # Test utilities
│   ├── livekit_docker_manager.hpp/cpp  # LiveKit Docker orchestration
│   ├── test_signaling_server.hpp/cpp  # P2P signaling server
│   └── test_helpers.hpp/cpp            # Common utilities
├── docker/                # Docker configurations
│   ├── docker-compose.yml
│   └── livekit.yaml
├── scripts/               # Test runner scripts
│   ├── run_integration_tests.sh
│   └── run_integration_tests.ps1
├── livekit_integration_test.cpp
├── p2p_integration_test.cpp
├── e2e_flow_test.cpp
├── performance_test.cpp
├── stability_test.cpp
├── memory_leak_test.cpp
└── CMakeLists.txt
```

## References

- [LiveKit Documentation](https://docs.livekit.io/)
- [WHIP RFC Draft](https://datatracker.ietf.org/doc/draft-ietf-wish-whip/)
- [WHEP RFC Draft](https://datatracker.ietf.org/doc/draft-murillo-whep/)
- [WebRTC Specification](https://www.w3.org/TR/webrtc/)
- [GoogleTest Documentation](https://google.github.io/googletest/)
