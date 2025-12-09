/**
 * @file connection_manager_test.cpp
 * @brief Unit tests for ConnectionManager (multi-stream support)
 */

#include "core/connection-manager.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>
#include <thread>

using namespace obswebrtc::core;
using namespace testing;

/**
 * @brief Test fixture for ConnectionManager tests
 */
class ConnectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.maxConnections = 4;
    }

    void TearDown() override {
        // No teardown needed
    }

    ConnectionManagerConfig config_;
};

// =============================================================================
// ConnectionInfo Tests
// =============================================================================

/**
 * @brief Test default ConnectionInfo values
 */
TEST_F(ConnectionManagerTest, ConnectionInfoDefaultValues) {
    ConnectionInfo info;

    EXPECT_TRUE(info.id.empty());
    EXPECT_TRUE(info.name.empty());
    EXPECT_EQ(info.state, ConnectionState::New);
    EXPECT_TRUE(info.serverUrl.empty());
}

// =============================================================================
// ConnectionManager Construction Tests
// =============================================================================

/**
 * @brief Test manager construction with default config
 */
TEST_F(ConnectionManagerTest, ConstructionWithDefaultConfig) {
    ConnectionManagerConfig defaultConfig;
    EXPECT_NO_THROW({
        ConnectionManager manager(defaultConfig);
    });
}

/**
 * @brief Test manager construction with custom config
 */
TEST_F(ConnectionManagerTest, ConstructionWithCustomConfig) {
    config_.maxConnections = 8;

    EXPECT_NO_THROW({
        ConnectionManager manager(config_);
    });
}

/**
 * @brief Test invalid max connections (zero)
 */
TEST_F(ConnectionManagerTest, InvalidMaxConnectionsZero) {
    config_.maxConnections = 0;

    EXPECT_THROW({
        ConnectionManager manager(config_);
    }, std::invalid_argument);
}

// =============================================================================
// Connection Management Tests
// =============================================================================

/**
 * @brief Test creating a connection
 */
TEST_F(ConnectionManagerTest, CreateConnection) {
    ConnectionManager manager(config_);

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");

    EXPECT_FALSE(connId.empty());
    EXPECT_EQ(manager.getConnectionCount(), 1);
}

/**
 * @brief Test creating multiple connections
 */
TEST_F(ConnectionManagerTest, CreateMultipleConnections) {
    ConnectionManager manager(config_);

    std::string conn1 = manager.createConnection("http://localhost:8080/whep1", "Connection 1");
    std::string conn2 = manager.createConnection("http://localhost:8080/whep2", "Connection 2");
    std::string conn3 = manager.createConnection("http://localhost:8080/whep3", "Connection 3");

    EXPECT_EQ(manager.getConnectionCount(), 3);
    EXPECT_NE(conn1, conn2);
    EXPECT_NE(conn2, conn3);
    EXPECT_NE(conn1, conn3);
}

/**
 * @brief Test exceeding max connections
 */
TEST_F(ConnectionManagerTest, ExceedMaxConnections) {
    config_.maxConnections = 2;
    ConnectionManager manager(config_);

    manager.createConnection("http://localhost:8080/whep1", "Connection 1");
    manager.createConnection("http://localhost:8080/whep2", "Connection 2");

    EXPECT_THROW({
        manager.createConnection("http://localhost:8080/whep3", "Connection 3");
    }, std::runtime_error);
}

/**
 * @brief Test removing a connection
 */
TEST_F(ConnectionManagerTest, RemoveConnection) {
    ConnectionManager manager(config_);

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");
    EXPECT_EQ(manager.getConnectionCount(), 1);

    bool removed = manager.removeConnection(connId);
    EXPECT_TRUE(removed);
    EXPECT_EQ(manager.getConnectionCount(), 0);
}

/**
 * @brief Test removing non-existent connection
 */
TEST_F(ConnectionManagerTest, RemoveNonExistentConnection) {
    ConnectionManager manager(config_);

    bool removed = manager.removeConnection("non-existent-id");
    EXPECT_FALSE(removed);
}

/**
 * @brief Test getting connection info
 */
TEST_F(ConnectionManagerTest, GetConnectionInfo) {
    ConnectionManager manager(config_);

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");

    ConnectionInfo info = manager.getConnectionInfo(connId);
    EXPECT_EQ(info.id, connId);
    EXPECT_EQ(info.name, "Test Connection");
    EXPECT_EQ(info.serverUrl, "http://localhost:8080/whep");
}

/**
 * @brief Test getting info for non-existent connection
 */
TEST_F(ConnectionManagerTest, GetNonExistentConnectionInfo) {
    ConnectionManager manager(config_);

    EXPECT_THROW({
        manager.getConnectionInfo("non-existent-id");
    }, std::runtime_error);
}

/**
 * @brief Test getting all connections
 */
TEST_F(ConnectionManagerTest, GetAllConnections) {
    ConnectionManager manager(config_);

    manager.createConnection("http://localhost:8080/whep1", "Connection 1");
    manager.createConnection("http://localhost:8080/whep2", "Connection 2");

    std::vector<ConnectionInfo> connections = manager.getAllConnections();
    EXPECT_EQ(connections.size(), 2);
}

/**
 * @brief Test connection exists check
 */
TEST_F(ConnectionManagerTest, ConnectionExists) {
    ConnectionManager manager(config_);

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");

    EXPECT_TRUE(manager.connectionExists(connId));
    EXPECT_FALSE(manager.connectionExists("non-existent-id"));
}

// =============================================================================
// Connection State Tests
// =============================================================================

/**
 * @brief Test initial connection state
 */
TEST_F(ConnectionManagerTest, InitialConnectionState) {
    ConnectionManager manager(config_);

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");

    ConnectionInfo info = manager.getConnectionInfo(connId);
    EXPECT_EQ(info.state, ConnectionState::New);
}

/**
 * @brief Test updating connection state
 */
TEST_F(ConnectionManagerTest, UpdateConnectionState) {
    ConnectionManager manager(config_);

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");

    manager.updateConnectionState(connId, ConnectionState::Checking);
    ConnectionInfo info = manager.getConnectionInfo(connId);
    EXPECT_EQ(info.state, ConnectionState::Checking);

    manager.updateConnectionState(connId, ConnectionState::Connected);
    info = manager.getConnectionInfo(connId);
    EXPECT_EQ(info.state, ConnectionState::Connected);
}

/**
 * @brief Test updating state for non-existent connection
 */
TEST_F(ConnectionManagerTest, UpdateNonExistentConnectionState) {
    ConnectionManager manager(config_);

    EXPECT_THROW({
        manager.updateConnectionState("non-existent-id", ConnectionState::Connected);
    }, std::runtime_error);
}

// =============================================================================
// Resource Management Tests
// =============================================================================

/**
 * @brief Test removing all connections
 */
TEST_F(ConnectionManagerTest, RemoveAllConnections) {
    ConnectionManager manager(config_);

    manager.createConnection("http://localhost:8080/whep1", "Connection 1");
    manager.createConnection("http://localhost:8080/whep2", "Connection 2");
    manager.createConnection("http://localhost:8080/whep3", "Connection 3");

    EXPECT_EQ(manager.getConnectionCount(), 3);

    manager.removeAllConnections();

    EXPECT_EQ(manager.getConnectionCount(), 0);
}

/**
 * @brief Test has available slots
 */
TEST_F(ConnectionManagerTest, HasAvailableSlots) {
    config_.maxConnections = 2;
    ConnectionManager manager(config_);

    EXPECT_TRUE(manager.hasAvailableSlots());

    manager.createConnection("http://localhost:8080/whep1", "Connection 1");
    EXPECT_TRUE(manager.hasAvailableSlots());

    manager.createConnection("http://localhost:8080/whep2", "Connection 2");
    EXPECT_FALSE(manager.hasAvailableSlots());
}

/**
 * @brief Test get max connections
 */
TEST_F(ConnectionManagerTest, GetMaxConnections) {
    config_.maxConnections = 8;
    ConnectionManager manager(config_);

    EXPECT_EQ(manager.getMaxConnections(), 8);
}

// =============================================================================
// Connection Callback Tests
// =============================================================================

/**
 * @brief Test connection state callback
 */
TEST_F(ConnectionManagerTest, ConnectionStateCallback) {
    ConnectionManager manager(config_);
    bool callbackInvoked = false;
    std::string receivedConnId;
    ConnectionState receivedState;

    manager.setStateCallback([&](const std::string& connId, ConnectionState state) {
        callbackInvoked = true;
        receivedConnId = connId;
        receivedState = state;
    });

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");
    manager.updateConnectionState(connId, ConnectionState::Connected);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(receivedConnId, connId);
    EXPECT_EQ(receivedState, ConnectionState::Connected);
}

/**
 * @brief Test error callback
 */
TEST_F(ConnectionManagerTest, ErrorCallback) {
    ConnectionManager manager(config_);
    bool callbackInvoked = false;
    std::string receivedConnId;
    std::string receivedError;

    manager.setErrorCallback([&](const std::string& connId, const std::string& error) {
        callbackInvoked = true;
        receivedConnId = connId;
        receivedError = error;
    });

    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");
    manager.reportError(connId, "Test error message");

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(receivedConnId, connId);
    EXPECT_EQ(receivedError, "Test error message");
}

// =============================================================================
// Thread Safety Tests
// =============================================================================

/**
 * @brief Test concurrent connection creation
 */
TEST_F(ConnectionManagerTest, ConcurrentConnectionCreation) {
    config_.maxConnections = 100;
    ConnectionManager manager(config_);
    const int numThreads = 10;
    const int connectionsPerThread = 5;

    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < connectionsPerThread; j++) {
                try {
                    std::string url = "http://localhost:8080/whep" + std::to_string(i * connectionsPerThread + j);
                    std::string name = "Connection " + std::to_string(i * connectionsPerThread + j);
                    manager.createConnection(url, name);
                    successCount++;
                } catch (...) {
                    // Ignore errors (max connections reached)
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(manager.getConnectionCount(), successCount.load());
    EXPECT_LE(manager.getConnectionCount(), static_cast<size_t>(numThreads * connectionsPerThread));
}

/**
 * @brief Test concurrent read/write operations
 */
TEST_F(ConnectionManagerTest, ConcurrentReadWrite) {
    config_.maxConnections = 10;
    ConnectionManager manager(config_);

    // Create initial connections
    std::string connId = manager.createConnection("http://localhost:8080/whep", "Test Connection");

    std::atomic<bool> running{true};
    const int iterations = 100;

    // Writer thread
    std::thread writer([&]() {
        for (int i = 0; i < iterations && running; i++) {
            manager.updateConnectionState(connId, ConnectionState::Checking);
            manager.updateConnectionState(connId, ConnectionState::Connected);
        }
    });

    // Reader thread
    std::thread reader([&]() {
        for (int i = 0; i < iterations && running; i++) {
            try {
                ConnectionInfo info = manager.getConnectionInfo(connId);
                (void)info.state;  // Just access it
            } catch (...) {
                // Ignore if connection was removed
            }
        }
    });

    writer.join();
    running = false;
    reader.join();

    // Should complete without crash
    EXPECT_TRUE(manager.connectionExists(connId));
}

// =============================================================================
// ID Generation Tests
// =============================================================================

/**
 * @brief Test connection ID uniqueness
 */
TEST_F(ConnectionManagerTest, UniqueConnectionIds) {
    config_.maxConnections = 100;
    ConnectionManager manager(config_);

    std::set<std::string> ids;
    for (int i = 0; i < 50; i++) {
        std::string connId = manager.createConnection(
            "http://localhost:8080/whep" + std::to_string(i),
            "Connection " + std::to_string(i)
        );
        EXPECT_TRUE(ids.find(connId) == ids.end()) << "Duplicate ID: " << connId;
        ids.insert(connId);
    }

    EXPECT_EQ(ids.size(), 50);
}
