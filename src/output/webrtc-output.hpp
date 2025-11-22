/**
 * @file webrtc-output.hpp
 * @brief WebRTC Output implementation for OBS Studio
 *
 * This class provides WebRTC output functionality for OBS Studio,
 * allowing Program output to be sent via WebRTC using WHIP protocol.
 */

#pragma once

#include "core/whip-client.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace obswebrtc {
namespace output {

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
    AAC
};

/**
 * @brief Packet type
 */
enum class PacketType {
    Video,
    Audio
};

/**
 * @brief Encoded packet data
 */
struct EncodedPacket {
    PacketType type;
    std::vector<uint8_t> data;
    int64_t timestamp;
    bool keyframe;
};

/**
 * @brief Error callback
 */
using ErrorCallback = std::function<void(const std::string& error)>;

/**
 * @brief State change callback
 */
using StateCallback = std::function<void(bool active)>;

/**
 * @brief Configuration for WebRTC Output
 */
struct WebRTCOutputConfig {
    std::string serverUrl;
    VideoCodec videoCodec = VideoCodec::H264;
    AudioCodec audioCodec = AudioCodec::Opus;
    int videoBitrate = 2500;  // kbps
    int audioBitrate = 128;   // kbps
    ErrorCallback errorCallback;
    StateCallback stateCallback;

    // Reconnection settings
    bool enableAutoReconnect = true;
    int maxReconnectRetries = 5;
    int reconnectInitialDelayMs = 1000;
    int reconnectMaxDelayMs = 30000;
};

/**
 * @brief WebRTC Output class
 *
 * This class implements WebRTC output functionality for OBS Studio.
 * It receives encoded video and audio packets from OBS and sends them
 * via WebRTC using the WHIP protocol.
 *
 * Features:
 * - H.264/VP8/VP9/AV1 video codec support
 * - Opus/AAC audio codec support
 * - Hardware encoder support (NVENC/AMF/QuickSync)
 * - Bitrate and framerate configuration
 * - Connection state monitoring
 * - Error handling and logging
 *
 * Example usage:
 * @code
 * WebRTCOutputConfig config;
 * config.serverUrl = "http://localhost:8080/whip";
 * config.videoCodec = VideoCodec::H264;
 * config.audioCodec = AudioCodec::Opus;
 * config.errorCallback = [](const std::string& error) {
 *     std::cerr << "Error: " << error << std::endl;
 * };
 *
 * WebRTCOutput output(config);
 * output.start();
 *
 * // Send encoded packets
 * EncodedPacket packet;
 * packet.type = PacketType::Video;
 * packet.data = encodedData;
 * packet.timestamp = pts;
 * packet.keyframe = isKeyframe;
 * output.sendPacket(packet);
 *
 * output.stop();
 * @endcode
 */
class WebRTCOutput {
public:
    /**
     * @brief Construct a new WebRTC Output
     * @param config Configuration for the output
     * @throws std::runtime_error if initialization fails
     */
    explicit WebRTCOutput(const WebRTCOutputConfig& config);

    /**
     * @brief Destructor - stops output and cleans up resources
     */
    ~WebRTCOutput();

    // Delete copy constructor and assignment operator (non-copyable)
    WebRTCOutput(const WebRTCOutput&) = delete;
    WebRTCOutput& operator=(const WebRTCOutput&) = delete;

    // Allow move semantics
    WebRTCOutput(WebRTCOutput&&) noexcept = default;
    WebRTCOutput& operator=(WebRTCOutput&&) noexcept = default;

    /**
     * @brief Start the WebRTC output
     *
     * This initiates the WebRTC connection using WHIP protocol.
     * The output will be ready to send packets once the connection
     * is established.
     *
     * @return true if started successfully, false otherwise
     */
    bool start();

    /**
     * @brief Stop the WebRTC output
     *
     * This stops the WebRTC connection and releases resources.
     * After calling stop(), no more packets can be sent.
     */
    void stop();

    /**
     * @brief Check if output is active
     * @return true if output is active and ready to send packets
     */
    bool isActive() const;

    /**
     * @brief Send an encoded packet
     * @param packet Encoded video or audio packet
     * @throws std::runtime_error if output is not active
     */
    void sendPacket(const EncodedPacket& packet);

    /**
     * @brief Get video bitrate
     * @return Video bitrate in kbps
     */
    int getVideoBitrate() const;

    /**
     * @brief Get audio bitrate
     * @return Audio bitrate in kbps
     */
    int getAudioBitrate() const;

    /**
     * @brief Set video bitrate
     * @param bitrate Video bitrate in kbps
     */
    void setVideoBitrate(int bitrate);

    /**
     * @brief Set audio bitrate
     * @param bitrate Audio bitrate in kbps
     */
    void setAudioBitrate(int bitrate);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace output
} // namespace obswebrtc
