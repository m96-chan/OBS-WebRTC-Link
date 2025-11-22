/**
 * @file reconnection-manager.hpp
 * @brief Automatic reconnection manager with exponential backoff
 */

#pragma once

#include <functional>
#include <memory>
#include <cstdint>

namespace obswebrtc {
namespace core {

/**
 * @brief Reconnection callback
 */
using ReconnectCallback = std::function<void()>;

/**
 * @brief Reconnection state change callback
 */
using ReconnectionStateCallback = std::function<void(bool reconnecting, int retryCount)>;

/**
 * @brief Configuration for ReconnectionManager
 */
struct ReconnectionConfig {
    int maxRetries = 5;                    // Maximum number of retry attempts
    int64_t initialDelayMs = 1000;         // Initial delay in milliseconds
    int64_t maxDelayMs = 30000;            // Maximum delay in milliseconds
    ReconnectCallback reconnectCallback;   // Callback to perform reconnection
    ReconnectionStateCallback stateCallback;  // Callback for state changes
};

/**
 * @brief Reconnection Manager
 *
 * This class manages automatic reconnection with exponential backoff.
 * It handles:
 * - Scheduling reconnection attempts
 * - Exponential backoff delay calculation
 * - Maximum retry limit enforcement
 * - Cancellation and reset
 *
 * Features:
 * - Exponential backoff: delay = initialDelay * 2^retryCount
 * - Capped at maxDelay to prevent excessively long delays
 * - Thread-safe operations
 * - Asynchronous reconnection scheduling
 *
 * Example usage:
 * @code
 * ReconnectionConfig config;
 * config.maxRetries = 5;
 * config.initialDelayMs = 1000;
 * config.maxDelayMs = 30000;
 * config.reconnectCallback = [this]() {
 *     // Attempt reconnection
 *     this->connect();
 * };
 *
 * ReconnectionManager manager(config);
 *
 * // On connection failure:
 * manager.scheduleReconnect();
 *
 * // On successful reconnection:
 * manager.onConnectionSuccess();
 *
 * // To cancel reconnection:
 * manager.cancel();
 * @endcode
 */
class ReconnectionManager {
public:
    /**
     * @brief Construct a new ReconnectionManager
     * @param config Configuration for reconnection
     */
    explicit ReconnectionManager(const ReconnectionConfig& config);

    /**
     * @brief Destructor - cancels pending reconnections
     */
    ~ReconnectionManager();

    // Delete copy constructor and assignment operator (non-copyable)
    ReconnectionManager(const ReconnectionManager&) = delete;
    ReconnectionManager& operator=(const ReconnectionManager&) = delete;

    // Allow move semantics
    ReconnectionManager(ReconnectionManager&&) noexcept = default;
    ReconnectionManager& operator=(ReconnectionManager&&) noexcept = default;

    /**
     * @brief Schedule a reconnection attempt
     *
     * This schedules a reconnection after a delay calculated using
     * exponential backoff. If max retries is reached, no reconnection
     * is scheduled.
     *
     * @return true if reconnection was scheduled, false if max retries reached
     */
    bool scheduleReconnect();

    /**
     * @brief Cancel pending reconnection
     *
     * This cancels any scheduled reconnection attempt but does not
     * reset the retry count.
     */
    void cancel();

    /**
     * @brief Reset reconnection state
     *
     * This resets the retry count to 0 and cancels any pending
     * reconnection. Call this when you want to start fresh.
     */
    void reset();

    /**
     * @brief Notify manager of successful connection
     *
     * This resets the retry count to 0, indicating that the connection
     * was successfully established.
     */
    void onConnectionSuccess();

    /**
     * @brief Check if reconnection is in progress
     * @return true if reconnection is scheduled or in progress
     */
    bool isReconnecting() const;

    /**
     * @brief Get current retry count
     * @return Current number of retry attempts
     */
    int getRetryCount() const;

    /**
     * @brief Get next delay duration
     * @return Next delay in milliseconds
     */
    int64_t getNextDelay() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace core
} // namespace obswebrtc
