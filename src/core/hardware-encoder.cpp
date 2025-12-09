/**
 * @file hardware-encoder.cpp
 * @brief Hardware encoder configuration and detection implementation
 */

#include "hardware-encoder.hpp"

#include <algorithm>
#include <mutex>
#include <stdexcept>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

namespace obswebrtc {
namespace core {

// =============================================================================
// HardwareEncoderDetector Implementation
// =============================================================================

class HardwareEncoderDetector::Impl {
public:
    Impl() {
        detectAvailableEncoders();
    }

    std::vector<HardwareEncoderType> getAvailableEncoders() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return availableEncoders_;
    }

    bool isAvailable(HardwareEncoderType type) const {
        std::lock_guard<std::mutex> lock(mutex_);
        if (type == HardwareEncoderType::None) {
            return false;
        }
        return std::find(availableEncoders_.begin(), availableEncoders_.end(), type) !=
               availableEncoders_.end();
    }

    HardwareEncoderType getBestEncoder() const {
        std::lock_guard<std::mutex> lock(mutex_);

        // Priority order: NVENC > QuickSync > AMF > Software
        const std::vector<HardwareEncoderType> priority = {
            HardwareEncoderType::NVENC,
            HardwareEncoderType::QuickSync,
            HardwareEncoderType::AMF,
            HardwareEncoderType::Software
        };

        for (const auto& type : priority) {
            if (std::find(availableEncoders_.begin(), availableEncoders_.end(), type) !=
                availableEncoders_.end()) {
                return type;
            }
        }

        return HardwareEncoderType::Software;
    }

private:
    void detectAvailableEncoders() {
        // Software encoder is always available
        availableEncoders_.push_back(HardwareEncoderType::Software);

        // Detect hardware encoders
        if (detectNVENC()) {
            availableEncoders_.push_back(HardwareEncoderType::NVENC);
        }
        if (detectAMF()) {
            availableEncoders_.push_back(HardwareEncoderType::AMF);
        }
        if (detectQuickSync()) {
            availableEncoders_.push_back(HardwareEncoderType::QuickSync);
        }
    }

    bool detectNVENC() {
#ifdef _WIN32
        // Check for NVIDIA GPU by looking for nvEncodeAPI64.dll
        HMODULE nvenc = LoadLibraryA("nvEncodeAPI64.dll");
        if (nvenc) {
            FreeLibrary(nvenc);
            return true;
        }
        // Also check 32-bit version
        nvenc = LoadLibraryA("nvEncodeAPI.dll");
        if (nvenc) {
            FreeLibrary(nvenc);
            return true;
        }
#elif defined(__linux__)
        // On Linux, check for libnvidia-encode.so
        void* handle = dlopen("libnvidia-encode.so.1", RTLD_LAZY);
        if (handle) {
            dlclose(handle);
            return true;
        }
#endif
        return false;
    }

    bool detectAMF() {
#ifdef _WIN32
        // Check for AMD AMF by looking for amfrt64.dll
        HMODULE amf = LoadLibraryA("amfrt64.dll");
        if (amf) {
            FreeLibrary(amf);
            return true;
        }
        // Also check 32-bit version
        amf = LoadLibraryA("amfrt32.dll");
        if (amf) {
            FreeLibrary(amf);
            return true;
        }
#endif
        return false;
    }

    bool detectQuickSync() {
#ifdef _WIN32
        // Check for Intel Quick Sync by looking for libmfx DLL
        HMODULE mfx = LoadLibraryA("libmfx-gen.dll");
        if (mfx) {
            FreeLibrary(mfx);
            return true;
        }
        // Also check legacy MFX library
        mfx = LoadLibraryA("libmfxhw64.dll");
        if (mfx) {
            FreeLibrary(mfx);
            return true;
        }
#endif
        return false;
    }

    std::vector<HardwareEncoderType> availableEncoders_;
    mutable std::mutex mutex_;
};

HardwareEncoderDetector::HardwareEncoderDetector()
    : impl_(std::make_unique<Impl>()) {}

HardwareEncoderDetector::~HardwareEncoderDetector() = default;

std::vector<HardwareEncoderType> HardwareEncoderDetector::getAvailableEncoders() const {
    return impl_->getAvailableEncoders();
}

bool HardwareEncoderDetector::isAvailable(HardwareEncoderType type) const {
    return impl_->isAvailable(type);
}

HardwareEncoderType HardwareEncoderDetector::getBestEncoder() const {
    return impl_->getBestEncoder();
}

std::string HardwareEncoderDetector::encoderTypeToString(HardwareEncoderType type) {
    switch (type) {
        case HardwareEncoderType::None:
            return "None";
        case HardwareEncoderType::NVENC:
            return "NVENC";
        case HardwareEncoderType::AMF:
            return "AMF";
        case HardwareEncoderType::QuickSync:
            return "QuickSync";
        case HardwareEncoderType::Software:
            return "Software";
        default:
            return "None";
    }
}

HardwareEncoderType HardwareEncoderDetector::encoderTypeFromString(const std::string& str) {
    if (str == "NVENC") return HardwareEncoderType::NVENC;
    if (str == "AMF") return HardwareEncoderType::AMF;
    if (str == "QuickSync") return HardwareEncoderType::QuickSync;
    if (str == "Software") return HardwareEncoderType::Software;
    return HardwareEncoderType::None;
}

// =============================================================================
// HardwareEncoderSettings Implementation
// =============================================================================

class HardwareEncoderSettings::Impl {
public:
    explicit Impl(const HardwareEncoderConfig& config)
        : config_(config), detector_() {
        resolveActualEncoder();
    }

    HardwareEncoderType getType() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return config_.type;
    }

    HardwareEncoderType getActualType() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return actualType_;
    }

    int getBitrate() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return config_.bitrate;
    }

    void setBitrate(int bitrate) {
        if (bitrate <= 0) {
            throw std::invalid_argument("Bitrate must be positive");
        }
        std::lock_guard<std::mutex> lock(mutex_);
        config_.bitrate = bitrate;
    }

    HardwareEncoderConfig getOptimalConfig(HardwareEncoderPreset preset) const {
        std::lock_guard<std::mutex> lock(mutex_);
        HardwareEncoderConfig optimal = config_;
        optimal.preset = preset;

        switch (preset) {
            case HardwareEncoderPreset::Quality:
                optimal.bitrate = std::max(config_.bitrate, 3000);
                optimal.enableBFrames = true;
                optimal.bFrameCount = 3;
                optimal.enableLookahead = true;
                optimal.lookaheadFrames = 8;
                optimal.profile = "high";
                break;

            case HardwareEncoderPreset::Balanced:
                optimal.bitrate = config_.bitrate;
                optimal.enableBFrames = true;
                optimal.bFrameCount = 2;
                optimal.enableLookahead = true;
                optimal.lookaheadFrames = 4;
                optimal.profile = "high";
                break;

            case HardwareEncoderPreset::Speed:
                optimal.bitrate = config_.bitrate;
                optimal.enableBFrames = false;
                optimal.bFrameCount = 0;
                optimal.enableLookahead = false;
                optimal.lookaheadFrames = 0;
                optimal.profile = "main";
                break;

            case HardwareEncoderPreset::LowLatency:
                optimal.bitrate = config_.bitrate;
                optimal.enableBFrames = false;
                optimal.bFrameCount = 0;
                optimal.enableLookahead = false;
                optimal.lookaheadFrames = 0;
                optimal.keyframeInterval = 1;
                optimal.profile = "baseline";
                break;
        }

        return optimal;
    }

    std::map<std::string, std::string> getNVENCConfig() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::map<std::string, std::string> nvencConfig;

        // Map preset to NVENC preset
        switch (config_.preset) {
            case HardwareEncoderPreset::Quality:
                nvencConfig["preset"] = "p7";  // Highest quality
                nvencConfig["tune"] = "hq";
                break;
            case HardwareEncoderPreset::Balanced:
                nvencConfig["preset"] = "p4";  // Balanced
                nvencConfig["tune"] = "hq";
                break;
            case HardwareEncoderPreset::Speed:
                nvencConfig["preset"] = "p2";  // Fast
                nvencConfig["tune"] = "hp";
                break;
            case HardwareEncoderPreset::LowLatency:
                nvencConfig["preset"] = "p1";  // Fastest
                nvencConfig["tune"] = "ll";
                nvencConfig["zerolatency"] = "1";
                break;
        }

        nvencConfig["bitrate"] = std::to_string(config_.bitrate);
        nvencConfig["maxbitrate"] = std::to_string(config_.maxBitrate);
        nvencConfig["profile"] = config_.profile;
        nvencConfig["bf"] = std::to_string(config_.enableBFrames ? config_.bFrameCount : 0);
        nvencConfig["lookahead"] = config_.enableLookahead ? std::to_string(config_.lookaheadFrames) : "0";
        nvencConfig["rc"] = "cbr";  // Constant bitrate for streaming

        return nvencConfig;
    }

    std::map<std::string, std::string> getAMFConfig() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::map<std::string, std::string> amfConfig;

        // Map preset to AMF preset
        switch (config_.preset) {
            case HardwareEncoderPreset::Quality:
                amfConfig["quality"] = "quality";
                break;
            case HardwareEncoderPreset::Balanced:
                amfConfig["quality"] = "balanced";
                break;
            case HardwareEncoderPreset::Speed:
                amfConfig["quality"] = "speed";
                break;
            case HardwareEncoderPreset::LowLatency:
                amfConfig["quality"] = "speed";
                amfConfig["latency"] = "ultralowlatency";
                break;
        }

        amfConfig["bitrate"] = std::to_string(config_.bitrate);
        amfConfig["maxbitrate"] = std::to_string(config_.maxBitrate);
        amfConfig["profile"] = config_.profile;
        amfConfig["bf"] = std::to_string(config_.enableBFrames ? config_.bFrameCount : 0);
        amfConfig["rc"] = "cbr";

        return amfConfig;
    }

    std::map<std::string, std::string> getQuickSyncConfig() const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::map<std::string, std::string> qsvConfig;

        // Map preset to QSV preset
        switch (config_.preset) {
            case HardwareEncoderPreset::Quality:
                qsvConfig["preset"] = "veryslow";
                break;
            case HardwareEncoderPreset::Balanced:
                qsvConfig["preset"] = "medium";
                break;
            case HardwareEncoderPreset::Speed:
                qsvConfig["preset"] = "veryfast";
                break;
            case HardwareEncoderPreset::LowLatency:
                qsvConfig["preset"] = "veryfast";
                qsvConfig["async_depth"] = "1";
                break;
        }

        qsvConfig["bitrate"] = std::to_string(config_.bitrate);
        qsvConfig["maxbitrate"] = std::to_string(config_.maxBitrate);
        qsvConfig["profile"] = config_.profile;
        qsvConfig["bf"] = std::to_string(config_.enableBFrames ? config_.bFrameCount : 0);
        qsvConfig["look_ahead"] = config_.enableLookahead ? "1" : "0";

        return qsvConfig;
    }

private:
    void resolveActualEncoder() {
        if (config_.type == HardwareEncoderType::None) {
            // Auto-detect best encoder
            actualType_ = detector_.getBestEncoder();
        } else if (config_.enableFallback && !detector_.isAvailable(config_.type)) {
            // Fallback to best available
            actualType_ = detector_.getBestEncoder();
        } else {
            actualType_ = config_.type;
        }
    }

    HardwareEncoderConfig config_;
    HardwareEncoderDetector detector_;
    HardwareEncoderType actualType_;
    mutable std::mutex mutex_;
};

HardwareEncoderSettings::HardwareEncoderSettings(const HardwareEncoderConfig& config)
    : impl_(std::make_unique<Impl>(config)) {}

HardwareEncoderSettings::~HardwareEncoderSettings() = default;

HardwareEncoderType HardwareEncoderSettings::getType() const {
    return impl_->getType();
}

HardwareEncoderType HardwareEncoderSettings::getActualType() const {
    return impl_->getActualType();
}

int HardwareEncoderSettings::getBitrate() const {
    return impl_->getBitrate();
}

void HardwareEncoderSettings::setBitrate(int bitrate) {
    impl_->setBitrate(bitrate);
}

HardwareEncoderConfig HardwareEncoderSettings::getOptimalConfig(HardwareEncoderPreset preset) const {
    return impl_->getOptimalConfig(preset);
}

std::map<std::string, std::string> HardwareEncoderSettings::getNVENCConfig() const {
    return impl_->getNVENCConfig();
}

std::map<std::string, std::string> HardwareEncoderSettings::getAMFConfig() const {
    return impl_->getAMFConfig();
}

std::map<std::string, std::string> HardwareEncoderSettings::getQuickSyncConfig() const {
    return impl_->getQuickSyncConfig();
}

}  // namespace core
}  // namespace obswebrtc
