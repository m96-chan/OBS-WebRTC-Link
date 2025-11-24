// Copyright (c) 2025 OBS-WebRTC-Link Project
// SPDX-License-Identifier: MIT

#include "audio-only-config.hpp"
#include <stdexcept>

AudioOnlyConfig::AudioOnlyConfig()
    : audioOnly_(false),
      qualityPreset_(AudioQuality::Medium),
      customBitrateKbps_(0),
      useCustomBitrate_(false),
      audioCodec_("opus"),
      echoCancellation_(true),
      noiseSuppression_(true),
      automaticGainControl_(false)
{
}

void AudioOnlyConfig::setAudioOnly(bool enabled)
{
    audioOnly_ = enabled;
}

bool AudioOnlyConfig::isAudioOnly() const
{
    return audioOnly_;
}

void AudioOnlyConfig::setAudioQualityPreset(AudioQuality quality)
{
    qualityPreset_ = quality;
    useCustomBitrate_ = false;
}

AudioQuality AudioOnlyConfig::getAudioQualityPreset() const
{
    return qualityPreset_;
}

int AudioOnlyConfig::getAudioBitrate() const
{
    if (useCustomBitrate_) {
        return customBitrateKbps_;
    }
    return getDefaultBitrateForQuality(qualityPreset_);
}

void AudioOnlyConfig::setCustomAudioBitrate(int bitrateKbps)
{
    if (bitrateKbps < 16 || bitrateKbps > 128) {
        throw std::invalid_argument("Audio bitrate must be between 16 and 128 kbps");
    }
    customBitrateKbps_ = bitrateKbps;
    useCustomBitrate_ = true;
}

std::string AudioOnlyConfig::getAudioCodec() const
{
    return audioCodec_;
}

void AudioOnlyConfig::setAudioCodec(const std::string& codec)
{
    audioCodec_ = codec;
}

void AudioOnlyConfig::setEchoCancellation(bool enabled)
{
    echoCancellation_ = enabled;
}

bool AudioOnlyConfig::isEchoCancellationEnabled() const
{
    return echoCancellation_;
}

void AudioOnlyConfig::setNoiseSuppression(bool enabled)
{
    noiseSuppression_ = enabled;
}

bool AudioOnlyConfig::isNoiseSuppressionEnabled() const
{
    return noiseSuppression_;
}

void AudioOnlyConfig::setAutomaticGainControl(bool enabled)
{
    automaticGainControl_ = enabled;
}

bool AudioOnlyConfig::isAutomaticGainControlEnabled() const
{
    return automaticGainControl_;
}

int AudioOnlyConfig::getDefaultBitrateForQuality(AudioQuality quality)
{
    switch (quality) {
    case AudioQuality::Low:
        return 32;
    case AudioQuality::Medium:
        return 48;
    case AudioQuality::High:
        return 64;
    default:
        return 48;
    }
}
