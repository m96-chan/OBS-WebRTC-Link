/**
 * Unit tests for Audio-Only Mode
 *
 * Tests the functionality of audio-only mode configuration,
 * including video track disabling and audio quality presets.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../src/core/audio-only-config.hpp"

// Test fixture for audio-only mode tests
class AudioOnlyModeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }

    void TearDown() override {
        // Cleanup code
    }
};

/**
 * Test: Audio-only mode can be enabled
 */
TEST_F(AudioOnlyModeTest, CanEnableAudioOnlyMode) {
    AudioOnlyConfig config;
    config.setAudioOnly(true);
    EXPECT_TRUE(config.isAudioOnly());
}

/**
 * Test: Audio-only mode is disabled by default
 */
TEST_F(AudioOnlyModeTest, AudioOnlyModeDisabledByDefault) {
    AudioOnlyConfig config;
    EXPECT_FALSE(config.isAudioOnly());
}

/**
 * Test: Video track is disabled when audio-only mode is enabled
 */
TEST_F(AudioOnlyModeTest, VideoTrackDisabledInAudioOnlyMode) {
    // TODO: Implement PeerConnection video track control
    // MockPeerConnection peer;
    // peer.setAudioOnly(true);
    // EXPECT_FALSE(peer.hasVideoTrack());
    GTEST_SKIP() << "PeerConnection audio-only control not yet implemented";
}

/**
 * Test: Video track is enabled when audio-only mode is disabled
 */
TEST_F(AudioOnlyModeTest, VideoTrackEnabledWhenNotAudioOnly) {
    // TODO: Implement PeerConnection video track control
    // MockPeerConnection peer;
    // peer.setAudioOnly(false);
    // EXPECT_TRUE(peer.canHaveVideoTrack());
    GTEST_SKIP() << "PeerConnection audio-only control not yet implemented";
}

/**
 * Test: Audio quality preset - High (64 kbps)
 */
TEST_F(AudioOnlyModeTest, AudioQualityPresetHigh) {
    AudioOnlyConfig config;
    config.setAudioQualityPreset(AudioQuality::High);
    EXPECT_EQ(64, config.getAudioBitrate());
}

/**
 * Test: Audio quality preset - Medium (48 kbps)
 */
TEST_F(AudioOnlyModeTest, AudioQualityPresetMedium) {
    AudioOnlyConfig config;
    config.setAudioQualityPreset(AudioQuality::Medium);
    EXPECT_EQ(48, config.getAudioBitrate());
}

/**
 * Test: Audio quality preset - Low (32 kbps)
 */
TEST_F(AudioOnlyModeTest, AudioQualityPresetLow) {
    AudioOnlyConfig config;
    config.setAudioQualityPreset(AudioQuality::Low);
    EXPECT_EQ(32, config.getAudioBitrate());
}

/**
 * Test: Opus codec is used for audio-only mode
 */
TEST_F(AudioOnlyModeTest, UsesOpusCodecForAudioOnly) {
    AudioOnlyConfig config;
    config.setAudioOnly(true);
    EXPECT_EQ("opus", config.getAudioCodec());
}

/**
 * Test: Echo cancellation is enabled by default
 */
TEST_F(AudioOnlyModeTest, EchoCancellationEnabledByDefault) {
    AudioOnlyConfig config;
    EXPECT_TRUE(config.isEchoCancellationEnabled());
}

/**
 * Test: Echo cancellation can be disabled
 */
TEST_F(AudioOnlyModeTest, CanDisableEchoCancellation) {
    AudioOnlyConfig config;
    config.setEchoCancellation(false);
    EXPECT_FALSE(config.isEchoCancellationEnabled());
}
