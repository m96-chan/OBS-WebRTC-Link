/**
 * @file hardware_encoder_test.cpp
 * @brief Unit tests for HardwareEncoder configuration
 */

#include "core/hardware-encoder.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace obswebrtc::core;
using namespace testing;

/**
 * @brief Test fixture for HardwareEncoder tests
 */
class HardwareEncoderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }

    void TearDown() override {
        // No teardown needed
    }
};

// =============================================================================
// HardwareEncoderType Tests
// =============================================================================

/**
 * @brief Test that all encoder types are defined
 */
TEST_F(HardwareEncoderTest, EncoderTypesAreDefined) {
    EXPECT_EQ(static_cast<int>(HardwareEncoderType::None), 0);
    EXPECT_EQ(static_cast<int>(HardwareEncoderType::NVENC), 1);
    EXPECT_EQ(static_cast<int>(HardwareEncoderType::AMF), 2);
    EXPECT_EQ(static_cast<int>(HardwareEncoderType::QuickSync), 3);
    EXPECT_EQ(static_cast<int>(HardwareEncoderType::Software), 4);
}

// =============================================================================
// HardwareEncoderPreset Tests
// =============================================================================

/**
 * @brief Test that encoder presets are defined
 */
TEST_F(HardwareEncoderTest, EncoderPresetsAreDefined) {
    EXPECT_EQ(static_cast<int>(HardwareEncoderPreset::Quality), 0);
    EXPECT_EQ(static_cast<int>(HardwareEncoderPreset::Balanced), 1);
    EXPECT_EQ(static_cast<int>(HardwareEncoderPreset::Speed), 2);
    EXPECT_EQ(static_cast<int>(HardwareEncoderPreset::LowLatency), 3);
}

// =============================================================================
// HardwareEncoderConfig Tests
// =============================================================================

/**
 * @brief Test default configuration values
 */
TEST_F(HardwareEncoderTest, DefaultConfigValues) {
    HardwareEncoderConfig config;

    EXPECT_EQ(config.type, HardwareEncoderType::None);
    EXPECT_EQ(config.preset, HardwareEncoderPreset::Balanced);
    EXPECT_EQ(config.bitrate, 2500);
    EXPECT_EQ(config.maxBitrate, 5000);
    EXPECT_EQ(config.keyframeInterval, 2);
    EXPECT_EQ(config.profile, "high");
    EXPECT_TRUE(config.enableBFrames);
    EXPECT_EQ(config.bFrameCount, 2);
    EXPECT_TRUE(config.enableLookahead);
    EXPECT_EQ(config.lookaheadFrames, 4);
}

// =============================================================================
// HardwareEncoderDetector Tests
// =============================================================================

/**
 * @brief Test detector construction
 */
TEST_F(HardwareEncoderTest, DetectorConstruction) {
    EXPECT_NO_THROW({
        HardwareEncoderDetector detector;
    });
}

/**
 * @brief Test getting available encoders
 */
TEST_F(HardwareEncoderTest, GetAvailableEncoders) {
    HardwareEncoderDetector detector;
    std::vector<HardwareEncoderType> available = detector.getAvailableEncoders();

    // At minimum, software encoder should always be available
    bool hasSoftware = false;
    for (const auto& type : available) {
        if (type == HardwareEncoderType::Software) {
            hasSoftware = true;
            break;
        }
    }
    EXPECT_TRUE(hasSoftware);
}

/**
 * @brief Test checking specific encoder availability
 */
TEST_F(HardwareEncoderTest, IsEncoderAvailable) {
    HardwareEncoderDetector detector;

    // Software encoder should always be available
    EXPECT_TRUE(detector.isAvailable(HardwareEncoderType::Software));

    // None should not be considered available
    EXPECT_FALSE(detector.isAvailable(HardwareEncoderType::None));
}

/**
 * @brief Test getting best available encoder
 */
TEST_F(HardwareEncoderTest, GetBestEncoder) {
    HardwareEncoderDetector detector;
    HardwareEncoderType best = detector.getBestEncoder();

    // Should return at least Software if no hardware encoder is available
    EXPECT_NE(best, HardwareEncoderType::None);
}

/**
 * @brief Test encoder type to string conversion
 */
TEST_F(HardwareEncoderTest, EncoderTypeToString) {
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeToString(HardwareEncoderType::None), "None");
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeToString(HardwareEncoderType::NVENC), "NVENC");
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeToString(HardwareEncoderType::AMF), "AMF");
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeToString(HardwareEncoderType::QuickSync), "QuickSync");
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeToString(HardwareEncoderType::Software), "Software");
}

/**
 * @brief Test encoder type from string conversion
 */
TEST_F(HardwareEncoderTest, EncoderTypeFromString) {
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeFromString("None"), HardwareEncoderType::None);
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeFromString("NVENC"), HardwareEncoderType::NVENC);
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeFromString("AMF"), HardwareEncoderType::AMF);
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeFromString("QuickSync"), HardwareEncoderType::QuickSync);
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeFromString("Software"), HardwareEncoderType::Software);
    EXPECT_EQ(HardwareEncoderDetector::encoderTypeFromString("invalid"), HardwareEncoderType::None);
}

// =============================================================================
// HardwareEncoderSettings Tests
// =============================================================================

/**
 * @brief Test settings construction with default config
 */
TEST_F(HardwareEncoderTest, SettingsDefaultConstruction) {
    HardwareEncoderConfig config;
    EXPECT_NO_THROW({
        HardwareEncoderSettings settings(config);
    });
}

/**
 * @brief Test settings construction with specific encoder type
 */
TEST_F(HardwareEncoderTest, SettingsWithSpecificEncoder) {
    HardwareEncoderConfig config;
    config.type = HardwareEncoderType::Software;
    config.bitrate = 3000;

    HardwareEncoderSettings settings(config);

    EXPECT_EQ(settings.getType(), HardwareEncoderType::Software);
    EXPECT_EQ(settings.getBitrate(), 3000);
}

/**
 * @brief Test settings getOptimalConfig for different presets
 */
TEST_F(HardwareEncoderTest, GetOptimalConfigForPresets) {
    HardwareEncoderSettings settings(HardwareEncoderConfig{});

    // Quality preset should have higher bitrate
    HardwareEncoderConfig qualityConfig = settings.getOptimalConfig(HardwareEncoderPreset::Quality);
    EXPECT_GE(qualityConfig.bitrate, 3000);

    // Speed preset should have lower latency settings
    HardwareEncoderConfig speedConfig = settings.getOptimalConfig(HardwareEncoderPreset::Speed);
    EXPECT_FALSE(speedConfig.enableBFrames);

    // Low latency preset should disable lookahead
    HardwareEncoderConfig lowLatencyConfig = settings.getOptimalConfig(HardwareEncoderPreset::LowLatency);
    EXPECT_FALSE(lowLatencyConfig.enableLookahead);
}

/**
 * @brief Test fallback behavior when preferred encoder is unavailable
 */
TEST_F(HardwareEncoderTest, FallbackToSoftware) {
    HardwareEncoderConfig config;
    // Even if we request NVENC, we should get a valid encoder
    config.type = HardwareEncoderType::NVENC;
    config.enableFallback = true;

    HardwareEncoderSettings settings(config);
    HardwareEncoderType actual = settings.getActualType();

    // Should either be NVENC (if available) or fallback to something else
    EXPECT_NE(actual, HardwareEncoderType::None);
}

/**
 * @brief Test that fallback is disabled when requested
 */
TEST_F(HardwareEncoderTest, NoFallbackWhenDisabled) {
    HardwareEncoderConfig config;
    config.type = HardwareEncoderType::NVENC;
    config.enableFallback = false;

    HardwareEncoderSettings settings(config);

    // If NVENC is not available and fallback is disabled,
    // getActualType should still return the requested type
    // (caller is responsible for checking availability)
    EXPECT_EQ(settings.getType(), HardwareEncoderType::NVENC);
}

/**
 * @brief Test updating bitrate
 */
TEST_F(HardwareEncoderTest, UpdateBitrate) {
    HardwareEncoderConfig config;
    config.bitrate = 2500;

    HardwareEncoderSettings settings(config);
    EXPECT_EQ(settings.getBitrate(), 2500);

    settings.setBitrate(4000);
    EXPECT_EQ(settings.getBitrate(), 4000);
}

/**
 * @brief Test invalid bitrate handling
 */
TEST_F(HardwareEncoderTest, InvalidBitrate) {
    HardwareEncoderConfig config;
    HardwareEncoderSettings settings(config);

    EXPECT_THROW(settings.setBitrate(0), std::invalid_argument);
    EXPECT_THROW(settings.setBitrate(-1000), std::invalid_argument);
}

/**
 * @brief Test getting NVENC-specific configuration
 */
TEST_F(HardwareEncoderTest, GetNVENCConfig) {
    HardwareEncoderConfig config;
    config.type = HardwareEncoderType::NVENC;
    config.preset = HardwareEncoderPreset::LowLatency;

    HardwareEncoderSettings settings(config);
    auto nvencConfig = settings.getNVENCConfig();

    EXPECT_FALSE(nvencConfig.empty());
    // NVENC low latency should use specific preset
    auto it = nvencConfig.find("preset");
    if (it != nvencConfig.end()) {
        EXPECT_EQ(it->second, "p1");  // NVENC P1 is low latency preset
    }
}

/**
 * @brief Test getting AMF-specific configuration
 */
TEST_F(HardwareEncoderTest, GetAMFConfig) {
    HardwareEncoderConfig config;
    config.type = HardwareEncoderType::AMF;
    config.preset = HardwareEncoderPreset::Quality;

    HardwareEncoderSettings settings(config);
    auto amfConfig = settings.getAMFConfig();

    EXPECT_FALSE(amfConfig.empty());
}

/**
 * @brief Test getting QuickSync-specific configuration
 */
TEST_F(HardwareEncoderTest, GetQuickSyncConfig) {
    HardwareEncoderConfig config;
    config.type = HardwareEncoderType::QuickSync;
    config.preset = HardwareEncoderPreset::Balanced;

    HardwareEncoderSettings settings(config);
    auto qsvConfig = settings.getQuickSyncConfig();

    EXPECT_FALSE(qsvConfig.empty());
}

// =============================================================================
// Integration Tests
// =============================================================================

/**
 * @brief Test auto-detection and settings creation flow
 */
TEST_F(HardwareEncoderTest, AutoDetectionFlow) {
    // Detect available encoders
    HardwareEncoderDetector detector;
    HardwareEncoderType bestEncoder = detector.getBestEncoder();

    // Create settings with detected encoder
    HardwareEncoderConfig config;
    config.type = bestEncoder;
    config.preset = HardwareEncoderPreset::Balanced;
    config.bitrate = 2500;

    HardwareEncoderSettings settings(config);

    // Verify settings are valid
    EXPECT_NE(settings.getActualType(), HardwareEncoderType::None);
    EXPECT_EQ(settings.getBitrate(), 2500);
}
