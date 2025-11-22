/**
 * @file whip_client_test.cpp
 * @brief Unit tests for WHIPClient
 */

#include "core/whip-client.hpp"

#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thread>

using namespace obswebrtc::core;
using namespace testing;

/**
 * @brief Mock HTTP transport for testing
 */
class MockHTTPTransport {
public:
    virtual ~MockHTTPTransport() = default;

    MOCK_METHOD(HTTPResponse, post, (const std::string& url, const HTTPRequest& request), ());
    MOCK_METHOD(HTTPResponse, patch, (const std::string& url, const HTTPRequest& request), ());
    MOCK_METHOD(HTTPResponse, del, (const std::string& url, const HTTPRequest& request), ());
};

class WHIPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = WHIPConfig{};
        config_.url = "https://sfu.example.com/whip";
        config_.bearerToken = "test-token-12345";
        config_.onConnected = [this]() { connected_ = true; };
        config_.onDisconnected = [this]() { connected_ = false; };
        config_.onError = [this](const std::string& error) { lastError_ = error; };
        config_.onIceCandidate = [this](const std::string& candidate, const std::string& mid) {
            receivedCandidates_.push_back({candidate, mid});
        };
    }

    WHIPConfig config_;
    bool connected_ = false;
    std::string lastError_;
    std::vector<std::pair<std::string, std::string>> receivedCandidates_;
};

/**
 * @brief Test WHIPClient construction
 */
TEST_F(WHIPClientTest, ConstructionWithValidConfig) {
    EXPECT_NO_THROW({ auto client = std::make_unique<WHIPClient>(config_); });
}

/**
 * @brief Test WHIPClient construction with empty URL
 */
TEST_F(WHIPClientTest, ConstructionWithEmptyUrl) {
    config_.url = "";
    EXPECT_THROW({ auto client = std::make_unique<WHIPClient>(config_); }, std::invalid_argument);
}

/**
 * @brief Test sending offer to WHIP server
 */
TEST_F(WHIPClientTest, SendOffer) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    std::string receivedAnswer;

    EXPECT_NO_THROW({ receivedAnswer = client->sendOffer(testOffer); });
}

/**
 * @brief Test sending offer with invalid SDP should throw
 */
TEST_F(WHIPClientTest, SendOfferWithInvalidSDP) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string invalidOffer = "";

    EXPECT_THROW({ client->sendOffer(invalidOffer); }, std::invalid_argument);
}

/**
 * @brief Test sending ICE candidate via PATCH
 */
TEST_F(WHIPClientTest, SendIceCandidate) {
    auto client = std::make_unique<WHIPClient>(config_);

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
TEST_F(WHIPClientTest, SendIceCandidateWithoutConnectionThrows) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";
    const std::string mid = "0";

    EXPECT_THROW({ client->sendIceCandidate(candidate, mid); }, std::runtime_error);
}

/**
 * @brief Test bearer token authentication
 */
TEST_F(WHIPClientTest, BearerTokenAuthentication) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should include bearer token in request
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test disconnection
 */
TEST_F(WHIPClientTest, Disconnect) {
    auto client = std::make_unique<WHIPClient>(config_);

    // First send offer to establish connection
    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    // Disconnect should not throw
    EXPECT_NO_THROW(client->disconnect());
}

/**
 * @brief Test that client can be destroyed while connected
 */
TEST_F(WHIPClientTest, DestructionWhileConnected) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    // Destruction should not throw
    EXPECT_NO_THROW(client.reset());
}

/**
 * @brief Test HTTP 401 unauthorized error handling
 */
TEST_F(WHIPClientTest, HandleUnauthorizedError) {
    config_.bearerToken = "invalid-token";
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on 401 error
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test handling of Location header in response
 */
TEST_F(WHIPClientTest, HandleLocationHeader) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should store resource URL from Location header
    EXPECT_NO_THROW({ std::string answer = client->sendOffer(testOffer); });
}

/**
 * @brief Test isConnected status
 */
TEST_F(WHIPClientTest, IsConnected) {
    auto client = std::make_unique<WHIPClient>(config_);

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

// ========== Additional Edge Cases and Error Handling Tests ==========

/**
 * @brief Test multiple disconnect calls
 */
TEST_F(WHIPClientTest, MultipleDisconnectCalls) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    // Multiple disconnect calls should not throw
    EXPECT_NO_THROW({
        client->disconnect();
        client->disconnect();
        client->disconnect();
    });

    EXPECT_FALSE(client->isConnected());
}

/**
 * @brief Test sending offer twice
 */
TEST_F(WHIPClientTest, SendOfferTwice) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // First offer
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });

    // Second offer should replace first connection
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });

    EXPECT_TRUE(client->isConnected());
}

/**
 * @brief Test empty bearer token
 */
TEST_F(WHIPClientTest, EmptyBearerToken) {
    config_.bearerToken = "";
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should still work without bearer token
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test with very long bearer token
 */
TEST_F(WHIPClientTest, VeryLongBearerToken) {
    config_.bearerToken = std::string(10000, 'a');
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should handle long tokens
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test with malformed URL
 */
TEST_F(WHIPClientTest, MalformedUrl) {
    config_.url = "not-a-valid-url";

    EXPECT_THROW({
        auto client = std::make_unique<WHIPClient>(config_);
    }, std::invalid_argument);
}

/**
 * @brief Test with URL without scheme
 */
TEST_F(WHIPClientTest, UrlWithoutScheme) {
    config_.url = "sfu.example.com/whip";

    EXPECT_THROW({
        auto client = std::make_unique<WHIPClient>(config_);
    }, std::invalid_argument);
}

/**
 * @brief Test sending ICE candidate with empty candidate
 */
TEST_F(WHIPClientTest, SendEmptyIceCandidate) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    EXPECT_THROW({
        client->sendIceCandidate("", "0");
    }, std::invalid_argument);
}

/**
 * @brief Test sending ICE candidate with empty mid
 */
TEST_F(WHIPClientTest, SendIceCandidateWithEmptyMid) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    const std::string candidate = "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host";

    // Empty mid should be handled gracefully
    EXPECT_NO_THROW({
        client->sendIceCandidate(candidate, "");
    });
}

/**
 * @brief Test SDP with special characters
 */
TEST_F(WHIPClientTest, SdpWithSpecialCharacters) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\na=test:value with spaces\r\n";

    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test very large SDP
 */
TEST_F(WHIPClientTest, VeryLargeSdp) {
    auto client = std::make_unique<WHIPClient>(config_);

    std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    // Add many attributes to make SDP large
    for (int i = 0; i < 1000; i++) {
        testOffer += "a=test" + std::to_string(i) + ":value\r\n";
    }

    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test move semantics
 */
TEST_F(WHIPClientTest, MoveSemantics) {
    auto client1 = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client1->sendOffer(testOffer);

    EXPECT_TRUE(client1->isConnected());

    // Move construction
    auto client2 = std::move(client1);
    EXPECT_TRUE(client2->isConnected());

    // Move assignment
    auto client3 = std::make_unique<WHIPClient>(config_);
    client3 = std::move(client2);
    EXPECT_TRUE(client3->isConnected());
}

/**
 * @brief Test callback invocations
 */
TEST_F(WHIPClientTest, CallbackInvocations) {
    int connectedCount = 0;
    int disconnectedCount = 0;
    int errorCount = 0;

    config_.onConnected = [&connectedCount]() { connectedCount++; };
    config_.onDisconnected = [&disconnectedCount]() { disconnectedCount++; };
    config_.onError = [&errorCount](const std::string&) { errorCount++; };

    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";
    client->sendOffer(testOffer);

    // Connected callback should be called
    EXPECT_GT(connectedCount, 0);

    client->disconnect();

    // Disconnected callback should be called
    EXPECT_GT(disconnectedCount, 0);
}

/**
 * @brief Test concurrent operations
 */
TEST_F(WHIPClientTest, ConcurrentOperations) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    std::thread t1([&client, &testOffer]() {
        EXPECT_NO_THROW({ client->sendOffer(testOffer); });
    });

    std::thread t2([&client]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        EXPECT_NO_THROW({ client->disconnect(); });
    });

    t1.join();
    t2.join();
}

/**
 * @brief Test performance - offer sending should complete quickly
 */
TEST_F(WHIPClientTest, OfferSendingPerformance) {
    auto client = std::make_unique<WHIPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    auto start = std::chrono::steady_clock::now();
    client->sendOffer(testOffer);
    auto end = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete within reasonable time (< 5 seconds for network operation)
    EXPECT_LT(duration.count(), 5000);
}
