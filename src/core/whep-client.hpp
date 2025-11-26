/**
 * @file whep-client.hpp
 * @brief WHEP (WebRTC-HTTP Egress Protocol) Client for receiving media
 *
 * This class implements the WHEP protocol (draft-murillo-whep) for
 * receiving media from SFU servers like LiveKit.
 * It handles HTTP-based signaling with Offer/Answer exchange and ICE candidate trickle.
 */

#pragma once

#include "peer-connection.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace obswebrtc {
namespace core {

/**
 * @brief HTTP Request structure (reuse from WHIP if needed)
 */
struct HTTPRequest;
struct HTTPResponse;

/**
 * @brief Callback function types for WHEP events
 */
using WHEPConnectedCallback = std::function<void()>;
using WHEPDisconnectedCallback = std::function<void()>;
using WHEPErrorCallback = std::function<void(const std::string& error)>;
using WHEPIceCandidateCallback =
    std::function<void(const std::string& candidate, const std::string& mid)>;

/**
 * @brief Configuration for WHEPClient
 */
struct WHEPConfig {
    std::string url;          // WHEP server URL (e.g., https://sfu.livekit.cloud/whep)
    std::string bearerToken;  // Optional bearer token for authentication

    // Event callbacks
    WHEPConnectedCallback onConnected;
    WHEPDisconnectedCallback onDisconnected;
    WHEPErrorCallback onError;
    WHEPIceCandidateCallback onIceCandidate;

    // Media frame callbacks (optional - for receiving media tracks)
    VideoFrameCallback videoFrameCallback;
    AudioFrameCallback audioFrameCallback;

    // ICE server configuration (optional - for WebRTC connection)
    std::vector<std::string> iceServers;
};

/**
 * @brief WHEP Client
 *
 * This class implements the WHEP protocol for WebRTC media egress (receiving):
 * - HTTP POST with SDP Offer to establish connection
 * - Receive SDP Answer from server
 * - HTTP PATCH for ICE candidate trickle
 * - HTTP DELETE to terminate session
 * - Bearer token authentication support
 *
 * Example usage:
 * @code
 * WHEPConfig config;
 * config.url = "https://sfu.livekit.cloud/whep";
 * config.bearerToken = "my-auth-token";
 * config.onConnected = []() {
 *     // Connection established
 * };
 *
 * auto client = std::make_unique<WHEPClient>(config);
 * std::string answer = client->sendOffer(myOfferSdp);
 * client->sendIceCandidate(candidate, mid);
 * @endcode
 */
class WHEPClient {
public:
    /**
     * @brief Construct a new WHEPClient
     * @param config Configuration for the WHEP client
     * @throws std::invalid_argument if config is invalid
     */
    explicit WHEPClient(const WHEPConfig& config);

    /**
     * @brief Destructor - closes connection and cleans up resources
     */
    ~WHEPClient();

    // Delete copy constructor and assignment operator (non-copyable)
    WHEPClient(const WHEPClient&) = delete;
    WHEPClient& operator=(const WHEPClient&) = delete;

    // Allow move semantics
    WHEPClient(WHEPClient&&) noexcept = default;
    WHEPClient& operator=(WHEPClient&&) noexcept = default;

    /**
     * @brief Send SDP offer to WHEP server and receive answer
     * @param sdp SDP offer string
     * @return SDP answer string from server
     * @throws std::invalid_argument if SDP is empty
     * @throws std::runtime_error if HTTP request fails
     */
    std::string sendOffer(const std::string& sdp);

    /**
     * @brief Send ICE candidate to WHEP server via PATCH
     * @param candidate ICE candidate string
     * @param mid Media stream identification tag
     * @throws std::runtime_error if not connected or send fails
     */
    void sendIceCandidate(const std::string& candidate, const std::string& mid);

    /**
     * @brief Disconnect from WHEP server
     * Sends HTTP DELETE to resource URL
     */
    void disconnect();

    /**
     * @brief Check if connected to WHEP server
     * @return true if connected
     */
    bool isConnected() const;

    /**
     * @brief Check if internal PeerConnection is created
     * @return true if PeerConnection exists (frame callbacks are set)
     */
    bool hasPeerConnection() const;

    /**
     * @brief Connect to WHEP server and establish WebRTC connection
     *
     * This initiates the full WHEP connection flow:
     * 1. Create internal PeerConnection (if frame callbacks are set)
     * 2. Generate SDP offer
     * 3. Send offer to WHEP server
     * 4. Receive and apply SDP answer
     * 5. Exchange ICE candidates
     *
     * @throws std::runtime_error if connection fails
     */
    void connect();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace core
}  // namespace obswebrtc
