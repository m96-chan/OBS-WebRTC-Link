/**
 * @file connection-manager.cpp
 * @brief Multi-connection management implementation
 */

#include "connection-manager.hpp"

#include <chrono>
#include <map>
#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>

namespace obswebrtc {
namespace core {

// =============================================================================
// ConnectionManager Implementation
// =============================================================================

class ConnectionManager::Impl {
public:
    explicit Impl(const ConnectionManagerConfig& config)
        : config_(config) {
        if (config_.maxConnections == 0) {
            throw std::invalid_argument("maxConnections must be greater than 0");
        }
    }

    ~Impl() {
        removeAllConnections();
    }

    std::string createConnection(const std::string& serverUrl, const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (connections_.size() >= config_.maxConnections) {
            throw std::runtime_error("Maximum number of connections reached");
        }

        std::string connectionId = generateConnectionId();

        ConnectionInfo info;
        info.id = connectionId;
        info.name = name;
        info.serverUrl = serverUrl;
        info.state = ConnectionState::New;
        info.createdAt = getCurrentTimeMs();

        connections_[connectionId] = info;

        return connectionId;
    }

    bool removeConnection(const std::string& connectionId) {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = connections_.find(connectionId);
        if (it == connections_.end()) {
            return false;
        }

        connections_.erase(it);
        return true;
    }

    void removeAllConnections() {
        std::lock_guard<std::mutex> lock(mutex_);
        connections_.clear();
    }

    ConnectionInfo getConnectionInfo(const std::string& connectionId) const {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = connections_.find(connectionId);
        if (it == connections_.end()) {
            throw std::runtime_error("Connection not found: " + connectionId);
        }

        return it->second;
    }

    std::vector<ConnectionInfo> getAllConnections() const {
        std::lock_guard<std::mutex> lock(mutex_);

        std::vector<ConnectionInfo> result;
        result.reserve(connections_.size());

        for (const auto& pair : connections_) {
            result.push_back(pair.second);
        }

        return result;
    }

    bool connectionExists(const std::string& connectionId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connections_.find(connectionId) != connections_.end();
    }

    size_t getConnectionCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connections_.size();
    }

    size_t getMaxConnections() const {
        return config_.maxConnections;
    }

    bool hasAvailableSlots() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connections_.size() < config_.maxConnections;
    }

    void updateConnectionState(const std::string& connectionId, ConnectionState state) {
        ConnectionStateCallback callback;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto it = connections_.find(connectionId);
            if (it == connections_.end()) {
                throw std::runtime_error("Connection not found: " + connectionId);
            }

            it->second.state = state;
            callback = stateCallback_;
        }

        if (callback) {
            callback(connectionId, state);
        }
    }

    void reportError(const std::string& connectionId, const std::string& error) {
        ConnectionErrorCallback callback;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            callback = errorCallback_;
        }

        if (callback) {
            callback(connectionId, error);
        }
    }

    void setStateCallback(ConnectionStateCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        stateCallback_ = std::move(callback);
    }

    void setErrorCallback(ConnectionErrorCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        errorCallback_ = std::move(callback);
    }

private:
    std::string generateConnectionId() {
        // Generate a unique ID using random bytes
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* hex = "0123456789abcdef";

        std::stringstream ss;
        ss << "conn-";

        for (int i = 0; i < 16; i++) {
            ss << hex[dis(gen)];
        }

        return ss.str();
    }

    uint64_t getCurrentTimeMs() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
    }

    ConnectionManagerConfig config_;
    std::map<std::string, ConnectionInfo> connections_;
    mutable std::mutex mutex_;
    ConnectionStateCallback stateCallback_;
    ConnectionErrorCallback errorCallback_;
};

// =============================================================================
// ConnectionManager Public Interface
// =============================================================================

ConnectionManager::ConnectionManager(const ConnectionManagerConfig& config)
    : impl_(std::make_unique<Impl>(config)) {}

ConnectionManager::~ConnectionManager() = default;

std::string ConnectionManager::createConnection(const std::string& serverUrl, const std::string& name) {
    return impl_->createConnection(serverUrl, name);
}

bool ConnectionManager::removeConnection(const std::string& connectionId) {
    return impl_->removeConnection(connectionId);
}

void ConnectionManager::removeAllConnections() {
    impl_->removeAllConnections();
}

ConnectionInfo ConnectionManager::getConnectionInfo(const std::string& connectionId) const {
    return impl_->getConnectionInfo(connectionId);
}

std::vector<ConnectionInfo> ConnectionManager::getAllConnections() const {
    return impl_->getAllConnections();
}

bool ConnectionManager::connectionExists(const std::string& connectionId) const {
    return impl_->connectionExists(connectionId);
}

size_t ConnectionManager::getConnectionCount() const {
    return impl_->getConnectionCount();
}

size_t ConnectionManager::getMaxConnections() const {
    return impl_->getMaxConnections();
}

bool ConnectionManager::hasAvailableSlots() const {
    return impl_->hasAvailableSlots();
}

void ConnectionManager::updateConnectionState(const std::string& connectionId, ConnectionState state) {
    impl_->updateConnectionState(connectionId, state);
}

void ConnectionManager::reportError(const std::string& connectionId, const std::string& error) {
    impl_->reportError(connectionId, error);
}

void ConnectionManager::setStateCallback(ConnectionStateCallback callback) {
    impl_->setStateCallback(std::move(callback));
}

void ConnectionManager::setErrorCallback(ConnectionErrorCallback callback) {
    impl_->setErrorCallback(std::move(callback));
}

}  // namespace core
}  // namespace obswebrtc
