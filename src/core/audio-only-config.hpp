// Copyright (c) 2025 OBS-WebRTC-Link Project
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

/**
 * @brief Audio quality presets for audio-only mode
 */
enum class AudioQuality {
    Low,     ///< 32 kbps - minimum bandwidth usage
    Medium,  ///< 48 kbps - balanced quality and bandwidth
    High     ///< 64 kbps - highest quality
};

/**
 * @brief Configuration for audio-only mode
 *
 * Manages settings for audio-only WebRTC connections including:
 * - Audio-only mode enable/disable
 * - Audio quality presets
 * - Audio processing options (echo cancellation, noise suppression)
 * - Codec selection
 */
class AudioOnlyConfig {
public:
    /**
     * @brief Construct default audio configuration
     *
     * Default settings:
     * - Audio-only mode: disabled
     * - Quality preset: Medium (48 kbps)
     * - Codec: Opus
     * - Echo cancellation: enabled
     * - Noise suppression: enabled
     */
    AudioOnlyConfig();

    /**
     * @brief Enable or disable audio-only mode
     * @param enabled true to enable audio-only mode, false to disable
     */
    void setAudioOnly(bool enabled);

    /**
     * @brief Check if audio-only mode is enabled
     * @return true if audio-only mode is enabled, false otherwise
     */
    bool isAudioOnly() const;

    /**
     * @brief Set audio quality preset
     * @param quality Quality preset to use
     */
    void setAudioQualityPreset(AudioQuality quality);

    /**
     * @brief Get current audio quality preset
     * @return Current audio quality preset
     */
    AudioQuality getAudioQualityPreset() const;

    /**
     * @brief Get audio bitrate in kbps for current quality preset
     * @return Audio bitrate in kbps
     */
    int getAudioBitrate() const;

    /**
     * @brief Set custom audio bitrate
     * @param bitrateKbps Audio bitrate in kbps (must be between 16 and 128)
     * @throws std::invalid_argument if bitrate is out of range
     */
    void setCustomAudioBitrate(int bitrateKbps);

    /**
     * @brief Get audio codec name
     * @return Audio codec name (e.g., "opus")
     */
    std::string getAudioCodec() const;

    /**
     * @brief Set audio codec
     * @param codec Codec name (e.g., "opus", "pcmu", "pcma")
     */
    void setAudioCodec(const std::string& codec);

    /**
     * @brief Enable or disable echo cancellation
     * @param enabled true to enable echo cancellation, false to disable
     */
    void setEchoCancellation(bool enabled);

    /**
     * @brief Check if echo cancellation is enabled
     * @return true if echo cancellation is enabled, false otherwise
     */
    bool isEchoCancellationEnabled() const;

    /**
     * @brief Enable or disable noise suppression
     * @param enabled true to enable noise suppression, false to disable
     */
    void setNoiseSuppression(bool enabled);

    /**
     * @brief Check if noise suppression is enabled
     * @return true if noise suppression is enabled, false otherwise
     */
    bool isNoiseSuppressionEnabled() const;

    /**
     * @brief Enable or disable automatic gain control
     * @param enabled true to enable AGC, false to disable
     */
    void setAutomaticGainControl(bool enabled);

    /**
     * @brief Check if automatic gain control is enabled
     * @return true if AGC is enabled, false otherwise
     */
    bool isAutomaticGainControlEnabled() const;

private:
    bool audioOnly_;
    AudioQuality qualityPreset_;
    int customBitrateKbps_;
    bool useCustomBitrate_;
    std::string audioCodec_;
    bool echoCancellation_;
    bool noiseSuppression_;
    bool automaticGainControl_;

    /**
     * @brief Get default bitrate for quality preset
     * @param quality Quality preset
     * @return Bitrate in kbps
     */
    static int getDefaultBitrateForQuality(AudioQuality quality);
};
