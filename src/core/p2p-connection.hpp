/**
 * @file p2p-connection.hpp
 * @brief P2P Direct Connection for WebRTC
 *
 * This class implements P2P direct connection using manual signaling
 * via Session ID and clipboard exchange for Offer/Answer/ICE candidates.
 * Supports both Host and Client roles for low-latency direct communication.
 */

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace obswebrtc {
namespace core {

/**
 * @brief P2P connection role
 */
enum class P2PRole {
    None,   // Not initialized
    Host,   // Connection initiator (generates offer)
    Client  // Connection receiver (generates answer)
};

/**
 * @brief TURN server configuration
 */
struct TurnServer {
    std::string url;       // TURN server URL (e.g., turn:turn.example.com:3478)
    std::string username;  // TURN username
    std::string password;  // TURN password
};

/**
 * @brief Session information for P2P connection
 */
struct P2PSessionInfo {
    std::string sessionId;  // Unique session identifier
    P2PRole role;           // Host or Client
    std::string offer;      // SDP offer (for host)
    std::string answer;     // SDP answer (for client)
};

/**
 * @brief Callback function types for P2P connection events
 */
using P2PConnectedCallback = std::function<void()>;
using P2PDisconnectedCallback = std::function<void()>;
using P2PErrorCallback = std::function<void(const std::string& error)>;
using P2PIceCandidateCallback =
    std::function<void(const std::string& candidate, const std::string& mid)>;
using P2PSessionIdCallback = std::function<void(const std::string& sessionId)>;
using P2POfferCallback = std::function<void(const std::string& offer)>;
using P2PAnswerCallback = std::function<void(const std::string& answer)>;

/**
 * @brief Configuration for P2P Connection
 */
struct P2PConnectionConfig {
    // ICE servers configuration
    std::vector<std::string> stunServers;  // STUN server URLs
    std::vector<TurnServer> turnServers;   // TURN server configurations

    // Event callbacks
    P2PConnectedCallback onConnected;
    P2PDisconnectedCallback onDisconnected;
    P2PErrorCallback onError;
    P2PIceCandidateCallback onIceCandidate;
    P2PSessionIdCallback onSessionIdGenerated;
    P2POfferCallback onOfferGenerated;
    P2PAnswerCallback onAnswerGenerated;
};

/**
 * @brief P2P Direct Connection Manager
 *
 * This class manages P2P WebRTC connections using manual signaling.
 * It supports both Host and Client roles for establishing direct connections.
 *
 * Example usage (Host):
 * @code
 * P2PConnectionConfig config;
 * config.stunServers = {"stun:stun.l.google.com:19302"};
 * config.onOfferGenerated = [](const std::string& offer) {
 *     // Copy offer to clipboard for sharing
 * };
 *
 * auto connection = std::make_unique<P2PConnection>(config);
 * connection->initializeAsHost();
 * std::string sessionId = connection->generateSessionId();
 * std::string offer = connection->createOffer();
 * // Share session ID and offer with client
 * @endcode
 *
 * Example usage (Client):
 * @code
 * P2PConnectionConfig config;
 * config.stunServers = {"stun:stun.l.google.com:19302"};
 * config.onAnswerGenerated = [](const std::string& answer) {
 *     // Copy answer to clipboard for sharing back to host
 * };
 *
 * auto connection = std::make_unique<P2PConnection>(config);
 * connection->initializeAsClient(sessionId);
 * std::string answer = connection->setRemoteOffer(offer);
 * // Share answer with host
 * @endcode
 */
class P2PConnection {
public:
    /**
     * @brief Construct a new P2PConnection
     * @param config Configuration for the P2P connection
     */
    explicit P2PConnection(const P2PConnectionConfig& config);

    /**
     * @brief Destructor - closes connection and cleans up resources
     */
    ~P2PConnection();

    // Delete copy constructor and assignment operator (non-copyable)
    P2PConnection(const P2PConnection&) = delete;
    P2PConnection& operator=(const P2PConnection&) = delete;

    // Allow move semantics
    P2PConnection(P2PConnection&&) noexcept = default;
    P2PConnection& operator=(P2PConnection&&) noexcept = default;

    /**
     * @brief Generate a unique session ID
     * @return 8-character session ID string
     */
    std::string generateSessionId();

    /**
     * @brief Initialize as Host (connection initiator)
     * @throws std::runtime_error if already initialized
     */
    void initializeAsHost();

    /**
     * @brief Initialize as Client (connection receiver)
     * @param sessionId Session ID from host
     * @throws std::invalid_argument if session ID is empty
     * @throws std::runtime_error if already initialized
     */
    void initializeAsClient(const std::string& sessionId);

    /**
     * @brief Create SDP offer (Host only)
     * @return SDP offer string
     * @throws std::runtime_error if not initialized as Host
     */
    std::string createOffer();

    /**
     * @brief Set remote SDP answer (Host only)
     * @param answer SDP answer from client
     * @throws std::invalid_argument if answer is empty
     * @throws std::runtime_error if not initialized as Host
     */
    void setRemoteAnswer(const std::string& answer);

    /**
     * @brief Set remote SDP offer and create answer (Client only)
     * @param offer SDP offer from host
     * @return SDP answer string
     * @throws std::invalid_argument if offer is empty
     * @throws std::runtime_error if not initialized as Client
     */
    std::string setRemoteOffer(const std::string& offer);

    /**
     * @brief Add remote ICE candidate
     * @param candidate ICE candidate string
     * @param mid Media stream identification tag
     * @throws std::runtime_error if not connected
     */
    void addRemoteIceCandidate(const std::string& candidate, const std::string& mid);

    /**
     * @brief Disconnect from P2P connection
     */
    void disconnect();

    /**
     * @brief Get current connection role
     * @return P2PRole (None, Host, or Client)
     */
    P2PRole getRole() const;

    /**
     * @brief Get session ID
     * @return Session ID string (empty if not initialized)
     */
    std::string getSessionId() const;

    /**
     * @brief Get session information
     * @return P2PSessionInfo structure
     */
    P2PSessionInfo getSessionInfo() const;

    /**
     * @brief Check if connected
     * @return true if connected
     */
    bool isConnected() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace core
}  // namespace obswebrtc
