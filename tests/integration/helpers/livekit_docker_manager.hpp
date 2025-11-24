/**
 * @file livekit_docker_manager.hpp
 * @brief Manages LiveKit Docker container for integration tests
 */

#pragma once

#include <memory>
#include <string>

namespace obswebrtc {
namespace testing {

/**
 * @brief Configuration for LiveKit Docker instance
 */
struct LiveKitDockerConfig {
    std::string containerName = "obs-webrtc-test-livekit";
    std::string imageName = "livekit/livekit-server:latest";
    int httpPort = 7880;
    int rtcMinPort = 50000;
    int rtcMaxPort = 50100;
    std::string apiKey = "APItest123";
    std::string apiSecret = "SECRETtest456";
    bool enableDebugLogging = false;
};

/**
 * @brief Manages LiveKit server in Docker for integration tests
 *
 * Automatically starts LiveKit in Docker container when created,
 * and stops/removes container when destroyed. Provides API for
 * generating test tokens and checking server health.
 */
class LiveKitDockerManager {
public:
    explicit LiveKitDockerManager(const LiveKitDockerConfig& config = LiveKitDockerConfig{});
    ~LiveKitDockerManager();

    // Non-copyable, movable
    LiveKitDockerManager(const LiveKitDockerManager&) = delete;
    LiveKitDockerManager& operator=(const LiveKitDockerManager&) = delete;
    LiveKitDockerManager(LiveKitDockerManager&&) noexcept;
    LiveKitDockerManager& operator=(LiveKitDockerManager&&) noexcept;

    /**
     * @brief Start the LiveKit Docker container
     * @throws std::runtime_error if Docker is not available or start fails
     */
    void start();

    /**
     * @brief Stop and remove the LiveKit Docker container
     */
    void stop();

    /**
     * @brief Wait for LiveKit server to become ready
     * @param timeoutMs Maximum time to wait in milliseconds
     * @return true if server is ready, false if timeout
     */
    bool waitForReady(int timeoutMs = 30000);

    /**
     * @brief Get the WHIP endpoint URL
     * @param roomName Room name for the connection
     * @return WHIP endpoint URL
     */
    std::string getWhipUrl(const std::string& roomName) const;

    /**
     * @brief Get the WHEP endpoint URL
     * @param roomName Room name for the connection
     * @return WHEP endpoint URL
     */
    std::string getWhepUrl(const std::string& roomName) const;

    /**
     * @brief Generate a LiveKit access token for testing
     * @param roomName Room name
     * @param participantName Participant name
     * @param canPublish Whether participant can publish
     * @param canSubscribe Whether participant can subscribe
     * @return JWT access token
     */
    std::string generateToken(const std::string& roomName, const std::string& participantName,
                               bool canPublish = true, bool canSubscribe = true) const;

    /**
     * @brief Check if LiveKit server is running and healthy
     * @return true if healthy, false otherwise
     */
    bool isHealthy() const;

    /**
     * @brief Get server logs (for debugging failed tests)
     * @param tailLines Number of lines to retrieve (0 = all)
     * @return Container logs
     */
    std::string getLogs(int tailLines = 100) const;

    /**
     * @brief Get the HTTP URL for the LiveKit server
     * @return HTTP URL (e.g., "http://localhost:7880")
     */
    std::string getHttpUrl() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl_;

    bool dockerAvailable() const;
    void pullImageIfNeeded();
    void removeExistingContainer();
};

} // namespace testing
} // namespace obswebrtc
