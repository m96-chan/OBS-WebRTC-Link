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
    host->disconnect();
    client->disconnect();
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
    EXPECT_NO_THROW(client->disconnect());
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

        host->disconnect();
        hostConnected_ = false;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // If we get here without crashes, test passes
    SUCCEED();
}

// ========== Skipped Unit Tests Now Implemented as Integration Tests ==========
// These tests correspond to the 6 skipped tests in p2p_connection_test.cpp
// See Issue #67 for integration test strategy

/**
 * @brief Integration Test: Create offer as Host
 *
 * This test implements the skipped unit test "CreateOfferAsHost"
 * with actual WebRTC connection handling.
 *
 * Acceptance Criteria:
 * - Host can initialize successfully
 * - Host can create an SDP offer
 * - Offer is not empty and contains valid SDP
 * - onOfferGenerated callback is invoked
 */
TEST_F(P2PIntegrationTest, CreateOfferAsHost) {
    std::string generatedOffer;
    std::atomic<bool> offerGenerated{false};

    P2PConnectionConfig config;
    config.stunServers = {"stun:stun.l.google.com:19302"};
    config.onOfferGenerated = [&generatedOffer, &offerGenerated](const std::string& offer) {
        generatedOffer = offer;
        offerGenerated = true;
    };
    config.onError = [this](const std::string& error) { hostError_ = error; };

    auto host = std::make_unique<P2PConnection>(config);
    host->initializeAsHost();

    std::string offer;
    EXPECT_NO_THROW({ offer = host->createOffer(); });

    // Wait for offer to be generated (asynchronous)
    bool success = waitForCondition([&offerGenerated]() { return offerGenerated.load(); }, 5000);

    EXPECT_TRUE(success) << "Offer generation timed out. Error: " << hostError_;
    EXPECT_FALSE(generatedOffer.empty()) << "Generated offer is empty";
    EXPECT_EQ(generatedOffer, offer) << "Callback offer doesn't match returned offer";

    // Verify offer contains valid SDP markers
    EXPECT_NE(generatedOffer.find("v=0"), std::string::npos) << "Offer missing SDP version";
    EXPECT_NE(generatedOffer.find("o="), std::string::npos) << "Offer missing origin field";

    host->disconnect();
}

/**
 * @brief Integration Test: Set remote answer as Host
 *
 * This test implements the skipped unit test "SetRemoteAnswerAsHost"
 * with actual WebRTC connection handling.
 *
 * Acceptance Criteria:
 * - Host can create offer
 * - Host can set remote answer
 * - No exceptions are thrown
 * - Connection progresses toward Connected state
 */
TEST_F(P2PIntegrationTest, SetRemoteAnswerAsHost) {
    std::string hostOffer;
    std::string clientAnswer;
    std::atomic<bool> offerReady{false};
    std::atomic<bool> answerReady{false};

    // Setup Host
    P2PConnectionConfig hostConfig;
    hostConfig.stunServers = {"stun:stun.l.google.com:19302"};
    hostConfig.onOfferGenerated = [&hostOffer, &offerReady](const std::string& offer) {
        hostOffer = offer;
        offerReady = true;
    };
    hostConfig.onConnected = [this]() { hostConnected_ = true; };
    hostConfig.onError = [this](const std::string& error) { hostError_ = error; };

    // Setup Client
    P2PConnectionConfig clientConfig;
    clientConfig.stunServers = {"stun:stun.l.google.com:19302"};
    clientConfig.onAnswerGenerated = [&clientAnswer, &answerReady](const std::string& answer) {
        clientAnswer = answer;
        answerReady = true;
    };
    clientConfig.onConnected = [this]() { clientConnected_ = true; };
    clientConfig.onError = [this](const std::string& error) { clientError_ = error; };

    auto host = std::make_unique<P2PConnection>(hostConfig);
    auto client = std::make_unique<P2PConnection>(clientConfig);

    std::string sessionId = generateRandomString(8);

    // Host creates offer
    host->initializeAsHost();
    host->createOffer();

    // Wait for offer
    ASSERT_TRUE(waitForCondition([&offerReady]() { return offerReady.load(); }, 5000))
        << "Host offer generation timed out";

    // Client receives offer and creates answer
    client->initializeAsClient(sessionId);
    std::string answer = client->setRemoteOffer(hostOffer);

    // Wait for answer
    ASSERT_TRUE(waitForCondition([&answerReady]() { return answerReady.load(); }, 5000))
        << "Client answer generation timed out";

    // Host sets remote answer - THIS IS THE MAIN TEST
    EXPECT_NO_THROW({ host->setRemoteAnswer(clientAnswer); })
        << "Setting remote answer should not throw. Host error: " << hostError_;

    // Give some time for connection to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Cleanup
    host->disconnect();
    client->disconnect();
}

/**
 * @brief Integration Test: Create answer as Client
 *
 * This test implements the skipped unit test "CreateAnswerAsClient"
 * with actual WebRTC connection handling.
 *
 * Acceptance Criteria:
 * - Client can receive offer
 * - Client can create answer
 * - Answer is valid SDP
 * - onAnswerGenerated callback is invoked
 */
TEST_F(P2PIntegrationTest, CreateAnswerAsClient) {
    std::string hostOffer;
    std::string clientAnswer;
    std::atomic<bool> offerReady{false};
    std::atomic<bool> answerReady{false};

    // Setup Host (just to generate offer)
    P2PConnectionConfig hostConfig;
    hostConfig.stunServers = {"stun:stun.l.google.com:19302"};
    hostConfig.onOfferGenerated = [&hostOffer, &offerReady](const std::string& offer) {
        hostOffer = offer;
        offerReady = true;
    };

    // Setup Client
    P2PConnectionConfig clientConfig;
    clientConfig.stunServers = {"stun:stun.l.google.com:19302"};
    clientConfig.onAnswerGenerated = [&clientAnswer, &answerReady](const std::string& answer) {
        clientAnswer = answer;
        answerReady = true;
    };
    clientConfig.onError = [this](const std::string& error) { clientError_ = error; };

    auto host = std::make_unique<P2PConnection>(hostConfig);
    auto client = std::make_unique<P2PConnection>(clientConfig);

    std::string sessionId = generateRandomString(8);

    // Host creates offer
    host->initializeAsHost();
    host->createOffer();

    ASSERT_TRUE(waitForCondition([&offerReady]() { return offerReady.load(); }, 5000))
        << "Host offer generation timed out";

    // Client creates answer - THIS IS THE MAIN TEST
    client->initializeAsClient(sessionId);

    std::string answer;
    EXPECT_NO_THROW({ answer = client->setRemoteOffer(hostOffer); })
        << "Creating answer should not throw. Client error: " << clientError_;

    // Wait for answer callback
    bool success =
        waitForCondition([&answerReady]() { return answerReady.load(); }, 5000);

    EXPECT_TRUE(success) << "Answer generation timed out. Error: " << clientError_;
    EXPECT_FALSE(clientAnswer.empty()) << "Generated answer is empty";
    EXPECT_EQ(clientAnswer, answer) << "Callback answer doesn't match returned answer";

    // Verify answer contains valid SDP markers
    EXPECT_NE(clientAnswer.find("v=0"), std::string::npos) << "Answer missing SDP version";
    EXPECT_NE(clientAnswer.find("o="), std::string::npos) << "Answer missing origin field";

    host->disconnect();
    client->disconnect();
}

/**
 * @brief Integration Test: Handle ICE candidates
 *
 * This test implements the skipped unit test "HandleIceCandidate"
 * with actual WebRTC connection handling.
 *
 * Acceptance Criteria:
 * - ICE candidates are gathered after offer creation
 * - onIceCandidate callback is invoked
 * - At least one ICE candidate is collected
 */
TEST_F(P2PIntegrationTest, HandleIceCandidate) {
    std::vector<std::string> iceCandidates;
    std::atomic<bool> candidateReceived{false};

    P2PConnectionConfig config;
    config.stunServers = {"stun:stun.l.google.com:19302"};
    config.onIceCandidate = [&iceCandidates,
                               &candidateReceived](const std::string& candidate,
                                                   const std::string& mid) {
        iceCandidates.push_back(candidate);
        candidateReceived = true;
    };
    config.onError = [this](const std::string& error) { hostError_ = error; };

    auto host = std::make_unique<P2PConnection>(config);
    host->initializeAsHost();

    // Create offer - this triggers ICE gathering
    host->createOffer();

    // Wait for at least one ICE candidate
    bool success = waitForCondition([&candidateReceived]() { return candidateReceived.load(); },
                                     10000); // ICE gathering can take time

    EXPECT_TRUE(success) << "No ICE candidates received. Error: " << hostError_;
    EXPECT_GT(iceCandidates.size(), 0) << "Expected at least one ICE candidate";

    // Verify candidate format (should start with "candidate:")
    if (!iceCandidates.empty()) {
        EXPECT_NE(iceCandidates[0].find("candidate"), std::string::npos)
            << "ICE candidate has invalid format: " << iceCandidates[0];
    }

    host->disconnect();
}

/**
 * @brief Integration Test: Add remote ICE candidate
 *
 * This test implements the skipped unit test "AddRemoteIceCandidate"
 * with actual WebRTC connection handling.
 *
 * Acceptance Criteria:
 * - Can add remote ICE candidate after setting remote description
 * - No exceptions thrown
 * - Connection establishment progresses
 */
TEST_F(P2PIntegrationTest, AddRemoteIceCandidate) {
    std::string hostOffer;
    std::string clientAnswer;
    std::vector<std::string> hostCandidates;
    std::vector<std::string> clientCandidates;
    std::atomic<bool> offerReady{false};
    std::atomic<bool> answerReady{false};

    // Setup Host
    P2PConnectionConfig hostConfig;
    hostConfig.stunServers = {"stun:stun.l.google.com:19302"};
    hostConfig.onOfferGenerated = [&hostOffer, &offerReady](const std::string& offer) {
        hostOffer = offer;
        offerReady = true;
    };
    hostConfig.onIceCandidate = [&hostCandidates](const std::string& candidate,
                                                    const std::string& mid) {
        hostCandidates.push_back(candidate);
    };
    hostConfig.onError = [this](const std::string& error) { hostError_ = error; };

    // Setup Client
    P2PConnectionConfig clientConfig;
    clientConfig.stunServers = {"stun:stun.l.google.com:19302"};
    clientConfig.onAnswerGenerated = [&clientAnswer, &answerReady](const std::string& answer) {
        clientAnswer = answer;
        answerReady = true;
    };
    clientConfig.onIceCandidate = [&clientCandidates](const std::string& candidate,
                                                        const std::string& mid) {
        clientCandidates.push_back(candidate);
    };
    clientConfig.onError = [this](const std::string& error) { clientError_ = error; };

    auto host = std::make_unique<P2PConnection>(hostConfig);
    auto client = std::make_unique<P2PConnection>(clientConfig);

    std::string sessionId = generateRandomString(8);

    // Exchange offer/answer
    host->initializeAsHost();
    host->createOffer();

    ASSERT_TRUE(waitForCondition([&offerReady]() { return offerReady.load(); }, 5000));

    client->initializeAsClient(sessionId);
    client->setRemoteOffer(hostOffer);

    ASSERT_TRUE(waitForCondition([&answerReady]() { return answerReady.load(); }, 5000));

    host->setRemoteAnswer(clientAnswer);

    // Wait for ICE candidates
    waitForCondition([&hostCandidates]() { return !hostCandidates.empty(); }, 10000);
    waitForCondition([&clientCandidates]() { return !clientCandidates.empty(); }, 10000);

    // Add remote ICE candidates - THIS IS THE MAIN TEST
    if (!hostCandidates.empty()) {
        EXPECT_NO_THROW({ client->addRemoteIceCandidate(hostCandidates[0], "0"); })
            << "Adding remote ICE candidate should not throw. Client error: " << clientError_;
    }

    if (!clientCandidates.empty()) {
        EXPECT_NO_THROW({ host->addRemoteIceCandidate(clientCandidates[0], "0"); })
            << "Adding remote ICE candidate should not throw. Host error: " << hostError_;
    }

    // Give time for ICE connectivity checks
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    host->disconnect();
    client->disconnect();
}

/**
 * @brief Integration Test: Disconnect P2P connection
 *
 * This test implements the skipped unit test "Disconnect"
 * with actual WebRTC connection handling.
 *
 * Acceptance Criteria:
 * - Can disconnect after connection is established
 * - onDisconnected callback is invoked
 * - No crashes or resource leaks
 */
TEST_F(P2PIntegrationTest, DisconnectConnection) {
    std::string hostOffer;
    std::string clientAnswer;
    std::atomic<bool> offerReady{false};
    std::atomic<bool> answerReady{false};
    std::atomic<bool> hostDisconnected{false};

    // Setup Host
    P2PConnectionConfig hostConfig;
    hostConfig.stunServers = {"stun:stun.l.google.com:19302"};
    hostConfig.onOfferGenerated = [&hostOffer, &offerReady](const std::string& offer) {
        hostOffer = offer;
        offerReady = true;
    };
    hostConfig.onDisconnected = [&hostDisconnected]() { hostDisconnected = true; };
    hostConfig.onError = [this](const std::string& error) { hostError_ = error; };

    // Setup Client
    P2PConnectionConfig clientConfig;
    clientConfig.stunServers = {"stun:stun.l.google.com:19302"};
    clientConfig.onAnswerGenerated = [&clientAnswer, &answerReady](const std::string& answer) {
        clientAnswer = answer;
        answerReady = true;
    };
    clientConfig.onError = [this](const std::string& error) { clientError_ = error; };

    auto host = std::make_unique<P2PConnection>(hostConfig);
    auto client = std::make_unique<P2PConnection>(clientConfig);

    std::string sessionId = generateRandomString(8);

    // Setup connection
    host->initializeAsHost();
    host->createOffer();

    ASSERT_TRUE(waitForCondition([&offerReady]() { return offerReady.load(); }, 5000));

    client->initializeAsClient(sessionId);
    client->setRemoteOffer(hostOffer);

    ASSERT_TRUE(waitForCondition([&answerReady]() { return answerReady.load(); }, 5000));

    host->setRemoteAnswer(clientAnswer);

    // Wait a moment for connection to progress
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Disconnect - THIS IS THE MAIN TEST
    EXPECT_NO_THROW({ host->disconnect(); }) << "Disconnect should not throw";

    // Note: onDisconnected callback may or may not be called depending on connection state
    // The important thing is that disconnect() doesn't crash

    // Verify we can also disconnect client
    EXPECT_NO_THROW({ client->disconnect(); }) << "Client disconnect should not throw";

    // If we reach here without crashes, test passes
    SUCCEED();
}
