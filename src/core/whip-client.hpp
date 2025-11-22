/**
 * @file whip-client.hpp
 * @brief WHIP (WebRTC-HTTP Ingestion Protocol) Client for sending media
 *
 * This class implements the WHIP protocol (draft-ietf-wish-whip) for
 * ingesting media into SFU servers like LiveKit.
 * It handles HTTP-based signaling with Offer/Answer exchange and ICE candidate trickle.
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

namespace obswebrtc {
namespace core {

/**
 * @brief HTTP Request structure
 */
struct HTTPRequest {
    std::map<std::string, std::string> headers;
    std::string body;
    std::string contentType;
};

/**
 * @brief HTTP Response structure
 */
struct HTTPResponse {
    int statusCode;
    std::map<std::string, std::string> headers;
    std::string body;
};

/**
 * @brief Callback function types for WHIP events
 */
using WHIPConnectedCallback = std::function<void()>;
using WHIPDisconnectedCallback = std::function<void()>;
using WHIPErrorCallback = std::function<void(const std::string& error)>;
using WHIPIceCandidateCallback =
    std::function<void(const std::string& candidate, const std::string& mid)>;

/**
 * @brief Configuration for WHIPClient
 */
struct WHIPConfig {
    std::string url;          // WHIP server URL (e.g., https://sfu.livekit.cloud/whip)
    std::string bearerToken;  // Optional bearer token for authentication

    // Event callbacks
    WHIPConnectedCallback onConnected;
    WHIPDisconnectedCallback onDisconnected;
    WHIPErrorCallback onError;
    WHIPIceCandidateCallback onIceCandidate;
};

/**
 * @brief WHIP Client
 *
 * This class implements the WHIP protocol for WebRTC media ingestion:
 * - HTTP POST with SDP Offer to establish connection
 * - Receive SDP Answer from server
 * - HTTP PATCH for ICE candidate trickle
 * - HTTP DELETE to terminate session
 * - Bearer token authentication support
 *
 * Example usage:
 * @code
 * WHIPConfig config;
 * config.url = "https://sfu.livekit.cloud/whip";
 * config.bearerToken = "my-auth-token";
 * config.onConnected = []() {
 *     // Connection established
 * };
 *
 * auto client = std::make_unique<WHIPClient>(config);
 * std::string answer = client->sendOffer(myOfferSdp);
 * client->sendIceCandidate(candidate, mid);
 * @endcode
 */
class WHIPClient {
public:
    /**
     * @brief Construct a new WHIPClient
     * @param config Configuration for the WHIP client
     * @throws std::invalid_argument if config is invalid
     */
    explicit WHIPClient(const WHIPConfig& config);

    /**
     * @brief Destructor - closes connection and cleans up resources
     */
    ~WHIPClient();

    // Delete copy constructor and assignment operator (non-copyable)
    WHIPClient(const WHIPClient&) = delete;
    WHIPClient& operator=(const WHIPClient&) = delete;

    // Allow move semantics
    WHIPClient(WHIPClient&&) noexcept = default;
    WHIPClient& operator=(WHIPClient&&) noexcept = default;

    /**
     * @brief Send SDP offer to WHIP server and receive answer
     * @param sdp SDP offer string
     * @return SDP answer string from server
     * @throws std::invalid_argument if SDP is empty
     * @throws std::runtime_error if HTTP request fails
     */
    std::string sendOffer(const std::string& sdp);

    /**
     * @brief Send ICE candidate to WHIP server via PATCH
     * @param candidate ICE candidate string
     * @param mid Media stream identification tag
     * @throws std::runtime_error if not connected or send fails
     */
    void sendIceCandidate(const std::string& candidate, const std::string& mid);

    /**
     * @brief Disconnect from WHIP server
     * Sends HTTP DELETE to resource URL
     */
    void disconnect();

    /**
     * @brief Check if connected to WHIP server
     * @return true if connected
     */
    bool isConnected() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace core
}  // namespace obswebrtc
