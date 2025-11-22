# Testing Guide

This document provides comprehensive guidance on testing the OBS-WebRTC-Link project.

---

## Table of Contents

1. [Test Overview](#test-overview)
2. [Building Tests](#building-tests)
3. [Running Tests](#running-tests)
4. [Test Structure](#test-structure)
5. [Writing Tests](#writing-tests)
6. [Test Coverage](#test-coverage)
7. [Continuous Integration](#continuous-integration)
8. [Troubleshooting](#troubleshooting)

---

## Test Overview

The OBS-WebRTC-Link project uses [Google Test](https://github.com/google/googletest) framework for unit testing. Tests are organized into the following categories:

- **Core Tests**: Tests for WebRTC core components ([peer-connection.hpp](../src/core/peer-connection.hpp), [signaling-client.hpp](../src/core/signaling-client.hpp), [whip-client.hpp](../src/core/whip-client.hpp), [whep-client.hpp](../src/core/whep-client.hpp))
- **Output Tests**: Tests for OBS output functionality ([webrtc-output.hpp](../src/output/webrtc-output.hpp))
- **Source Tests**: Tests for OBS source functionality ([webrtc-source.hpp](../src/source/webrtc-source.hpp))
- **Integration Tests**: End-to-end tests with real SFU servers

### Test Files Location

```
tests/
├── unit/
│   ├── peer_connection_test.cpp
│   ├── signaling_client_test.cpp
│   ├── whip_client_test.cpp
│   ├── whep_client_test.cpp
│   ├── reconnection_manager_test.cpp
│   ├── webrtc_output_test.cpp
│   └── webrtc_source_test.cpp
└── CMakeLists.txt
```

---

## Building Tests

### Prerequisites

- CMake 3.20 or later
- C++17 compatible compiler (MSVC 2019+, GCC 9+, Clang 10+)
- Google Test (automatically fetched by CMake)

### Build Instructions

**Option 1: Build tests with OBS plugin**

```bash
mkdir build && cd build
cmake .. -DOBS_INCLUDE_SEARCH_PATH="path/to/obs-studio/include" \
         -DOBS_LIB_SEARCH_PATH="path/to/obs-studio/lib"
cmake --build . --config Debug
```

**Option 2: Build tests only (without OBS SDK)**

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTS_ONLY=ON
cmake --build . --config Debug
```

This option is useful for CI environments where OBS SDK is not available.

---

## Running Tests

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Specific Test Suite

```bash
# Run only PeerConnection tests
ctest -R PeerConnectionTest --output-on-failure

# Run only SignalingClient tests
ctest -R SignalingClientTest --output-on-failure

# Run only WHIP tests
ctest -R WHIPClientTest --output-on-failure

# Run only WHEP tests
ctest -R WHEPClientTest --output-on-failure
```

### Run Tests with Verbose Output

```bash
ctest --verbose --output-on-failure
```

### Run Tests in Debug Mode (Windows)

```bash
"C:\Program Files\CMake\bin\ctest.exe" --test-dir build -C Debug --output-on-failure
```

---

## Test Structure

### Test Fixture Pattern

All tests use Google Test's test fixture pattern for proper setup and teardown:

```cpp
class PeerConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize test resources
        config_ = PeerConnectionConfig{};
        config_.iceServers = {"stun:stun.l.google.com:19302"};
    }

    void TearDown() override {
        // Clean up test resources
    }

    PeerConnectionConfig config_;
};

TEST_F(PeerConnectionTest, CreateOfferSucceeds) {
    auto pc = std::make_unique<PeerConnection>(config_);
    EXPECT_NO_THROW(pc->createOffer());
}
```

### Test Naming Conventions

Tests follow this naming pattern:

```
TEST_F(ComponentTest, MethodName_Condition_ExpectedBehavior)
```

Examples:
- `TEST_F(WHIPClientTest, SendOffer_ValidSDP_ReturnsAnswer)`
- `TEST_F(SignalingClientTest, Connect_InvalidURL_ThrowsException)`
- `TEST_F(PeerConnectionTest, AddIceCandidate_ValidCandidate_Succeeds)`

---

## Writing Tests

### Basic Test Template

```cpp
/**
 * @brief Test description here
 */
TEST_F(ComponentTest, TestName) {
    // Arrange: Set up test conditions
    auto component = std::make_unique<Component>(config_);

    // Act: Perform the operation being tested
    component->doSomething();

    // Assert: Verify the results
    EXPECT_TRUE(component->isReady());
}
```

### Testing Success Cases

```cpp
TEST_F(WHIPClientTest, SendOffer) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    std::string receivedAnswer;

    EXPECT_NO_THROW({ receivedAnswer = client->sendOffer(testOffer); });
}
```

### Testing Error Cases

```cpp
TEST_F(WHIPClientTest, SendOfferWithInvalidSDP) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string invalidOffer = "";

    EXPECT_THROW({ client->sendOffer(invalidOffer); }, std::invalid_argument);
}
```

### Testing HTTP Errors

```cpp
TEST_F(WHIPClientTest, HandleUnauthorizedError) {
    config_.bearerToken = "invalid-token";
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on 401 error
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}
```

### Testing Network Errors

Use TEST-NET-1 (192.0.2.1) IP address range for guaranteed timeout scenarios:

```cpp
TEST_F(WHIPClientTest, HandleNetworkTimeout) {
    // Use an IP that will timeout (TEST-NET-1)
    config_.url = "http://192.0.2.1:12345/whip";
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on timeout
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}
```

### Testing Callbacks

```cpp
TEST_F(SignalingClientTest, OnConnectedCallbackFired) {
    bool connected = false;
    config_.onConnected = [&connected]() { connected = true; };

    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    // Give time for callback to fire
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(connected);
}
```

### Testing JSON Serialization/Deserialization

```cpp
TEST_F(SignalingClientTest, JsonSerializationOfferMessage) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string offerSdp = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    EXPECT_NO_THROW(client->sendOffer(offerSdp));
}

TEST_F(SignalingClientTest, JsonDeserializationMalformedOffer) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string malformedMessage = R"({"type": "offer"})";

    EXPECT_NO_THROW(client->handleMessage(malformedMessage));
    EXPECT_FALSE(lastError_.empty());
}
```

### Testing Concurrent Operations

```cpp
TEST_F(SignalingClientTest, ConcurrentMessageHandling) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string offerSdp = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    const std::string message1 = R"({"type": "offer", "sdp": ")" + offerSdp + R"("})";

    const std::string candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    const std::string message2 = R"({"type": "candidate", "candidate": ")" + candidate + R"(", "mid": "0"})";

    EXPECT_NO_THROW({
        client->handleMessage(message1);
        client->handleMessage(message2);
    });

    EXPECT_EQ(receivedOffer_, offerSdp);
    EXPECT_EQ(receivedCandidates_.size(), 1);
}
```

### Testing Move Semantics

```cpp
TEST_F(WHEPClientTest, MoveSemantics) {
    auto client1 = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client1->sendOffer(testOffer);

    EXPECT_TRUE(client1->isConnected());

    // Move construction
    auto client2 = std::move(client1);
    EXPECT_TRUE(client2->isConnected());

    // Move assignment
    auto client3 = std::make_unique<WHEPClient>(config_);
    client3 = std::move(client2);
    EXPECT_TRUE(client3->isConnected());
}
```

---

## Test Coverage

### Current Test Coverage

#### Core Components

**PeerConnection** ([tests/unit/peer_connection_test.cpp](../tests/unit/peer_connection_test.cpp)):
- ✅ Construction with valid config
- ✅ Offer/Answer generation
- ✅ Remote description setting
- ✅ ICE candidate handling
- ✅ Connection state transitions
- ✅ Error handling
- ⚠️ Some tests failing (see [Issue #55](https://github.com/m96-chan/OBS-WebRTC-Link/issues/55))

**SignalingClient** ([tests/unit/signaling_client_test.cpp](../tests/unit/signaling_client_test.cpp)):
- ✅ Connection management (connect/disconnect)
- ✅ Offer/Answer exchange
- ✅ ICE candidate exchange
- ✅ JSON serialization/deserialization
- ✅ Malformed message handling
- ✅ Network error simulation
- ✅ Empty SDP validation
- ✅ Concurrent message handling
- ✅ Special characters handling
- ⚠️ Some tests failing (see [Issue #55](https://github.com/m96-chan/OBS-WebRTC-Link/issues/55))

**WHIPClient** ([tests/unit/whip_client_test.cpp](../tests/unit/whip_client_test.cpp)):
- ✅ Construction with valid/invalid config
- ✅ Offer sending
- ✅ ICE candidate sending
- ✅ Bearer token authentication
- ✅ HTTP error handling (400, 401, 403, 404, 500, 503)
- ✅ Network errors (timeout, DNS failure, connection refused)
- ✅ SSL certificate errors
- ✅ Header verification
- ✅ Move semantics
- ✅ Disconnection

**WHEPClient** ([tests/unit/whep_client_test.cpp](../tests/unit/whep_client_test.cpp)):
- ✅ Construction with valid/invalid config
- ✅ Offer sending
- ✅ ICE candidate sending
- ✅ Bearer token authentication
- ✅ HTTP error handling (400, 401, 403, 404, 500, 503)
- ✅ Network errors (timeout, DNS failure, connection refused)
- ✅ SSL certificate errors
- ✅ Move semantics
- ✅ Disconnection

**ReconnectionManager** ([tests/unit/reconnection_manager_test.cpp](../tests/unit/reconnection_manager_test.cpp)):
- ✅ Exponential backoff calculation
- ✅ Max retry enforcement
- ✅ Cancellation
- ✅ Reset functionality

### Coverage Gaps

The following areas need additional test coverage (tracked in [Issue #55](https://github.com/m96-chan/OBS-WebRTC-Link/issues/55)):

1. **PeerConnection**: Fix failing tests for offer/answer generation and ICE candidate collection
2. **SignalingClient**: Fix message reception and JSON error handling tests
3. **Integration Tests**: End-to-end tests with real LiveKit server
4. **Output/Source**: OBS integration tests

---

## Continuous Integration

### GitHub Actions

Tests are automatically run on every pull request and push to main branch. See [.github/workflows/build.yml](../.github/workflows/build.yml) for CI configuration.

CI runs tests on:
- ✅ Windows (MSVC)
- ✅ Linux (GCC)
- ✅ macOS (Clang)

### CI Test Execution

```yaml
- name: Run Tests
  run: |
    cd build
    ctest --output-on-failure --verbose
```

### Test Failure Policy

- All tests must pass before merging to main
- PR checks include:
  - ✅ Code formatting (clang-format)
  - ✅ Build success
  - ✅ Unit tests passing

---

## Troubleshooting

### Common Issues

#### Tests Fail to Build

**Problem**: CMake cannot find libdatachannel

**Solution**: Initialize submodules:
```bash
git submodule update --init --recursive
```

#### Tests Timeout

**Problem**: Network tests timeout in CI

**Solution**: Increase timeout in CMakeLists.txt:
```cmake
set_tests_properties(WHIPClientTest PROPERTIES TIMEOUT 120)
```

#### HTTP Tests Fail

**Problem**: HTTP error tests fail due to actual network calls

**Solution**: These tests intentionally make network calls to non-existent endpoints to test error handling. Ensure network is available.

#### SSL Certificate Errors

**Problem**: SSL tests fail with certificate errors

**Solution**: Tests use `badssl.com` for SSL testing. Ensure DNS resolution works:
```bash
ping self-signed.badssl.com
```

### Debugging Tests

#### Enable Verbose Logging

```cpp
config_.logCallback = [](LogLevel level, const std::string& msg) {
    std::cout << "[" << static_cast<int>(level) << "] " << msg << std::endl;
};
```

#### Run Single Test

```bash
ctest -R SpecificTestName --verbose --output-on-failure
```

#### Debug with GDB (Linux)

```bash
gdb --args ./tests/unit/obs-webrtc-core-tests --gtest_filter=PeerConnectionTest.CreateOffer
```

#### Debug with Visual Studio (Windows)

1. Open the solution in Visual Studio
2. Set `obs-webrtc-core-tests` as startup project
3. Set breakpoints
4. Run with F5

### Known Test Failures

See [Issue #55](https://github.com/m96-chan/OBS-WebRTC-Link/issues/55) for current test failures:

**PeerConnection Tests** (18 failing):
- Offer/Answer generation tests
- ICE candidate collection tests
- State transition tests

**SignalingClient Tests** (5 failing):
- Message reception tests
- JSON deserialization error handling

These failures are due to core implementation issues, not test issues. They are tracked for fixing.

---

## Best Practices

### 1. Test Isolation

Each test should be independent and not rely on other tests:

```cpp
TEST_F(ComponentTest, Test1) {
    // Don't rely on Test2 running first
}

TEST_F(ComponentTest, Test2) {
    // Don't rely on Test1 state
}
```

### 2. Clear Test Names

Use descriptive names that explain what is being tested:

```cpp
// Good
TEST_F(WHIPClientTest, SendOffer_EmptySDP_ThrowsInvalidArgument)

// Bad
TEST_F(WHIPClientTest, Test1)
```

### 3. Single Assertion Per Test

Each test should verify one specific behavior:

```cpp
// Good - one concept
TEST_F(WHIPClientTest, SendOffer_ValidSDP_ReturnsAnswer) {
    EXPECT_NO_THROW(client->sendOffer(validSdp));
}

// Bad - multiple concepts
TEST_F(WHIPClientTest, SendOfferAndDisconnect) {
    EXPECT_NO_THROW(client->sendOffer(validSdp));
    EXPECT_NO_THROW(client->disconnect());
    EXPECT_FALSE(client->isConnected());
}
```

### 4. Use Mock Objects for External Dependencies

For testing components that depend on network or file I/O, use mock objects:

```cpp
class MockSignalingTransport : public SignalingTransport {
public:
    MOCK_METHOD(void, connect, (const std::string& url), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(void, sendMessage, (const std::string& message), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
};
```

### 5. Clean Up Resources

Always clean up in TearDown():

```cpp
void TearDown() override {
    if (client_) {
        client_->disconnect();
        client_.reset();
    }
}
```

---

## Testing Checklist

When adding new features, ensure:

- [ ] Unit tests cover all public methods
- [ ] Success cases are tested
- [ ] Error cases are tested
- [ ] Edge cases are tested
- [ ] Callbacks are tested
- [ ] Thread safety is considered
- [ ] Memory leaks are checked
- [ ] Tests are documented
- [ ] Tests pass on all platforms

---

**See Also**:
- [API Reference](API-REFERENCE.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [Project Structure](OBS-WebRTC-Link-Project-Structure.md)
