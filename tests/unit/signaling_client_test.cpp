/**
 * @file signaling_client_test.cpp
 * @brief Unit tests for SignalingClient
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core/signaling-client.hpp"
#include <thread>
#include <chrono>

using namespace obswebrtc::core;
using namespace testing;

/**
 * @brief Mock signaling transport for testing
 */
class MockSignalingTransport : public SignalingTransport {
public:
    MOCK_METHOD(void, connect, (const std::string& url), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(void, sendMessage, (const std::string& message), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
};

class SignalingClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = SignalingConfig{};
        config_.url = "wss://signaling.example.com";
        config_.onConnected = [this]() {
            connected_ = true;
        };
        config_.onDisconnected = [this]() {
            connected_ = false;
        };
        config_.onError = [this](const std::string& error) {
            lastError_ = error;
        };
        config_.onOffer = [this](const std::string& sdp) {
            receivedOffer_ = sdp;
        };
        config_.onAnswer = [this](const std::string& sdp) {
            receivedAnswer_ = sdp;
        };
        config_.onIceCandidate = [this](const std::string& candidate, const std::string& mid) {
            receivedCandidates_.push_back({candidate, mid});
        };
    }

    SignalingConfig config_;
    bool connected_ = false;
    std::string lastError_;
    std::string receivedOffer_;
    std::string receivedAnswer_;
    std::vector<std::pair<std::string, std::string>> receivedCandidates_;
};

/**
 * @brief Test SignalingClient construction
 */
TEST_F(SignalingClientTest, ConstructionWithValidConfig) {
    EXPECT_NO_THROW({
        auto client = std::make_unique<SignalingClient>(config_);
    });
}

/**
 * @brief Test SignalingClient construction with empty URL
 */
TEST_F(SignalingClientTest, ConstructionWithEmptyUrl) {
    config_.url = "";
    EXPECT_THROW({
        auto client = std::make_unique<SignalingClient>(config_);
    }, std::invalid_argument);
}

/**
 * @brief Test connection establishment
 */
TEST_F(SignalingClientTest, ConnectToSignalingServer) {
    auto client = std::make_unique<SignalingClient>(config_);

    // Initially not connected
    EXPECT_FALSE(client->isConnected());

    // Connect should not throw
    EXPECT_NO_THROW(client->connect());
}

/**
 * @brief Test disconnection
 */
TEST_F(SignalingClientTest, DisconnectFromSignalingServer) {
    auto client = std::make_unique<SignalingClient>(config_);

    // Connect first
    client->connect();

    // Disconnect should not throw
    EXPECT_NO_THROW(client->disconnect());

    // Should be disconnected
    EXPECT_FALSE(client->isConnected());
}

/**
 * @brief Test sending offer
 */
TEST_F(SignalingClientTest, SendOffer) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    EXPECT_NO_THROW(client->sendOffer(testOffer));
}

/**
 * @brief Test sending answer
 */
TEST_F(SignalingClientTest, SendAnswer) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string testAnswer = "v=0\r\no=- 789 012 IN IP4 0.0.0.0\r\n";

    EXPECT_NO_THROW(client->sendAnswer(testAnswer));
}

/**
 * @brief Test sending ICE candidate
 */
TEST_F(SignalingClientTest, SendIceCandidate) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    const std::string mid = "0";

    EXPECT_NO_THROW(client->sendIceCandidate(candidate, mid));
}

/**
 * @brief Test sending offer without connection should throw
 */
TEST_F(SignalingClientTest, SendOfferWithoutConnectionThrows) {
    auto client = std::make_unique<SignalingClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    EXPECT_THROW(client->sendOffer(testOffer), std::runtime_error);
}

/**
 * @brief Test sending answer without connection should throw
 */
TEST_F(SignalingClientTest, SendAnswerWithoutConnectionThrows) {
    auto client = std::make_unique<SignalingClient>(config_);

    const std::string testAnswer = "v=0\r\no=- 789 012 IN IP4 0.0.0.0\r\n";

    EXPECT_THROW(client->sendAnswer(testAnswer), std::runtime_error);
}

/**
 * @brief Test sending ICE candidate without connection should throw
 */
TEST_F(SignalingClientTest, SendIceCandidateWithoutConnectionThrows) {
    auto client = std::make_unique<SignalingClient>(config_);

    const std::string candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    const std::string mid = "0";

    EXPECT_THROW(client->sendIceCandidate(candidate, mid), std::runtime_error);
}

/**
 * @brief Test receiving offer via message
 */
TEST_F(SignalingClientTest, ReceiveOfferMessage) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    // Simulate receiving an offer message
    const std::string offerSdp = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    const std::string message = R"({
        "type": "offer",
        "sdp": ")" + offerSdp + R"("
    })";

    // Simulate message reception (in real implementation, this would come from WebSocket)
    client->handleMessage(message);

    // Check that offer callback was called
    EXPECT_EQ(receivedOffer_, offerSdp);
}

/**
 * @brief Test receiving answer via message
 */
TEST_F(SignalingClientTest, ReceiveAnswerMessage) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    // Simulate receiving an answer message
    const std::string answerSdp = "v=0\r\no=- 789 012 IN IP4 0.0.0.0\r\n";
    const std::string message = R"({
        "type": "answer",
        "sdp": ")" + answerSdp + R"("
    })";

    client->handleMessage(message);

    // Check that answer callback was called
    EXPECT_EQ(receivedAnswer_, answerSdp);
}

/**
 * @brief Test receiving ICE candidate via message
 */
TEST_F(SignalingClientTest, ReceiveIceCandidateMessage) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    // Simulate receiving an ICE candidate message
    const std::string candidateStr = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    const std::string midStr = "0";
    const std::string message = R"({
        "type": "candidate",
        "candidate": ")" + candidateStr + R"(",
        "mid": ")" + midStr + R"("
    })";

    client->handleMessage(message);

    // Check that ICE candidate callback was called
    ASSERT_EQ(receivedCandidates_.size(), 1);
    EXPECT_EQ(receivedCandidates_[0].first, candidateStr);
    EXPECT_EQ(receivedCandidates_[0].second, midStr);
}

/**
 * @brief Test receiving invalid JSON message
 */
TEST_F(SignalingClientTest, ReceiveInvalidJsonMessage) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string invalidMessage = "this is not json";

    // Should not throw, but should call error callback
    EXPECT_NO_THROW(client->handleMessage(invalidMessage));
    EXPECT_FALSE(lastError_.empty());
}

/**
 * @brief Test receiving message with unknown type
 */
TEST_F(SignalingClientTest, ReceiveUnknownMessageType) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    const std::string unknownMessage = R"({
        "type": "unknown",
        "data": "some data"
    })";

    // Should not throw, but should call error callback
    EXPECT_NO_THROW(client->handleMessage(unknownMessage));
    EXPECT_FALSE(lastError_.empty());
}

/**
 * @brief Test multiple connect/disconnect cycles
 */
TEST_F(SignalingClientTest, MultipleConnectDisconnectCycles) {
    auto client = std::make_unique<SignalingClient>(config_);

    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_THROW(client->connect());
        EXPECT_NO_THROW(client->disconnect());
        EXPECT_FALSE(client->isConnected());
    }
}

/**
 * @brief Test that client can be destroyed while connected
 */
TEST_F(SignalingClientTest, DestructionWhileConnected) {
    auto client = std::make_unique<SignalingClient>(config_);
    client->connect();

    // Destruction should not throw
    EXPECT_NO_THROW(client.reset());
}
