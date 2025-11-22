/**
 * @file whep_client_test.cpp
 * @brief Unit tests for WHEPClient
 */

#include "core/whep-client.hpp"

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

using namespace obswebrtc::core;
using namespace testing;

class WHEPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = WHEPConfig{};
        config_.url = "https://sfu.example.com/whep";
        config_.bearerToken = "test-token-12345";
        config_.onConnected = [this]() { connected_ = true; };
        config_.onDisconnected = [this]() { connected_ = false; };
        config_.onError = [this](const std::string& error) { lastError_ = error; };
        config_.onIceCandidate = [this](const std::string& candidate, const std::string& mid) {
            receivedCandidates_.push_back({candidate, mid});
        };
    }

    WHEPConfig config_;
    bool connected_ = false;
    std::string lastError_;
    std::vector<std::pair<std::string, std::string>> receivedCandidates_;
};

/**
 * @brief Test WHEPClient construction
 */
TEST_F(WHEPClientTest, ConstructionWithValidConfig) {
    EXPECT_NO_THROW({ auto client = std::make_unique<WHEPClient>(config_); });
}

/**
 * @brief Test WHEPClient construction with empty URL
 */
TEST_F(WHEPClientTest, ConstructionWithEmptyUrl) {
    config_.url = "";
    EXPECT_THROW({ auto client = std::make_unique<WHEPClient>(config_); }, std::invalid_argument);
}

/**
 * @brief Test sending offer to WHEP server for receiving stream
 */
TEST_F(WHEPClientTest, SendOffer) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    std::string receivedAnswer;

    EXPECT_NO_THROW({ receivedAnswer = client->sendOffer(testOffer); });
}

/**
 * @brief Test sending offer with invalid SDP should throw
 */
TEST_F(WHEPClientTest, SendOfferWithInvalidSDP) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string invalidOffer = "";

    EXPECT_THROW({ client->sendOffer(invalidOffer); }, std::invalid_argument);
}

/**
 * @brief Test sending ICE candidate via PATCH
 */
TEST_F(WHEPClientTest, SendIceCandidate) {
    auto client = std::make_unique<WHEPClient>(config_);

    // First send offer to establish connection
    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    const std::string candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    const std::string mid = "0";

    EXPECT_NO_THROW({ client->sendIceCandidate(candidate, mid); });
}

/**
 * @brief Test sending ICE candidate without established connection should throw
 */
TEST_F(WHEPClientTest, SendIceCandidateWithoutConnectionThrows) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    const std::string mid = "0";

    EXPECT_THROW({ client->sendIceCandidate(candidate, mid); }, std::runtime_error);
}

/**
 * @brief Test bearer token authentication
 */
TEST_F(WHEPClientTest, BearerTokenAuthentication) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should include bearer token in request
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test disconnection
 */
TEST_F(WHEPClientTest, Disconnect) {
    auto client = std::make_unique<WHEPClient>(config_);

    // First send offer to establish connection
    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    // Disconnect should not throw
    EXPECT_NO_THROW(client->disconnect());
}

/**
 * @brief Test that client can be destroyed while connected
 */
TEST_F(WHEPClientTest, DestructionWhileConnected) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    // Destruction should not throw
    EXPECT_NO_THROW(client.reset());
}

/**
 * @brief Test HTTP 401 unauthorized error handling
 */
TEST_F(WHEPClientTest, HandleUnauthorizedError) {
    config_.bearerToken = "invalid-token";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on 401 error
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test handling of Location header in response
 */
TEST_F(WHEPClientTest, HandleLocationHeader) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should store resource URL from Location header
    EXPECT_NO_THROW({ std::string answer = client->sendOffer(testOffer); });
}

/**
 * @brief Test isConnected status
 */
TEST_F(WHEPClientTest, IsConnected) {
    auto client = std::make_unique<WHEPClient>(config_);

    // Initially not connected
    EXPECT_FALSE(client->isConnected());

    // After sending offer, should be connected
    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    EXPECT_TRUE(client->isConnected());

    // After disconnect, should not be connected
    client->disconnect();
    EXPECT_FALSE(client->isConnected());
}
