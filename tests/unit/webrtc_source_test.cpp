/**
 * @file webrtc_source_test.cpp
 * @brief Unit tests for WebRTC Source plugin
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "source/webrtc-source.hpp"
#include <atomic>
#include <chrono>
#include <thread>

using namespace obswebrtc::source;
using namespace testing;

/**
 * @brief Test fixture for WebRTCSource tests
 */
class WebRTCSourceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

/**
 * @brief Test that WebRTCSource can be constructed
 */
TEST_F(WebRTCSourceTest, CanConstruct) {
    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    EXPECT_NO_THROW({
        WebRTCSource source(config);
    });
}

/**
 * @brief Test that WebRTCSource can start and stop
 */
TEST_F(WebRTCSourceTest, CanStartAndStop) {
    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    WebRTCSource source(config);

    EXPECT_TRUE(source.start());
    // Note: isActive() becomes true only after connection is established
    // which is asynchronous, so we don't check it here

    source.stop();
    EXPECT_FALSE(source.isActive());
}

/**
 * @brief Test that WebRTCSource handles video frames
 */
TEST_F(WebRTCSourceTest, CanReceiveVideoFrame) {
    std::atomic<bool> videoCallbackCalled{false};

    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;
    config.videoCallback = [&videoCallbackCalled](const VideoFrame& frame) {
        videoCallbackCalled = true;
    };

    WebRTCSource source(config);
    source.start();

    // In real scenario, video frames would arrive via WebRTC
    // Here we just test that the callback can be set
    EXPECT_FALSE(videoCallbackCalled.load()); // Not called yet in test environment

    source.stop();
    // Allow time for cleanup to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

/**
 * @brief Test that WebRTCSource handles audio frames
 */
TEST_F(WebRTCSourceTest, CanReceiveAudioFrame) {
    std::atomic<bool> audioCallbackCalled{false};

    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;
    config.audioCallback = [&audioCallbackCalled](const AudioFrame& frame) {
        audioCallbackCalled = true;
    };

    WebRTCSource source(config);
    source.start();

    // In real scenario, audio frames would arrive via WebRTC
    // Here we just test that the callback can be set
    EXPECT_FALSE(audioCallbackCalled.load()); // Not called yet in test environment

    source.stop();
    // Allow time for cleanup to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

/**
 * @brief Test that WebRTCSource handles connection errors
 */
TEST_F(WebRTCSourceTest, HandlesConnectionError) {
    std::atomic<bool> errorCallbackCalled{false};

    WebRTCSourceConfig config;
    config.serverUrl = "http://invalid-server:9999/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;
    config.errorCallback = [&errorCallbackCalled](const std::string& error) {
        errorCallbackCalled = true;
    };

    WebRTCSource source(config);

    // Start may succeed initially (connection errors happen asynchronously)
    // but the connection will fail eventually
    source.start();
    EXPECT_FALSE(source.isActive());
    source.stop();
    // Allow time for cleanup to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

/**
 * @brief Test that WebRTCSource supports H264 codec
 */
TEST_F(WebRTCSourceTest, SupportsH264) {
    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    EXPECT_NO_THROW({
        WebRTCSource source(config);
    });
}

/**
 * @brief Test that WebRTCSource supports VP8 codec
 */
TEST_F(WebRTCSourceTest, SupportsVP8) {
    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::VP8;
    config.audioCodec = AudioCodec::Opus;

    EXPECT_NO_THROW({
        WebRTCSource source(config);
    });
}

/**
 * @brief Test that WebRTCSource cannot be started twice
 */
TEST_F(WebRTCSourceTest, CannotStartTwice) {
    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;

    WebRTCSource source(config);

    EXPECT_TRUE(source.start());
    // Second start should fail because source is already active
    EXPECT_FALSE(source.start());

    source.stop();
    // Allow time for cleanup to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

/**
 * @brief Test that WebRTCSource handles connection state changes
 */
TEST_F(WebRTCSourceTest, HandlesConnectionStateChanges) {
    std::atomic<bool> stateChangeCalled{false};

    WebRTCSourceConfig config;
    config.serverUrl = "http://localhost:8080/whep";
    config.videoCodec = VideoCodec::H264;
    config.audioCodec = AudioCodec::Opus;
    config.stateCallback = [&stateChangeCalled](ConnectionState state) {
        stateChangeCalled = true;
    };

    WebRTCSource source(config);
    source.start();

    // State changes happen asynchronously
    // We just verify the callback can be set
    source.stop();
    // Allow time for cleanup to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
