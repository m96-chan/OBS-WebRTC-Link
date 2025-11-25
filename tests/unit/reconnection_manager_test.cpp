/**
 * @file reconnection_manager_test.cpp
 * @brief Unit tests for ReconnectionManager
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core/reconnection-manager.hpp"
#include <thread>
#include <chrono>

using namespace obswebrtc::core;
using namespace testing;

/**
 * @brief Test fixture for ReconnectionManager tests
 */
class ReconnectionManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

/**
 * @brief Test that ReconnectionManager can be constructed
 */
TEST_F(ReconnectionManagerTest, CanConstruct) {
    ReconnectionConfig config;
    config.maxRetries = 5;
    config.initialDelayMs = 1000;
    config.maxDelayMs = 30000;

    EXPECT_NO_THROW({
        ReconnectionManager manager(config);
    });
}

/**
 * @brief Test that ReconnectionManager starts in idle state
 */
TEST_F(ReconnectionManagerTest, StartsInIdleState) {
    ReconnectionConfig config;
    config.maxRetries = 5;
    config.initialDelayMs = 1000;

    ReconnectionManager manager(config);

    EXPECT_FALSE(manager.isReconnecting());
    EXPECT_EQ(manager.getRetryCount(), 0);
}

/**
 * @brief Test that ReconnectionManager schedules reconnection
 */
TEST_F(ReconnectionManagerTest, SchedulesReconnection) {
    std::atomic<bool> reconnectCalled{false};

    ReconnectionConfig config;
    config.maxRetries = 5;
    config.initialDelayMs = 100;  // Short delay for testing
    config.reconnectCallback = [&reconnectCalled]() {
        reconnectCalled = true;
    };

    ReconnectionManager manager(config);

    manager.scheduleReconnect();
    EXPECT_TRUE(manager.isReconnecting());

    // Wait for reconnect callback with timeout (more reliable than fixed sleep)
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(1000);  // Generous timeout for slow CI

    while (!reconnectCalled.load()) {
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (elapsed >= timeout) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_TRUE(reconnectCalled.load()) << "Reconnect callback was not invoked within timeout";
}

/**
 * @brief Test exponential backoff
 */
TEST_F(ReconnectionManagerTest, UsesExponentialBackoff) {
    std::vector<int64_t> reconnectTimes;

    ReconnectionConfig config;
    config.maxRetries = 3;
    config.initialDelayMs = 50;  // Shorter initial delay for faster test
    config.maxDelayMs = 1000;
    config.reconnectCallback = [&reconnectTimes]() {
        reconnectTimes.push_back(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
    };

    ReconnectionManager manager(config);

    auto startTime = std::chrono::steady_clock::now();

    // Schedule first reconnection
    manager.scheduleReconnect();
    // Wait for first callback to complete (50ms delay + buffer)
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Schedule second reconnection
    manager.scheduleReconnect();
    // Wait for second callback to complete (100ms delay + buffer)
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    // Check that we got at least 2 reconnect callbacks
    EXPECT_GE(reconnectTimes.size(), 2);
    if (reconnectTimes.size() >= 2) {
        // Calculate the time differences between callbacks
        int64_t firstDelay = reconnectTimes[0] - std::chrono::duration_cast<std::chrono::milliseconds>(startTime.time_since_epoch()).count();
        int64_t secondDelay = reconnectTimes[1] - reconnectTimes[0];

        // First delay should be approximately initialDelayMs (50ms)
        EXPECT_GE(firstDelay, 40);  // Allow some tolerance
        EXPECT_LE(firstDelay, 200);

        // Second delay should be approximately 2x initialDelayMs (100ms) due to exponential backoff
        EXPECT_GE(secondDelay, 80);  // Allow some tolerance
        EXPECT_LE(secondDelay, 400);  // Increased tolerance for macOS CI timing variations

        // Second delay should be longer than first (exponential backoff verification)
        EXPECT_GT(secondDelay, firstDelay);
    }
}

/**
 * @brief Test maximum retry limit
 */
TEST_F(ReconnectionManagerTest, RespectsMaxRetries) {
    int reconnectCount = 0;

    ReconnectionConfig config;
    config.maxRetries = 3;
    config.initialDelayMs = 50;
    config.reconnectCallback = [&reconnectCount]() {
        reconnectCount++;
    };

    ReconnectionManager manager(config);

    // Schedule more reconnections than max retries
    for (int i = 0; i < 5; i++) {
        manager.scheduleReconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Should not exceed max retries
    EXPECT_LE(reconnectCount, 3);
}

/**
 * @brief Test that reset clears retry count
 */
TEST_F(ReconnectionManagerTest, ResetClearsRetryCount) {
    ReconnectionConfig config;
    config.maxRetries = 5;
    config.initialDelayMs = 100;

    ReconnectionManager manager(config);

    manager.scheduleReconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_GT(manager.getRetryCount(), 0);

    manager.reset();

    EXPECT_EQ(manager.getRetryCount(), 0);
    EXPECT_FALSE(manager.isReconnecting());
}

/**
 * @brief Test that cancel stops reconnection
 */
TEST_F(ReconnectionManagerTest, CancelStopsReconnection) {
    bool reconnectCalled = false;

    ReconnectionConfig config;
    config.maxRetries = 5;
    config.initialDelayMs = 200;
    config.reconnectCallback = [&reconnectCalled]() {
        reconnectCalled = true;
    };

    ReconnectionManager manager(config);

    manager.scheduleReconnect();
    EXPECT_TRUE(manager.isReconnecting());

    // Cancel before callback fires
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    manager.cancel();

    EXPECT_FALSE(manager.isReconnecting());

    // Wait to ensure callback doesn't fire
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(reconnectCalled);
}

/**
 * @brief Test successful reconnection resets retry count
 */
TEST_F(ReconnectionManagerTest, SuccessfulReconnectResetsCount) {
    ReconnectionConfig config;
    config.maxRetries = 5;
    config.initialDelayMs = 100;

    ReconnectionManager manager(config);

    manager.scheduleReconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    EXPECT_GT(manager.getRetryCount(), 0);

    manager.onConnectionSuccess();

    EXPECT_EQ(manager.getRetryCount(), 0);
}

/**
 * @brief Test max delay cap
 */
TEST_F(ReconnectionManagerTest, CapsDelayAtMaxDelay) {
    ReconnectionConfig config;
    config.maxRetries = 10;
    config.initialDelayMs = 100;
    config.maxDelayMs = 500;

    ReconnectionManager manager(config);

    // Schedule many reconnections to test max delay cap
    for (int i = 0; i < 5; i++) {
        manager.scheduleReconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }

    // Delay should be capped at maxDelayMs
    int64_t nextDelay = manager.getNextDelay();
    EXPECT_LE(nextDelay, 500);
}
