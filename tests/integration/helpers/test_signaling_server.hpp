/**
 * @file test_signaling_server.hpp
 * @brief Simple HTTP server for P2P signaling in tests
 */

#pragma once

#include <functional>
#include <memory>
#include <string>

namespace obswebrtc {
namespace testing {

/**
 * @brief Simple HTTP server for testing P2P connections
 *
 * Provides a basic signaling server that can exchange SDP offers/answers
 * and ICE candidates between peers for P2P integration tests.
 */
class TestSignalingServer {
public:
    using MessageHandler = std::function<void(const std::string& message)>;

    explicit TestSignalingServer(int port = 0); // 0 = auto-assign port
    ~TestSignalingServer();

    // Non-copyable, movable
    TestSignalingServer(const TestSignalingServer&) = delete;
    TestSignalingServer& operator=(const TestSignalingServer&) = delete;
    TestSignalingServer(TestSignalingServer&&) noexcept;
    TestSignalingServer& operator=(TestSignalingServer&&) noexcept;

    /**
     * @brief Start the signaling server
     */
    void start();

    /**
     * @brief Stop the signaling server
     */
    void stop();

    /**
     * @brief Get the port the server is listening on
     */
    int getPort() const;

    /**
     * @brief Get the base URL for the signaling server
     */
    std::string getUrl() const;

    /**
     * @brief Create a new signaling session
     * @return Session ID
     */
    std::string createSession();

    /**
     * @brief Post a message to a session
     * @param sessionId Session ID
     * @param message Message content
     */
    void postMessage(const std::string& sessionId, const std::string& message);

    /**
     * @brief Get pending messages for a session
     * @param sessionId Session ID
     * @return List of messages
     */
    std::vector<std::string> getMessages(const std::string& sessionId);

    /**
     * @brief Delete a session
     * @param sessionId Session ID
     */
    void deleteSession(const std::string& sessionId);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace testing
} // namespace obswebrtc
