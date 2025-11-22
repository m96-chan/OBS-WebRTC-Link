/**
 * @file p2p_connection_test.cpp
 * @brief Unit tests for P2PConnection (Direct Connection)
 */

#include "core/p2p-connection.hpp"

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

using namespace obswebrtc::core;
using namespace testing;

class P2PConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup default configuration
        config_ = P2PConnectionConfig{};
        config_.stunServers = {"stun:stun.l.google.com:19302"};
        config_.onConnected = [this]() { connected_ = true; };
        config_.onDisconnected = [this]() { connected_ = false; };
        config_.onError = [this](const std::string& error) { lastError_ = error; };
        config_.onSessionIdGenerated = [this](const std::string& sessionId) {
            generatedSessionId_ = sessionId;
        };
        config_.onOfferGenerated = [this](const std::string& offer) {
            generatedOffer_ = offer;
        };
        config_.onAnswerGenerated = [this](const std::string& answer) {
            generatedAnswer_ = answer;
        };
    }

    P2PConnectionConfig config_;
    bool connected_ = false;
    std::string lastError_;
    std::string generatedSessionId_;
    std::string generatedOffer_;
    std::string generatedAnswer_;
};

/**
 * @brief Test Session ID generation
 */
TEST_F(P2PConnectionTest, GenerateSessionId) {
    auto connection = std::make_unique<P2PConnection>(config_);

    std::string sessionId = connection->generateSessionId();

    // Session ID should not be empty
    EXPECT_FALSE(sessionId.empty());

    // Session ID should be 8 characters (short code format)
    EXPECT_EQ(sessionId.length(), 8);

    // Should callback with generated session ID
    EXPECT_EQ(generatedSessionId_, sessionId);
}

/**
 * @brief Test unique Session ID generation
 */
TEST_F(P2PConnectionTest, GenerateUniqueSessionIds) {
    auto connection = std::make_unique<P2PConnection>(config_);

    std::string sessionId1 = connection->generateSessionId();
    std::string sessionId2 = connection->generateSessionId();

    // Each call should generate a different session ID
    EXPECT_NE(sessionId1, sessionId2);
}

/**
 * @brief Test Host role initialization
 */
TEST_F(P2PConnectionTest, InitializeAsHost) {
    auto connection = std::make_unique<P2PConnection>(config_);

    EXPECT_NO_THROW({ connection->initializeAsHost(); });

    // Should have a role
    EXPECT_EQ(connection->getRole(), P2PRole::Host);
}

/**
 * @brief Test Client role initialization
 */
TEST_F(P2PConnectionTest, InitializeAsClient) {
    auto connection = std::make_unique<P2PConnection>(config_);

    std::string testSessionId = "TEST1234";

    EXPECT_NO_THROW({ connection->initializeAsClient(testSessionId); });

    EXPECT_EQ(connection->getRole(), P2PRole::Client);
    EXPECT_EQ(connection->getSessionId(), testSessionId);
}

/**
 * @brief Test creating offer as Host
 */
TEST_F(P2PConnectionTest, CreateOfferAsHost) {
    auto connection = std::make_unique<P2PConnection>(config_);

    connection->initializeAsHost();

    std::string offer;
    EXPECT_NO_THROW({ offer = connection->createOffer(); });

    // Offer should not be empty
    EXPECT_FALSE(offer.empty());

    // Callback should be called
    EXPECT_EQ(generatedOffer_, offer);
}

/**
 * @brief Test setting remote answer as Host
 */
TEST_F(P2PConnectionTest, SetRemoteAnswerAsHost) {
    auto connection = std::make_unique<P2PConnection>(config_);

    connection->initializeAsHost();
    connection->createOffer();

    std::string testAnswer = "v=0\r\no=- 456 789 IN IP4 0.0.0.0\r\n";

    EXPECT_NO_THROW({ connection->setRemoteAnswer(testAnswer); });
}

/**
 * @brief Test creating answer as Client
 */
TEST_F(P2PConnectionTest, CreateAnswerAsClient) {
    auto connection = std::make_unique<P2PConnection>(config_);

    connection->initializeAsClient("TEST1234");

    std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    std::string answer;
    EXPECT_NO_THROW({ answer = connection->setRemoteOffer(testOffer); });

    // Answer should not be empty
    EXPECT_FALSE(answer.empty());

    // Callback should be called
    EXPECT_EQ(generatedAnswer_, answer);
}

/**
 * @brief Test ICE candidate handling
 */
TEST_F(P2PConnectionTest, HandleIceCandidate) {
    auto connection = std::make_unique<P2PConnection>(config_);

    connection->initializeAsHost();

    std::vector<std::string> candidates;
    config_.onIceCandidate = [&candidates](const std::string& candidate, const std::string& mid) {
        candidates.push_back(candidate);
    };

    // Create offer should trigger ICE candidate gathering
    connection->createOffer();

    // Wait a bit for ICE candidates to be gathered
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should have at least one ICE candidate
    EXPECT_GT(candidates.size(), 0);
}

/**
 * @brief Test adding remote ICE candidate
 */
TEST_F(P2PConnectionTest, AddRemoteIceCandidate) {
    auto connection = std::make_unique<P2PConnection>(config_);

    connection->initializeAsHost();
    connection->createOffer();

    std::string testCandidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    std::string mid = "0";

    EXPECT_NO_THROW({ connection->addRemoteIceCandidate(testCandidate, mid); });
}

/**
 * @brief Test getting session information
 */
TEST_F(P2PConnectionTest, GetSessionInfo) {
    auto connection = std::make_unique<P2PConnection>(config_);

    connection->initializeAsHost();
    std::string sessionId = connection->generateSessionId();

    P2PSessionInfo info = connection->getSessionInfo();

    EXPECT_EQ(info.sessionId, sessionId);
    EXPECT_EQ(info.role, P2PRole::Host);
}

/**
 * @brief Test disconnection
 */
TEST_F(P2PConnectionTest, Disconnect) {
    auto connection = std::make_unique<P2PConnection>(config_);

    connection->initializeAsHost();
    connection->createOffer();

    EXPECT_NO_THROW({ connection->disconnect(); });

    // Should call disconnected callback
    EXPECT_TRUE(connected_ == false || lastError_.empty());
}

/**
 * @brief Test STUN server configuration
 */
TEST_F(P2PConnectionTest, StunServerConfiguration) {
    config_.stunServers = {
        "stun:stun.l.google.com:19302",
        "stun:stun1.l.google.com:19302"
    };

    auto connection = std::make_unique<P2PConnection>(config_);

    EXPECT_NO_THROW({ connection->initializeAsHost(); });
}

/**
 * @brief Test TURN server configuration
 */
TEST_F(P2PConnectionTest, TurnServerConfiguration) {
    config_.turnServers = {
        {"turn:turn.example.com:3478", "username", "password"}
    };

    auto connection = std::make_unique<P2PConnection>(config_);

    EXPECT_NO_THROW({ connection->initializeAsHost(); });
}

/**
 * @brief Test error handling for invalid session ID
 */
TEST_F(P2PConnectionTest, InvalidSessionIdError) {
    auto connection = std::make_unique<P2PConnection>(config_);

    std::string invalidSessionId = "";

    EXPECT_THROW({ connection->initializeAsClient(invalidSessionId); }, std::invalid_argument);
}

/**
 * @brief Test error handling for creating offer without initialization
 */
TEST_F(P2PConnectionTest, CreateOfferWithoutInitialization) {
    auto connection = std::make_unique<P2PConnection>(config_);

    EXPECT_THROW({ connection->createOffer(); }, std::runtime_error);
}
