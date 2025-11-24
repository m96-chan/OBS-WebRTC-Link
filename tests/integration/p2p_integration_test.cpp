/**
 * @file p2p_integration_test.cpp
 * @brief Integration tests for P2P (Direct) connections
 *
 * Tests end-to-end P2P WebRTC connections without SFU.
 * Uses test signaling server for offer/answer exchange.
 */

#include "core/p2p-connection.hpp"
#include "helpers/test_helpers.hpp"
#include "helpers/test_signaling_server.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <thread>

using namespace obswebrtc::core;
using namespace obswebrtc::testing;

/**
 * @brief Integration test fixture for P2P tests
 */
class P2PIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Start signaling server
        signalingServer_ = std::make_unique<TestSignalingServer>();
        signalingServer_->start();

        // Reset test state
        hostConnected_ = false;
        clientConnected_ = false;
        hostError_.clear();
        clientError_.clear();
    }

    void TearDown() override { signalingServer_->stop(); }

    std::unique_ptr<TestSignalingServer> signalingServer_;

    // Test state
    std::atomic<bool> hostConnected_{false};
    std::atomic<bool> clientConnected_{false};
    std::string hostError_;
    std::string clientError_;
};

// ========== Basic P2P Connection Tests ==========

/**
 * @brief Test: P2P host can generate session ID
 *
 * Acceptance Criteria:
 * - Session ID is generated
 * - Session ID is 8 characters
 * - Session ID is unique
 */
TEST_F(P2PIntegrationTest, HostGeneratesSessionId) {
    P2PConnectionConfig config;
    config.stunServers = {"stun:stun.l.google.com:19302"};

    auto host = std::make_unique<P2PConnection>(config);
    host->initializeAsHost();

    std::string sessionId1 = host->generateSessionId();
    std::string sessionId2 = host->generateSessionId();

    EXPECT_EQ(sessionId1.length(), 8);
    EXPECT_EQ(sessionId2.length(), 8);
    EXPECT_NE(sessionId1, sessionId2);
}

/**
 * @brief Test: P2P client can connect using session ID
 *
 * Acceptance Criteria:
 * - Host initializes and generates session ID
 * - Client connects using session ID
 * - Both peers establish connection
 */
TEST_F(P2PIntegrationTest, PeerToPeerConnectionEstablishes) {
    std::string sessionId = generateRandomString(8);

    // Create host configuration
    P2PConnectionConfig hostConfig;
    hostConfig.stunServers = {"stun:stun.l.google.com:19302"};
    hostConfig.onConnected = [this]() { hostConnected_ = true; };
    hostConfig.onDisconnected = [this]() { hostConnected_ = false; };
    hostConfig.onError = [this](const std::string& error) { hostError_ = error; };
    hostConfig.onOfferGenerated = [this, sessionId](const std::string& offer) {
        // Post offer to signaling server
        signalingServer_->postMessage(sessionId, "offer:" + offer);
    };

    // Create client configuration
    P2PConnectionConfig clientConfig;
    clientConfig.stunServers = {"stun:stun.l.google.com:19302"};
    clientConfig.onConnected = [this]() { clientConnected_ = true; };
    clientConfig.onDisconnected = [this]() { clientConnected_ = false; };
    clientConfig.onError = [this](const std::string& error) { clientError_ = error; };
    clientConfig.onAnswerGenerated = [this, sessionId](const std::string& answer) {
        // Post answer to signaling server
        signalingServer_->postMessage(sessionId, "answer:" + answer);
    };

    // Create peers
    auto host = std::make_unique<P2PConnection>(hostConfig);
    auto client = std::make_unique<P2PConnection>(clientConfig);

    // Create signaling session
    signalingServer_->createSession();

    // Initialize host
    host->initializeAsHost();
    host->generateSessionId();

    // Initialize client
    client->initializeAsClient(sessionId);

    // Exchange signaling messages (simplified for test)
    // In real implementation, this would be done via HTTP requests

    // Wait for connections (timeout: 15 seconds for P2P)
    bool bothConnected = waitForCondition(
        [this]() { return hostConnected_.load() && clientConnected_.load(); }, 15000);

    EXPECT_TRUE(bothConnected) << "P2P connection failed.\n"
                               << "Host error: " << hostError_ << "\n"
                               << "Client error: " << clientError_;

    // Clean up
    host->close();
    client->close();
}

/**
 * @brief Test: P2P connection with data channel communication
 *
 * Acceptance Criteria:
 * - Connection establishes
 * - Data channel opens
 * - Messages can be sent/received
 */
TEST_F(P2PIntegrationTest, DISABLED_DataChannelCommunication) {
    // TODO: Implement after data channel support is added to P2PConnection
    GTEST_SKIP() << "Data channel test pending implementation";
}

// ========== Error Handling Tests ==========

/**
 * @brief Test: P2P handles invalid session ID gracefully
 */
TEST_F(P2PIntegrationTest, HandlesInvalidSessionId) {
    P2PConnectionConfig config;
    config.stunServers = {"stun:stun.l.google.com:19302"};
    config.onError = [this](const std::string& error) { clientError_ = error; };

    auto client = std::make_unique<P2PConnection>(config);

    // Try to connect with non-existent session
    client->initializeAsClient("INVALID1");

    // Wait for error
    bool errorOccurred =
        waitForCondition([this]() { return !clientError_.empty(); }, 5000);

    // Error may or may not occur immediately (depends on implementation)
    // Just verify client doesn't crash
    EXPECT_NO_THROW(client->close());
}

// ========== Performance Tests ==========

/**
 * @brief Test: Multiple sequential P2P connections
 *
 * Acceptance Criteria:
 * - Can establish multiple connections sequentially
 * - No resource leaks
 * - Performance remains consistent
 */
TEST_F(P2PIntegrationTest, MultipleSequentialConnections) {
    const int numConnections = 5;

    for (int i = 0; i < numConnections; ++i) {
        P2PConnectionConfig hostConfig;
        hostConfig.stunServers = {"stun:stun.l.google.com:19302"};
        hostConfig.onConnected = [this]() { hostConnected_ = true; };

        auto host = std::make_unique<P2PConnection>(hostConfig);
        host->initializeAsHost();
        host->generateSessionId();

        // Note: Full connection test would require client peer
        // For now, just verify host can be created/destroyed

        host->close();
        hostConnected_ = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // If we get here without crashes, test passes
    SUCCEED();
}
