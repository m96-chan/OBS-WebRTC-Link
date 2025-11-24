/**
 * @file livekit_integration_test.cpp
 * @brief Integration tests for LiveKit SFU connections
 *
 * These tests require Docker to run a local LiveKit server.
 * Tests verify end-to-end WebRTC connections using WHIP/WHEP protocols.
 */

#include "core/whip-client.hpp"
#include "core/whep-client.hpp"
#include "helpers/livekit_docker_manager.hpp"
#include "helpers/test_helpers.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

using namespace obswebrtc::core;
using namespace obswebrtc::testing;

/**
 * @brief Integration test fixture for LiveKit tests
 *
 * Automatically starts/stops LiveKit container for each test.
 * Provides helper methods for common test operations.
 */
class LiveKitIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Skip test if Docker is not available
        try {
            livekit_ = std::make_unique<LiveKitDockerManager>();
            livekit_->start();

            // Wait for LiveKit to be ready
            bool ready = livekit_->waitForReady(30000);
            if (!ready) {
                GTEST_SKIP() << "LiveKit server failed to start within 30 seconds. Logs:\n"
                             << livekit_->getLogs();
            }
        } catch (const std::exception& e) {
            GTEST_SKIP() << "Docker not available or LiveKit start failed: " << e.what();
        }

        // Reset test state
        publisherConnected_ = false;
        subscriberConnected_ = false;
        publisherError_.clear();
        subscriberError_.clear();
    }

    void TearDown() override {
        // Stop LiveKit (managed by unique_ptr destructor)
    }

    std::unique_ptr<LiveKitDockerManager> livekit_;

    // Test state
    std::atomic<bool> publisherConnected_{false};
    std::atomic<bool> subscriberConnected_{false};
    std::string publisherError_;
    std::string subscriberError_;
};

// ========== Basic Connectivity Tests ==========

/**
 * @brief Test: LiveKit server starts successfully
 *
 * Acceptance Criteria:
 * - Docker container starts without errors
 * - Server becomes healthy within timeout
 * - HTTP API is accessible
 */
TEST_F(LiveKitIntegrationTest, ServerStartsSuccessfully) {
    ASSERT_TRUE(livekit_->isHealthy());

    std::string httpUrl = livekit_->getHttpUrl();
    EXPECT_FALSE(httpUrl.empty());
    EXPECT_NE(httpUrl.find("http://localhost:"), std::string::npos);
}

/**
 * @brief Test: WHIP endpoint URL is correct
 */
TEST_F(LiveKitIntegrationTest, WhipEndpointUrl) {
    std::string roomName = "test-room-" + generateRandomString();
    std::string whipUrl = livekit_->getWhipUrl(roomName);

    EXPECT_FALSE(whipUrl.empty());
    EXPECT_NE(whipUrl.find("/whip/"), std::string::npos);
    EXPECT_NE(whipUrl.find(roomName), std::string::npos);
}

/**
 * @brief Test: WHEP endpoint URL is correct
 */
TEST_F(LiveKitIntegrationTest, WhepEndpointUrl) {
    std::string roomName = "test-room-" + generateRandomString();
    std::string whepUrl = livekit_->getWhepUrl(roomName);

    EXPECT_FALSE(whepUrl.empty());
    EXPECT_NE(whepUrl.find("/whep/"), std::string::npos);
    EXPECT_NE(whepUrl.find(roomName), std::string::npos);
}

// ========== WHIP Publishing Tests ==========

/**
 * @brief Test: WHIP client can publish to LiveKit
 *
 * TDD Workflow:
 * 1. RED: Test fails because WHIP client doesn't connect
 * 2. GREEN: Implement connection logic
 * 3. REFACTOR: Clean up implementation
 *
 * Acceptance Criteria:
 * - WHIP client creates offer
 * - Server responds with answer
 * - Connection establishes successfully
 * - State changes to Connected
 */
TEST_F(LiveKitIntegrationTest, WhipClientPublishSucceeds) {
    std::string roomName = "test-room-" + generateRandomString();
    std::string whipUrl = livekit_->getWhipUrl(roomName);

    // Create WHIP client configuration
    WHIPClientConfig config;
    config.url = whipUrl;
    config.bearerToken = livekit_->generateToken(roomName, "publisher", true, false);

    config.onConnected = [this]() { publisherConnected_ = true; };

    config.onDisconnected = [this]() { publisherConnected_ = false; };

    config.onError = [this](const std::string& error) {
        publisherError_ = error;
        publisherConnected_ = false;
    };

    // Create and start WHIP client
    auto whipClient = std::make_unique<WHIPClient>(config);
    whipClient->start();

    // Wait for connection (timeout: 10 seconds)
    bool connected = waitForCondition([this]() { return publisherConnected_.load(); }, 10000);

    EXPECT_TRUE(connected) << "Publisher failed to connect. Error: " << publisherError_
                           << "\nLiveKit logs:\n"
                           << livekit_->getLogs();

    // Verify connection state
    EXPECT_EQ(whipClient->getState(), ConnectionState::Connected);

    // Clean up
    whipClient->stop();
}

/**
 * @brief Test: WHIP client handles invalid URL gracefully
 */
TEST_F(LiveKitIntegrationTest, WhipClientHandlesInvalidUrl) {
    WHIPClientConfig config;
    config.url = "http://localhost:9999/invalid/endpoint";
    config.bearerToken = "invalid-token";

    config.onError = [this](const std::string& error) { publisherError_ = error; };

    auto whipClient = std::make_unique<WHIPClient>(config);
    whipClient->start();

    // Wait for error (timeout: 5 seconds)
    bool errorOccurred =
        waitForCondition([this]() { return !publisherError_.empty(); }, 5000);

    EXPECT_TRUE(errorOccurred);
    EXPECT_FALSE(publisherError_.empty());
}

// ========== WHEP Subscription Tests ==========

/**
 * @brief Test: WHEP client can subscribe from LiveKit
 *
 * Acceptance Criteria:
 * - WHEP client creates offer
 * - Server responds with answer
 * - Connection establishes successfully
 * - Can receive media tracks
 */
TEST_F(LiveKitIntegrationTest, WhepClientSubscribeSucceeds) {
    std::string roomName = "test-room-" + generateRandomString();

    // First, publish a stream
    std::string whipUrl = livekit_->getWhipUrl(roomName);
    WHIPClientConfig publishConfig;
    publishConfig.url = whipUrl;
    publishConfig.bearerToken = livekit_->generateToken(roomName, "publisher", true, false);
    publishConfig.onConnected = [this]() { publisherConnected_ = true; };

    auto publisher = std::make_unique<WHIPClient>(publishConfig);
    publisher->start();

    bool publisherReady =
        waitForCondition([this]() { return publisherConnected_.load(); }, 10000);
    ASSERT_TRUE(publisherReady) << "Publisher failed to connect";

    // Now subscribe to the stream
    std::string whepUrl = livekit_->getWhepUrl(roomName);
    WHEPClientConfig subscribeConfig;
    subscribeConfig.url = whepUrl;
    subscribeConfig.bearerToken = livekit_->generateToken(roomName, "subscriber", false, true);
    subscribeConfig.onConnected = [this]() { subscriberConnected_ = true; };
    subscribeConfig.onError = [this](const std::string& error) { subscriberError_ = error; };

    auto subscriber = std::make_unique<WHEPClient>(subscribeConfig);
    subscriber->start();

    // Wait for subscriber connection
    bool subscriberReady =
        waitForCondition([this]() { return subscriberConnected_.load(); }, 10000);

    EXPECT_TRUE(subscriberReady) << "Subscriber failed to connect. Error: " << subscriberError_
                                 << "\nLiveKit logs:\n"
                                 << livekit_->getLogs();

    // Clean up
    subscriber->stop();
    publisher->stop();
}

// ========== Reconnection Tests ==========

/**
 * @brief Test: WHIP client reconnects after network interruption
 *
 * Acceptance Criteria:
 * - Client detects connection loss
 * - Automatic reconnection attempt
 * - Connection re-establishes successfully
 */
TEST_F(LiveKitIntegrationTest, DISABLED_WhipClientReconnectsAfterDisconnection) {
    // TODO: Implement after reconnection manager integration
    GTEST_SKIP() << "Reconnection test pending reconnection manager implementation";
}

// ========== Error Handling Tests ==========

/**
 * @brief Test: WHIP client handles authentication failure
 */
TEST_F(LiveKitIntegrationTest, WhipClientHandlesAuthenticationFailure) {
    std::string roomName = "test-room-" + generateRandomString();
    std::string whipUrl = livekit_->getWhipUrl(roomName);

    WHIPClientConfig config;
    config.url = whipUrl;
    config.bearerToken = "invalid-token-12345";
    config.onError = [this](const std::string& error) { publisherError_ = error; };

    auto whipClient = std::make_unique<WHIPClient>(config);
    whipClient->start();

    // Wait for authentication error
    bool errorOccurred =
        waitForCondition([this]() { return !publisherError_.empty(); }, 5000);

    EXPECT_TRUE(errorOccurred);
    // Error message should indicate authentication failure
    EXPECT_NE(publisherError_.find("auth"), std::string::npos);
}

// ========== Resource Cleanup Tests ==========

/**
 * @brief Test: Multiple sequential connections don't leak resources
 *
 * Acceptance Criteria:
 * - Can create and destroy multiple connections
 * - Memory usage remains stable
 * - No zombie connections remain
 */
TEST_F(LiveKitIntegrationTest, MultipleConnectionsNoResourceLeak) {
    MemoryUsage memBefore = getCurrentMemoryUsage();

    // Create and destroy 10 connections
    for (int i = 0; i < 10; ++i) {
        std::string roomName = "test-room-" + std::to_string(i);
        std::string whipUrl = livekit_->getWhipUrl(roomName);

        WHIPClientConfig config;
        config.url = whipUrl;
        config.bearerToken = livekit_->generateToken(roomName, "publisher-" + std::to_string(i),
                                                      true, false);
        config.onConnected = [this]() { publisherConnected_ = true; };

        auto whipClient = std::make_unique<WHIPClient>(config);
        whipClient->start();

        waitForCondition([this]() { return publisherConnected_.load(); }, 5000);

        whipClient->stop();
        publisherConnected_ = false;

        // Small delay between connections
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    MemoryUsage memAfter = getCurrentMemoryUsage();

    // Check for memory leaks (allow 10MB threshold for test overhead)
    bool noLeak = checkMemoryLeak(memBefore, memAfter, 10 * 1024 * 1024);
    EXPECT_TRUE(noLeak) << "Potential memory leak detected. RSS increased by "
                        << (memAfter.rss - memBefore.rss) / 1024 << " KB";
}

// ========== Concurrent Connections Tests ==========

/**
 * @brief Test: Multiple concurrent publishers in same room
 *
 * Acceptance Criteria:
 * - Multiple publishers can connect to same room
 * - Each publisher maintains independent connection
 * - No interference between connections
 */
TEST_F(LiveKitIntegrationTest, MultipleConcurrentPublishers) {
    std::string roomName = "test-room-concurrent-" + generateRandomString();

    std::vector<std::unique_ptr<WHIPClient>> publishers;
    std::vector<std::atomic<bool>> connectionStates;

    const int numPublishers = 3;
    connectionStates.resize(numPublishers);

    // Create multiple publishers
    for (int i = 0; i < numPublishers; ++i) {
        std::string whipUrl = livekit_->getWhipUrl(roomName);

        WHIPClientConfig config;
        config.url = whipUrl;
        config.bearerToken = livekit_->generateToken(roomName, "publisher-" + std::to_string(i),
                                                      true, false);
        config.onConnected = [&connectionStates, i]() { connectionStates[i] = true; };

        auto publisher = std::make_unique<WHIPClient>(config);
        publisher->start();
        publishers.push_back(std::move(publisher));
    }

    // Wait for all publishers to connect
    bool allConnected = waitForCondition(
        [&connectionStates, numPublishers]() {
            int connected = 0;
            for (int i = 0; i < numPublishers; ++i) {
                if (connectionStates[i].load())
                    connected++;
            }
            return connected == numPublishers;
        },
        15000);

    EXPECT_TRUE(allConnected) << "Not all publishers connected successfully";

    // Verify all are connected
    for (int i = 0; i < numPublishers; ++i) {
        EXPECT_TRUE(connectionStates[i].load()) << "Publisher " << i << " failed to connect";
        EXPECT_EQ(publishers[i]->getState(), ConnectionState::Connected);
    }

    // Clean up
    for (auto& publisher : publishers) {
        publisher->stop();
    }
}
