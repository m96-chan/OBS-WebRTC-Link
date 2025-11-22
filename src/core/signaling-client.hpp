/**
 * @file signaling-client.hpp
 * @brief WebRTC Signaling Client for Offer/Answer exchange
 *
 * This class provides a clean abstraction for WebRTC signaling,
 * handling Offer/Answer and ICE candidate exchange.
 * It supports multiple signaling protocols through a pluggable transport layer.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>

namespace obswebrtc {
namespace core {

/**
 * @brief Callback function types for signaling events
 */
using SignalingConnectedCallback = std::function<void()>;
using SignalingDisconnectedCallback = std::function<void()>;
using SignalingErrorCallback = std::function<void(const std::string& error)>;
using SignalingOfferCallback = std::function<void(const std::string& sdp)>;
using SignalingAnswerCallback = std::function<void(const std::string& sdp)>;
using SignalingIceCandidateCallback = std::function<void(const std::string& candidate, const std::string& mid)>;

/**
 * @brief Configuration for SignalingClient
 */
struct SignalingConfig {
    std::string url;  // Signaling server URL (e.g., wss://signaling.example.com)

    // Event callbacks
    SignalingConnectedCallback onConnected;
    SignalingDisconnectedCallback onDisconnected;
    SignalingErrorCallback onError;
    SignalingOfferCallback onOffer;
    SignalingAnswerCallback onAnswer;
    SignalingIceCandidateCallback onIceCandidate;
};

/**
 * @brief Abstract transport layer for signaling
 *
 * This allows different signaling protocols (WebSocket, HTTP, custom)
 * to be plugged into the SignalingClient.
 */
class SignalingTransport {
public:
    virtual ~SignalingTransport() = default;

    /**
     * @brief Connect to the signaling server
     * @param url Server URL
     */
    virtual void connect(const std::string& url) = 0;

    /**
     * @brief Disconnect from the signaling server
     */
    virtual void disconnect() = 0;

    /**
     * @brief Send a message to the signaling server
     * @param message JSON-encoded message
     */
    virtual void sendMessage(const std::string& message) = 0;

    /**
     * @brief Check if transport is connected
     * @return true if connected
     */
    virtual bool isConnected() const = 0;
};

/**
 * @brief WebRTC Signaling Client
 *
 * This class manages the signaling process for WebRTC connections:
 * - Offer/Answer exchange
 * - ICE candidate trickling
 * - Connection state management
 *
 * Example usage:
 * @code
 * SignalingConfig config;
 * config.url = "wss://signaling.example.com";
 * config.onOffer = [](const std::string& sdp) {
 *     // Handle received offer
 * };
 * config.onAnswer = [](const std::string& sdp) {
 *     // Handle received answer
 * };
 *
 * auto client = std::make_unique<SignalingClient>(config);
 * client->connect();
 * client->sendOffer(myOfferSdp);
 * @endcode
 */
class SignalingClient {
public:
    /**
     * @brief Construct a new SignalingClient
     * @param config Configuration for the signaling client
     * @throws std::invalid_argument if config is invalid
     */
    explicit SignalingClient(const SignalingConfig& config);

    /**
     * @brief Construct with custom transport (for testing)
     * @param config Configuration
     * @param transport Custom transport implementation
     */
    SignalingClient(const SignalingConfig& config, std::unique_ptr<SignalingTransport> transport);

    /**
     * @brief Destructor - closes connection and cleans up resources
     */
    ~SignalingClient();

    // Delete copy constructor and assignment operator (non-copyable)
    SignalingClient(const SignalingClient&) = delete;
    SignalingClient& operator=(const SignalingClient&) = delete;

    // Allow move semantics
    SignalingClient(SignalingClient&&) noexcept = default;
    SignalingClient& operator=(SignalingClient&&) noexcept = default;

    /**
     * @brief Connect to the signaling server
     * @throws std::runtime_error if connection fails
     */
    void connect();

    /**
     * @brief Disconnect from the signaling server
     */
    void disconnect();

    /**
     * @brief Check if connected to signaling server
     * @return true if connected
     */
    bool isConnected() const;

    /**
     * @brief Send an SDP offer to the remote peer
     * @param sdp SDP offer string
     * @throws std::runtime_error if not connected or send fails
     */
    void sendOffer(const std::string& sdp);

    /**
     * @brief Send an SDP answer to the remote peer
     * @param sdp SDP answer string
     * @throws std::runtime_error if not connected or send fails
     */
    void sendAnswer(const std::string& sdp);

    /**
     * @brief Send an ICE candidate to the remote peer
     * @param candidate ICE candidate string
     * @param mid Media stream identification tag
     * @throws std::runtime_error if not connected or send fails
     */
    void sendIceCandidate(const std::string& candidate, const std::string& mid);

    /**
     * @brief Handle incoming message from signaling server
     *
     * This method parses the message and calls appropriate callbacks.
     * In a real implementation, this would be called by the transport layer.
     *
     * @param message JSON-encoded message
     */
    void handleMessage(const std::string& message);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core
} // namespace obswebrtc
