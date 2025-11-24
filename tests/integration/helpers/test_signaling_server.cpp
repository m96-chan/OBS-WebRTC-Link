/**
 * @file test_signaling_server.cpp
 * @brief Implementation of simple test signaling server
 */

#include "test_signaling_server.hpp"

#include <map>
#include <mutex>
#include <random>
#include <sstream>
#include <vector>

namespace obswebrtc {
namespace testing {

struct TestSignalingServer::Impl {
    int port = 0;
    bool running = false;
    std::mutex mutex;
    std::map<std::string, std::vector<std::string>> sessions;

    std::string generateSessionId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);

        const char* hex = "0123456789ABCDEF";
        std::string id(8, '0');
        for (char& c : id) {
            c = hex[dis(gen)];
        }
        return id;
    }
};

TestSignalingServer::TestSignalingServer(int port) : pImpl_(std::make_unique<Impl>()) {
    pImpl_->port = port;
}

TestSignalingServer::~TestSignalingServer() {
    stop();
}

TestSignalingServer::TestSignalingServer(TestSignalingServer&&) noexcept = default;
TestSignalingServer& TestSignalingServer::operator=(TestSignalingServer&&) noexcept = default;

void TestSignalingServer::start() {
    // TODO: Implement actual HTTP server
    // For MVP, we'll use in-memory message passing
    pImpl_->running = true;
}

void TestSignalingServer::stop() {
    pImpl_->running = false;
    std::lock_guard<std::mutex> lock(pImpl_->mutex);
    pImpl_->sessions.clear();
}

int TestSignalingServer::getPort() const { return pImpl_->port; }

std::string TestSignalingServer::getUrl() const {
    std::ostringstream url;
    url << "http://localhost:" << pImpl_->port;
    return url.str();
}

std::string TestSignalingServer::createSession() {
    std::lock_guard<std::mutex> lock(pImpl_->mutex);
    std::string sessionId = pImpl_->generateSessionId();
    pImpl_->sessions[sessionId] = std::vector<std::string>();
    return sessionId;
}

void TestSignalingServer::postMessage(const std::string& sessionId, const std::string& message) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex);
    auto it = pImpl_->sessions.find(sessionId);
    if (it != pImpl_->sessions.end()) {
        it->second.push_back(message);
    }
}

std::vector<std::string> TestSignalingServer::getMessages(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex);
    auto it = pImpl_->sessions.find(sessionId);
    if (it != pImpl_->sessions.end()) {
        std::vector<std::string> messages = it->second;
        it->second.clear(); // Clear after reading
        return messages;
    }
    return {};
}

void TestSignalingServer::deleteSession(const std::string& sessionId) {
    std::lock_guard<std::mutex> lock(pImpl_->mutex);
    pImpl_->sessions.erase(sessionId);
}

} // namespace testing
} // namespace obswebrtc
