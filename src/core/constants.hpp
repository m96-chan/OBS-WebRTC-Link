/**
 * @file constants.hpp
 * @brief Shared constants for the OBS-WebRTC-Link core library
 *
 * This header centralizes magic numbers and configuration values
 * used throughout the codebase for better maintainability.
 */

#pragma once

#include <cstdint>

namespace obswebrtc {
namespace core {
namespace constants {

// =============================================================================
// Audio Configuration
// =============================================================================

/** Default audio sample rate in Hz (Opus standard) */
constexpr int kDefaultAudioSampleRate = 48000;

/** Default number of audio channels (stereo) */
constexpr int kDefaultAudioChannels = 2;

/** Minimum audio bitrate in kbps */
constexpr int kMinAudioBitrateKbps = 16;

/** Maximum audio bitrate in kbps */
constexpr int kMaxAudioBitrateKbps = 128;

// =============================================================================
// Network Calculations
// =============================================================================

/** Number of bits per byte */
constexpr int kBitsPerByte = 8;

/** Milliseconds per second */
constexpr int kMsPerSecond = 1000;

/** Bytes per kilobyte (decimal) */
constexpr int kBytesPerKB = 1000;

/** Bytes per megabyte (decimal) */
constexpr int kBytesPerMB = 1000000;

/** Bytes per gigabyte (decimal) */
constexpr int64_t kBytesPerGB = 1000000000;

/** Kilobits per megabit */
constexpr int kKbpsPerMbps = 1000;

// =============================================================================
// Timeouts
// =============================================================================

/** Default timeout for offer generation in seconds */
constexpr int kDefaultOfferTimeoutSec = 5;

/** Default timeout for ICE gathering in milliseconds */
constexpr int kDefaultIceGatheringTimeoutMs = 500;

// =============================================================================
// Session/Connection
// =============================================================================

/** Length of P2P session ID */
constexpr int kSessionIdLength = 8;

// =============================================================================
// Unit Conversion Helpers
// =============================================================================

/**
 * @brief Convert bytes to kilobytes (decimal)
 */
constexpr double bytesToKB(uint64_t bytes) {
    return static_cast<double>(bytes) / kBytesPerKB;
}

/**
 * @brief Convert bytes to megabytes (decimal)
 */
constexpr double bytesToMB(uint64_t bytes) {
    return static_cast<double>(bytes) / kBytesPerMB;
}

/**
 * @brief Convert bytes to gigabytes (decimal)
 */
constexpr double bytesToGB(uint64_t bytes) {
    return static_cast<double>(bytes) / kBytesPerGB;
}

/**
 * @brief Convert kbps to Mbps
 */
constexpr double kbpsToMbps(uint32_t kbps) {
    return static_cast<double>(kbps) / kKbpsPerMbps;
}

}  // namespace constants
}  // namespace core
}  // namespace obswebrtc
