/**
 * @file livekit_docker_manager.cpp
 * @brief Implementation of LiveKit Docker manager for integration tests
 */

#include "livekit_docker_manager.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace obswebrtc {
namespace testing {

namespace {

/**
 * @brief Execute a shell command and capture output
 */
std::string execCommand(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;

#ifdef _WIN32
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
#endif

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

/**
 * @brief Check if a string contains another string
 */
bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

} // namespace

struct LiveKitDockerManager::Impl {
    LiveKitDockerConfig config;
    bool isRunning = false;

    Impl(const LiveKitDockerConfig& cfg) : config(cfg) {}
};

LiveKitDockerManager::LiveKitDockerManager(const LiveKitDockerConfig& config)
    : pImpl_(std::make_unique<Impl>(config)) {}

LiveKitDockerManager::~LiveKitDockerManager() {
    if (pImpl_ && pImpl_->isRunning) {
        stop();
    }
}

LiveKitDockerManager::LiveKitDockerManager(LiveKitDockerManager&&) noexcept = default;
LiveKitDockerManager& LiveKitDockerManager::operator=(LiveKitDockerManager&&) noexcept = default;

bool LiveKitDockerManager::dockerAvailable() const {
    try {
        std::string output = execCommand("docker --version");
        return contains(output, "Docker version");
    } catch (...) {
        return false;
    }
}

void LiveKitDockerManager::pullImageIfNeeded() {
    try {
        std::string checkCmd = "docker images -q " + pImpl_->config.imageName;
        std::string output = execCommand(checkCmd);

        if (output.empty()) {
            // Image not found, pull it
            std::string pullCmd = "docker pull " + pImpl_->config.imageName;
            execCommand(pullCmd);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to pull LiveKit image: " + std::string(e.what()));
    }
}

void LiveKitDockerManager::removeExistingContainer() {
    try {
        // Check if container exists
        std::string checkCmd = "docker ps -a -q -f name=" + pImpl_->config.containerName;
        std::string output = execCommand(checkCmd);

        if (!output.empty()) {
            // Container exists, remove it
            std::string removeCmd = "docker rm -f " + pImpl_->config.containerName;
            execCommand(removeCmd);
        }
    } catch (const std::exception& e) {
        // Ignore errors during cleanup
    }
}

void LiveKitDockerManager::start() {
    if (!dockerAvailable()) {
        throw std::runtime_error(
            "Docker is not available. Please install Docker to run integration tests.");
    }

    // Clean up any existing container
    removeExistingContainer();

    // Pull image if needed
    pullImageIfNeeded();

    // Build docker run command
    std::ostringstream cmd;
    cmd << "docker run -d "
        << "--name " << pImpl_->config.containerName << " "
        << "-p " << pImpl_->config.httpPort << ":7880 "
        << "-p " << pImpl_->config.rtcMinPort << "-" << pImpl_->config.rtcMaxPort << ":"
        << pImpl_->config.rtcMinPort << "-" << pImpl_->config.rtcMaxPort << "/udp "
        << "-e LIVEKIT_API_KEY=" << pImpl_->config.apiKey << " "
        << "-e LIVEKIT_API_SECRET=" << pImpl_->config.apiSecret << " ";

    if (pImpl_->config.enableDebugLogging) {
        cmd << "-e LIVEKIT_LOG_LEVEL=debug ";
    }

    cmd << pImpl_->config.imageName;

    try {
        std::string output = execCommand(cmd.str());
        if (output.empty()) {
            throw std::runtime_error("Failed to start Docker container");
        }

        pImpl_->isRunning = true;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to start LiveKit container: " + std::string(e.what()));
    }
}

void LiveKitDockerManager::stop() {
    if (!pImpl_->isRunning) {
        return;
    }

    try {
        std::string cmd = "docker stop " + pImpl_->config.containerName;
        execCommand(cmd);

        cmd = "docker rm " + pImpl_->config.containerName;
        execCommand(cmd);

        pImpl_->isRunning = false;
    } catch (const std::exception& e) {
        // Log error but don't throw in destructor path
    }
}

bool LiveKitDockerManager::waitForReady(int timeoutMs) {
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(timeoutMs);

    while (true) {
        if (isHealthy()) {
            return true;
        }

        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (elapsed >= timeout) {
            return false;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

bool LiveKitDockerManager::isHealthy() const {
    try {
        // Check if container is running
        std::string cmd = "docker inspect -f {{.State.Running}} " + pImpl_->config.containerName;
        std::string output = execCommand(cmd);

        if (!contains(output, "true")) {
            return false;
        }

        // TODO: Add HTTP health check to LiveKit API
        // For now, just check if container is running
        return true;
    } catch (...) {
        return false;
    }
}

std::string LiveKitDockerManager::getHttpUrl() const {
    std::ostringstream url;
    url << "http://localhost:" << pImpl_->config.httpPort;
    return url.str();
}

std::string LiveKitDockerManager::getWhipUrl(const std::string& roomName) const {
    std::ostringstream url;
    url << getHttpUrl() << "/whip/" << roomName;
    return url.str();
}

std::string LiveKitDockerManager::getWhepUrl(const std::string& roomName) const {
    std::ostringstream url;
    url << getHttpUrl() << "/whep/" << roomName;
    return url.str();
}

std::string LiveKitDockerManager::generateToken(const std::string& roomName,
                                                 const std::string& participantName,
                                                 bool canPublish, bool canSubscribe) const {
    // TODO: Implement proper JWT token generation
    // For now, return placeholder (will be implemented with JWT library)
    return "test-token-" + roomName + "-" + participantName;
}

std::string LiveKitDockerManager::getLogs(int tailLines) const {
    try {
        std::ostringstream cmd;
        cmd << "docker logs ";

        if (tailLines > 0) {
            cmd << "--tail " << tailLines << " ";
        }

        cmd << pImpl_->config.containerName;

        return execCommand(cmd.str());
    } catch (const std::exception& e) {
        return "Failed to get logs: " + std::string(e.what());
    }
}

} // namespace testing
} // namespace obswebrtc
