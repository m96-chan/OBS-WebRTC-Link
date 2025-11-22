/**
 * @file webrtc-source.hpp
 * @brief WebRTC Source implementation for receiving streams
 */

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <cstdint>

namespace obswebrtc {
namespace source {

/**
 * @brief Video codec types
 */
enum class VideoCodec {
    H264,
    VP8,
    VP9,
    AV1
};

/**
 * @brief Audio codec types
 */
enum class AudioCodec {
    Opus,
    PCM
};

/**
 * @brief Connection state
 */
enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed
};

/**
 * @brief Video frame structure
 */
struct VideoFrame {
    std::vector<uint8_t> data;
    uint32_t width;
    uint32_t height;
    uint64_t timestamp;
    bool keyframe;
};

/**
 * @brief Audio frame structure
 */
struct AudioFrame {
    std::vector<uint8_t> data;
    uint32_t sampleRate;
    uint32_t channels;
    uint64_t timestamp;
};

/**
 * @brief WebRTC Source configuration
 */
struct WebRTCSourceConfig {
    std::string serverUrl;
    VideoCodec videoCodec;
    AudioCodec audioCodec;
    std::function<void(const VideoFrame&)> videoCallback;
    std::function<void(const AudioFrame&)> audioCallback;
    std::function<void(const std::string&)> errorCallback;
    std::function<void(ConnectionState)> stateCallback;
};

/**
 * @brief WebRTC Source class for receiving streams
 */
class WebRTCSource {
public:
    /**
     * @brief Construct a new WebRTCSource
     * @param config Source configuration
     */
    explicit WebRTCSource(const WebRTCSourceConfig& config);

    /**
     * @brief Destroy the WebRTCSource
     */
    ~WebRTCSource();

    /**
     * @brief Start receiving stream
     * @return true if started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop receiving stream
     */
    void stop();

    /**
     * @brief Check if source is active
     * @return true if active, false otherwise
     */
    bool isActive() const;

    /**
     * @brief Get current connection state
     * @return Current connection state
     */
    ConnectionState getConnectionState() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace source
} // namespace obswebrtc
