/**
 * @file connection-manager.hpp
 * @brief Multi-connection management for WebRTC streams
 *
 * This module provides:
 * - Management of multiple concurrent WebRTC connections
 * - Connection lifecycle management
 * - Resource allocation and limits
 * - Thread-safe operations
 */

#pragma once

#include "peer-connection.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace obswebrtc {
namespace core {

/**
 * @brief Information about a managed connection
 */
struct ConnectionInfo {
    std::string id;           ///< Unique connection identifier
    std::string name;         ///< Human-readable name
    std::string serverUrl;    ///< Server URL for the connection
    ConnectionState state = ConnectionState::New;  ///< Current connection state
    uint64_t createdAt = 0;   ///< Creation timestamp (ms since epoch)
};

/**
 * @brief Callback for connection state changes
 */
using ConnectionStateCallback = std::function<void(const std::string& connectionId, ConnectionState state)>;

/**
 * @brief Callback for connection errors
 */
using ConnectionErrorCallback = std::function<void(const std::string& connectionId, const std::string& error)>;

/**
 * @brief Configuration for ConnectionManager
 */
struct ConnectionManagerConfig {
    size_t maxConnections = 4;  ///< Maximum number of concurrent connections
};

/**
 * @brief Manages multiple concurrent WebRTC connections
 *
 * This class provides centralized management for multiple WebRTC connections,
 * including:
 * - Connection creation and removal
 * - Connection state tracking
 * - Resource limits (max connections)
 * - Thread-safe operations
 *
 * Example usage:
 * @code
 * ConnectionManagerConfig config;
 * config.maxConnections = 4;
 *
 * ConnectionManager manager(config);
 *
 * // Create connections
 * std::string conn1 = manager.createConnection("http://server/whep1", "Camera 1");
 * std::string conn2 = manager.createConnection("http://server/whep2", "Camera 2");
 *
 * // Set callbacks
 * manager.setStateCallback([](const std::string& id, ConnectionState state) {
 *     std::cout << "Connection " << id << " state: " << static_cast<int>(state) << std::endl;
 * });
 *
 * // Get connection info
 * ConnectionInfo info = manager.getConnectionInfo(conn1);
 *
 * // Remove connection
 * manager.removeConnection(conn1);
 * @endcode
 */
class ConnectionManager {
public:
    /**
     * @brief Construct a new Connection Manager
     * @param config Configuration for the manager
     * @throws std::invalid_argument if maxConnections is 0
     */
    explicit ConnectionManager(const ConnectionManagerConfig& config);

    /**
     * @brief Destructor - cleans up all connections
     */
    ~ConnectionManager();

    // Delete copy constructor and assignment (non-copyable)
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    /**
     * @brief Create a new connection
     * @param serverUrl URL for the WebRTC server
     * @param name Human-readable name for the connection
     * @return Unique connection ID
     * @throws std::runtime_error if max connections exceeded
     */
    std::string createConnection(const std::string& serverUrl, const std::string& name);

    /**
     * @brief Remove a connection
     * @param connectionId ID of the connection to remove
     * @return true if connection was removed, false if not found
     */
    bool removeConnection(const std::string& connectionId);

    /**
     * @brief Remove all connections
     */
    void removeAllConnections();

    /**
     * @brief Get connection information
     * @param connectionId ID of the connection
     * @return Connection information
     * @throws std::runtime_error if connection not found
     */
    ConnectionInfo getConnectionInfo(const std::string& connectionId) const;

    /**
     * @brief Get all connections
     * @return Vector of connection information
     */
    std::vector<ConnectionInfo> getAllConnections() const;

    /**
     * @brief Check if a connection exists
     * @param connectionId ID of the connection
     * @return true if connection exists
     */
    bool connectionExists(const std::string& connectionId) const;

    /**
     * @brief Get the current number of connections
     * @return Number of active connections
     */
    size_t getConnectionCount() const;

    /**
     * @brief Get the maximum number of connections
     * @return Maximum connection limit
     */
    size_t getMaxConnections() const;

    /**
     * @brief Check if there are available connection slots
     * @return true if new connections can be created
     */
    bool hasAvailableSlots() const;

    /**
     * @brief Update connection state
     * @param connectionId ID of the connection
     * @param state New connection state
     * @throws std::runtime_error if connection not found
     */
    void updateConnectionState(const std::string& connectionId, ConnectionState state);

    /**
     * @brief Report an error for a connection
     * @param connectionId ID of the connection
     * @param error Error message
     */
    void reportError(const std::string& connectionId, const std::string& error);

    /**
     * @brief Set callback for connection state changes
     * @param callback Function to call on state changes
     */
    void setStateCallback(ConnectionStateCallback callback);

    /**
     * @brief Set callback for connection errors
     * @param callback Function to call on errors
     */
    void setErrorCallback(ConnectionErrorCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace core
}  // namespace obswebrtc
