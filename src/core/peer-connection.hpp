/**
 * @file peer-connection.hpp
 * @brief WebRTC PeerConnection wrapper using libdatachannel
 *
 * This class provides a clean C++ interface for WebRTC peer connections,
 * wrapping libdatachannel's C++ API. It is designed to be OBS-independent
 * and thread-safe for use in both OBS Output and Source implementations.
 */

#pragma once

#include <rtc/rtc.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace obswebrtc {
namespace core {

/**
 * @brief Log level for logging callbacks
 */
enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

/**
 * @brief ICE connection state
 */
enum class ConnectionState {
    New,
    Checking,
    Connected,
    Completed,
    Failed,
    Disconnected,
    Closed
};

/**
 * @brief SDP type (Offer or Answer)
 */
enum class SdpType {
    Offer,
    Answer
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
 * @brief Callback function types
 */
using LogCallback = std::function<void(LogLevel level, const std::string& message)>;
using StateChangeCallback = std::function<void(ConnectionState state)>;
using IceCandidateCallback =
    std::function<void(const std::string& candidate, const std::string& mid)>;
using LocalDescriptionCallback = std::function<void(SdpType type, const std::string& sdp)>;
using VideoFrameCallback = std::function<void(const VideoFrame& frame)>;
using AudioFrameCallback = std::function<void(const AudioFrame& frame)>;

/**
 * @brief Configuration for PeerConnection
 */
struct PeerConnectionConfig {
    std::vector<std::string> iceServers;  // STUN/TURN server URLs
    LogCallback logCallback;
    StateChangeCallback stateCallback;
    IceCandidateCallback iceCandidateCallback;
    LocalDescriptionCallback localDescriptionCallback;
    VideoFrameCallback videoFrameCallback;
    AudioFrameCallback audioFrameCallback;
};

/**
 * @brief WebRTC PeerConnection wrapper
 *
 * This class wraps libdatachannel's PeerConnection and provides:
 * - Offer/Answer generation and handling
 * - ICE candidate collection and exchange
 * - Connection state monitoring
 * - Thread-safe operations
 * - OBS-independent design
 *
 * Example usage:
 * @code
 * PeerConnectionConfig config;
 * config.iceServers = {"stun:stun.l.google.com:19302"};
 * config.logCallback = [](LogLevel level, const std::string& msg) {
 *     std::cout << msg << std::endl;
 * };
 *
 * auto pc = std::make_unique<PeerConnection>(config);
 * pc->createOffer();
 * @endcode
 */
class PeerConnection {
public:
    /**
     * @brief Construct a new PeerConnection
     * @param config Configuration for the peer connection
     * @throws std::runtime_error if initialization fails
     */
    explicit PeerConnection(const PeerConnectionConfig& config);

    /**
     * @brief Destructor - closes connection and cleans up resources
     */
    ~PeerConnection();

    // Delete copy constructor and assignment operator (non-copyable)
    PeerConnection(const PeerConnection&) = delete;
    PeerConnection& operator=(const PeerConnection&) = delete;

    // Allow move semantics
    PeerConnection(PeerConnection&&) noexcept = default;
    PeerConnection& operator=(PeerConnection&&) noexcept = default;

    /**
     * @brief Create an SDP offer
     *
     * This initiates the connection process. The generated offer will be
     * delivered via the localDescriptionCallback.
     *
     * @throws std::runtime_error if offer creation fails
     */
    void createOffer();

    /**
     * @brief Create an SDP answer
     *
     * This responds to a received offer. The generated answer will be
     * delivered via the localDescriptionCallback.
     *
     * @throws std::runtime_error if answer creation fails
     */
    void createAnswer();

    /**
     * @brief Set remote description (offer or answer)
     * @param type SDP type (Offer or Answer)
     * @param sdp SDP content
     * @throws std::runtime_error if setting remote description fails
     */
    void setRemoteDescription(SdpType type, const std::string& sdp);

    /**
     * @brief Add a remote ICE candidate
     * @param candidate ICE candidate string
     * @param mid Media stream identification tag
     * @throws std::runtime_error if adding candidate fails
     */
    void addIceCandidate(const std::string& candidate, const std::string& mid);

    /**
     * @brief Get current connection state
     * @return Current connection state
     */
    ConnectionState getState() const;

    /**
     * @brief Check if connection is established
     * @return true if connected or completed
     */
    bool isConnected() const;

    /**
     * @brief Close the peer connection
     *
     * This gracefully closes the connection and releases resources.
     * After calling close(), the PeerConnection should not be used.
     */
    void close();

    /**
     * @brief Get local description (for debugging)
     * @return Local SDP string, or empty if not set
     */
    std::string getLocalDescription() const;

    /**
     * @brief Get remote description (for debugging)
     * @return Remote SDP string, or empty if not set
     */
    std::string getRemoteDescription() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace core
}  // namespace obswebrtc
