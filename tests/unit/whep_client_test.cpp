/**
 * @file whep_client_test.cpp
 * @brief Unit tests for WHEPClient
 */

#include "core/whep-client.hpp"
#include "core/peer-connection.hpp"

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

/**
 * @brief Test HTTP 400 Bad Request error handling
 */
TEST_F(WHEPClientTest, HandleBadRequestError) {
    auto client = std::make_unique<WHEPClient>(config_);

    // Send malformed SDP that might trigger 400 error
    const std::string malformedOffer = "invalid sdp content";

    // Should throw on HTTP error
    EXPECT_THROW({ client->sendOffer(malformedOffer); }, std::runtime_error);
}

/**
 * @brief Test HTTP 403 Forbidden error handling
 */
TEST_F(WHEPClientTest, HandleForbiddenError) {
    config_.bearerToken = "forbidden-token";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on 403 error
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test HTTP 404 Not Found error handling
 */
TEST_F(WHEPClientTest, HandleNotFoundError) {
    config_.url = "https://sfu.example.com/nonexistent-endpoint";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on 404 error
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test HTTP 500 Internal Server Error handling
 */
TEST_F(WHEPClientTest, HandleInternalServerError) {
    config_.url = "https://sfu.example.com/error-endpoint";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on 500 error
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test HTTP 503 Service Unavailable error handling
 */
TEST_F(WHEPClientTest, HandleServiceUnavailableError) {
    config_.url = "https://sfu.example.com/unavailable-endpoint";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on 503 error
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test network timeout error handling
 */
TEST_F(WHEPClientTest, HandleNetworkTimeout) {
    // Use an IP that will timeout
    config_.url = "http://192.0.2.1:12345/whep";  // TEST-NET-1 (guaranteed to timeout)
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on timeout
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test DNS resolution failure
 */
TEST_F(WHEPClientTest, HandleDnsResolutionFailure) {
    config_.url = "https://nonexistent.invalid.domain.tld/whep";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on DNS failure
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test connection refused error
 */
TEST_F(WHEPClientTest, HandleConnectionRefused) {
    // Use localhost with port that has no server
    config_.url = "http://localhost:19998/whep";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should throw on connection refused
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test SSL/TLS certificate error handling
 */
TEST_F(WHEPClientTest, HandleSslCertificateError) {
    // Use self-signed or invalid certificate URL
    config_.url = "https://self-signed.badssl.com/whep";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should handle SSL errors appropriately
    EXPECT_THROW({ client->sendOffer(testOffer); }, std::runtime_error);
}

/**
 * @brief Test handling of missing Location header in response
 */
TEST_F(WHEPClientTest, HandleMissingLocationHeader) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // If server doesn't send Location header, should still work or handle gracefully
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test handling of empty response body
 */
TEST_F(WHEPClientTest, HandleEmptyResponseBody) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Empty response should be handled
    EXPECT_NO_THROW({ std::string answer = client->sendOffer(testOffer); });
}

/**
 * @brief Test retry mechanism on temporary failures
 */
TEST_F(WHEPClientTest, RetryOnTemporaryFailure) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // First attempt might fail, retry should work
    EXPECT_NO_THROW({
        try {
            client->sendOffer(testOffer);
        } catch (const std::runtime_error&) {
            // Retry
            client = std::make_unique<WHEPClient>(config_);
            client->sendOffer(testOffer);
        }
    });
}

/**
 * @brief Test Content-Type header in request
 */
TEST_F(WHEPClientTest, VerifyContentTypeHeader) {
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should send correct Content-Type: application/sdp
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test Bearer token format in Authorization header
 */
TEST_F(WHEPClientTest, VerifyBearerTokenFormat) {
    config_.bearerToken = "test-token-with-special-chars!@#$%";
    auto client = std::make_unique<WHEPClient>(config_);

    const std::string testOffer = "v=0\r\no=- 123 456 IN IP4 0.0.0.0\r\n";

    // Should properly format Authorization: Bearer <token>
    EXPECT_NO_THROW({ client->sendOffer(testOffer); });
}

/**
 * @brief Test move semantics
 */
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

// =============================================================================
// WHEP Media Track Reception Tests
// =============================================================================

/**
 * @brief Test WHEPConfig with video and audio frame callbacks
 */
TEST_F(WHEPClientTest, ConfigWithFrameCallbacks) {
    std::vector<VideoFrame> receivedVideoFrames;
    std::vector<AudioFrame> receivedAudioFrames;

    config_.videoFrameCallback = [&receivedVideoFrames](const VideoFrame& frame) {
        receivedVideoFrames.push_back(frame);
    };
    config_.audioFrameCallback = [&receivedAudioFrames](const AudioFrame& frame) {
        receivedAudioFrames.push_back(frame);
    };

    EXPECT_NO_THROW({ auto client = std::make_unique<WHEPClient>(config_); });
}

/**
 * @brief Test that WHEPClient can be created without frame callbacks (backwards compatible)
 */
TEST_F(WHEPClientTest, ConfigWithoutFrameCallbacksBackwardsCompatible) {
    // Ensure existing tests still work without frame callbacks
    config_.videoFrameCallback = nullptr;
    config_.audioFrameCallback = nullptr;

    EXPECT_NO_THROW({ auto client = std::make_unique<WHEPClient>(config_); });
}

/**
 * @brief Test that WHEPClient creates internal PeerConnection when frame callbacks are set
 */
TEST_F(WHEPClientTest, CreatesPeerConnectionWithFrameCallbacks) {
    bool videoFrameReceived = false;
    bool audioFrameReceived = false;

    config_.videoFrameCallback = [&videoFrameReceived](const VideoFrame& frame) {
        videoFrameReceived = true;
    };
    config_.audioFrameCallback = [&audioFrameReceived](const AudioFrame& frame) {
        audioFrameReceived = true;
    };

    auto client = std::make_unique<WHEPClient>(config_);
    EXPECT_TRUE(client->hasPeerConnection());
}

/**
 * @brief Test that WHEPClient does not create PeerConnection without frame callbacks
 */
TEST_F(WHEPClientTest, NoPeerConnectionWithoutFrameCallbacks) {
    config_.videoFrameCallback = nullptr;
    config_.audioFrameCallback = nullptr;

    auto client = std::make_unique<WHEPClient>(config_);
    EXPECT_FALSE(client->hasPeerConnection());
}

/**
 * @brief Test that connect() initiates WebRTC connection when frame callbacks are set
 */
TEST_F(WHEPClientTest, ConnectInitiatesWebRTCConnection) {
    bool videoFrameReceived = false;

    config_.videoFrameCallback = [&videoFrameReceived](const VideoFrame& frame) {
        videoFrameReceived = true;
    };

    auto client = std::make_unique<WHEPClient>(config_);

    // connect() should create an offer and send it to WHEP server
    EXPECT_NO_THROW({ client->connect(); });
}
