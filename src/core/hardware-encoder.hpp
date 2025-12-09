/**
 * @file hardware-encoder.hpp
 * @brief Hardware encoder configuration and detection for WebRTC
 *
 * This module provides:
 * - Hardware encoder detection (NVENC, AMF, QuickSync)
 * - Encoder configuration with presets
 * - Automatic fallback to software encoder
 * - Encoder-specific optimization settings
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace obswebrtc {
namespace core {

/**
 * @brief Hardware encoder types
 */
enum class HardwareEncoderType {
    None = 0,       ///< No encoder selected
    NVENC = 1,      ///< NVIDIA NVENC
    AMF = 2,        ///< AMD Advanced Media Framework
    QuickSync = 3,  ///< Intel Quick Sync Video
    Software = 4    ///< Software encoder (x264/x265)
};

/**
 * @brief Encoder presets for different use cases
 */
enum class HardwareEncoderPreset {
    Quality = 0,    ///< High quality, higher latency
    Balanced = 1,   ///< Balance between quality and speed
    Speed = 2,      ///< Fast encoding, lower quality
    LowLatency = 3  ///< Minimum latency for real-time streaming
};

/**
 * @brief Hardware encoder configuration
 */
struct HardwareEncoderConfig {
    HardwareEncoderType type = HardwareEncoderType::None;
    HardwareEncoderPreset preset = HardwareEncoderPreset::Balanced;

    // Bitrate settings (kbps)
    int bitrate = 2500;
    int maxBitrate = 5000;

    // Keyframe settings
    int keyframeInterval = 2;  // seconds

    // Profile (baseline, main, high)
    std::string profile = "high";

    // B-frame settings
    bool enableBFrames = true;
    int bFrameCount = 2;

    // Lookahead settings
    bool enableLookahead = true;
    int lookaheadFrames = 4;

    // Fallback behavior
    bool enableFallback = true;  ///< Fall back to software if hardware unavailable
};

/**
 * @brief Detects available hardware encoders on the system
 *
 * This class provides methods to detect and enumerate available
 * hardware encoders, allowing the application to select the best
 * available option.
 */
class HardwareEncoderDetector {
public:
    /**
     * @brief Construct a new Hardware Encoder Detector
     */
    HardwareEncoderDetector();

    /**
     * @brief Destructor
     */
    ~HardwareEncoderDetector();

    /**
     * @brief Get list of available hardware encoders
     * @return Vector of available encoder types
     */
    std::vector<HardwareEncoderType> getAvailableEncoders() const;

    /**
     * @brief Check if a specific encoder is available
     * @param type Encoder type to check
     * @return true if available, false otherwise
     */
    bool isAvailable(HardwareEncoderType type) const;

    /**
     * @brief Get the best available encoder
     *
     * Priority: NVENC > QuickSync > AMF > Software
     *
     * @return Best available encoder type
     */
    HardwareEncoderType getBestEncoder() const;

    /**
     * @brief Convert encoder type to string
     * @param type Encoder type
     * @return String representation
     */
    static std::string encoderTypeToString(HardwareEncoderType type);

    /**
     * @brief Convert string to encoder type
     * @param str String representation
     * @return Encoder type (None if invalid)
     */
    static HardwareEncoderType encoderTypeFromString(const std::string& str);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Manages hardware encoder settings and provides encoder-specific configurations
 *
 * This class handles:
 * - Configuration validation
 * - Preset-based optimal settings
 * - Encoder-specific parameter generation
 * - Fallback handling
 */
class HardwareEncoderSettings {
public:
    /**
     * @brief Construct settings with given configuration
     * @param config Encoder configuration
     */
    explicit HardwareEncoderSettings(const HardwareEncoderConfig& config);

    /**
     * @brief Destructor
     */
    ~HardwareEncoderSettings();

    /**
     * @brief Get configured encoder type
     * @return Configured encoder type
     */
    HardwareEncoderType getType() const;

    /**
     * @brief Get actual encoder type after fallback resolution
     * @return Actual encoder type that will be used
     */
    HardwareEncoderType getActualType() const;

    /**
     * @brief Get current bitrate
     * @return Bitrate in kbps
     */
    int getBitrate() const;

    /**
     * @brief Set bitrate
     * @param bitrate Bitrate in kbps
     * @throws std::invalid_argument if bitrate <= 0
     */
    void setBitrate(int bitrate);

    /**
     * @brief Get optimal configuration for a preset
     * @param preset Encoder preset
     * @return Optimized configuration
     */
    HardwareEncoderConfig getOptimalConfig(HardwareEncoderPreset preset) const;

    /**
     * @brief Get NVENC-specific configuration
     * @return Map of NVENC parameters
     */
    std::map<std::string, std::string> getNVENCConfig() const;

    /**
     * @brief Get AMF-specific configuration
     * @return Map of AMF parameters
     */
    std::map<std::string, std::string> getAMFConfig() const;

    /**
     * @brief Get QuickSync-specific configuration
     * @return Map of QuickSync parameters
     */
    std::map<std::string, std::string> getQuickSyncConfig() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace core
}  // namespace obswebrtc
