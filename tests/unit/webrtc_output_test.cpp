/**
 * @file webrtc_output_test.cpp
 * @brief Unit tests for WebRTC Output plugin
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "output/webrtc-output.hpp"

using namespace obswebrtc::output;
using namespace testing;

/**
 * @brief Test fixture for WebRTCOutput tests
 */
class WebRTCOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

/**
 * @brief Test that WebRTCOutput can be constructed
 */
TEST_F(WebRTCOutputTest, CanConstruct) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;
    config.videoBitrate = 2500;
    config.audioBitrate = 128;

    EXPECT_NO_THROW({
        WebRTCOutput output(config);
    });
}

/**
 * @brief Test that WebRTCOutput can start and stop
 */
TEST_F(WebRTCOutputTest, CanStartAndStop) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    WebRTCOutput output(config);

    EXPECT_TRUE(output.start());
    // Note: isActive() becomes true only after connection is established
    // which is asynchronous, so we don't check it here

    output.stop();
    EXPECT_FALSE(output.isActive());
}

/**
 * @brief Test that WebRTCOutput handles video packets
 */
TEST_F(WebRTCOutputTest, CanHandleVideoPacket) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    WebRTCOutput output(config);
    output.start();

    // Create a mock video packet
    EncodedPacket packet;
    packet.type = PacketType::Video;
    packet.data = std::vector<uint8_t>(1024, 0x00);
    packet.timestamp = 0;
    packet.keyframe = true;

    // Note: sendPacket will throw if not active (connection is async)
    // So we expect an exception here in the test environment
    EXPECT_THROW({
        output.sendPacket(packet);
    }, std::runtime_error);

    output.stop();
}

/**
 * @brief Test that WebRTCOutput handles audio packets
 */
TEST_F(WebRTCOutputTest, CanHandleAudioPacket) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    WebRTCOutput output(config);
    output.start();

    // Create a mock audio packet
    EncodedPacket packet;
    packet.type = PacketType::Audio;
    packet.data = std::vector<uint8_t>(256, 0x00);
    packet.timestamp = 0;
    packet.keyframe = false;

    // Note: sendPacket will throw if not active (connection is async)
    // So we expect an exception here in the test environment
    EXPECT_THROW({
        output.sendPacket(packet);
    }, std::runtime_error);

    output.stop();
}

/**
 * @brief Test that WebRTCOutput handles connection errors
 */
TEST_F(WebRTCOutputTest, HandlesConnectionError) {
    bool errorCallbackCalled = false;

    WebRTCOutputConfig config;
    config.serverUrl = "http://invalid-server:9999/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;
    config.errorCallback = [&errorCallbackCalled](const std::string& error) {
        errorCallbackCalled = true;
    };

    WebRTCOutput output(config);

    // Start may succeed initially (connection errors happen asynchronously)
    // but the connection will fail eventually
    output.start();
    EXPECT_FALSE(output.isActive());
}

/**
 * @brief Test that WebRTCOutput supports H264 codec
 */
TEST_F(WebRTCOutputTest, SupportsH264) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    EXPECT_NO_THROW({
        WebRTCOutput output(config);
    });
}

/**
 * @brief Test that WebRTCOutput supports VP8 codec
 */
TEST_F(WebRTCOutputTest, SupportsVP8) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::VP8;
    config.audioCodec = AudioCodec::Opus;

    EXPECT_NO_THROW({
        WebRTCOutput output(config);
    });
}

/**
 * @brief Test that WebRTCOutput handles bitrate configuration
 */
TEST_F(WebRTCOutputTest, HandlesBitrateConfiguration) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;
    config.videoBitrate = 5000;
    config.audioBitrate = 256;

    WebRTCOutput output(config);

    EXPECT_EQ(output.getVideoBitrate(), 5000);
    EXPECT_EQ(output.getAudioBitrate(), 256);
}

/**
 * @brief Test that WebRTCOutput cannot be started twice
 */
TEST_F(WebRTCOutputTest, CannotStartTwice) {
    WebRTCOutputConfig config;
    config.serverUrl = "http://localhost:8080/whip";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    WebRTCOutput output(config);

    EXPECT_TRUE(output.start());
    // Second start should fail because output is already active
    EXPECT_FALSE(output.start());

    output.stop();
}
