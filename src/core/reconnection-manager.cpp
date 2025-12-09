/**
 * @file reconnection-manager.cpp
 * @brief Automatic reconnection manager implementation
 */

#include "reconnection-manager.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace obswebrtc {
namespace core {

/**
 * @brief Private implementation of ReconnectionManager
 *
 * Uses a single worker thread with condition variable for efficient
 * reconnection scheduling. This avoids creating a new thread for each
 * reconnection attempt.
 */
class ReconnectionManager::Impl {
public:
    explicit Impl(const ReconnectionConfig& config)
        : config_(config)
        , retryCount_(0)
        , reconnecting_(false)
        , stopped_(false)
        , hasScheduledReconnect_(false)
    {
        // Start the worker thread
        workerThread_ = std::thread([this]() { workerLoop(); });
    }

    ~Impl()
    {
        stop();
    }

    bool scheduleReconnect()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        // Check if max retries reached
        if (retryCount_ >= config_.maxRetries) {
            return false;
        }

        // Calculate delay using exponential backoff
        int64_t delay = calculateDelay();

        // Increment retry count
        retryCount_++;

        // Schedule the reconnection
        reconnectTime_ = std::chrono::steady_clock::now() +
                         std::chrono::milliseconds(delay);
        hasScheduledReconnect_ = true;
        reconnecting_ = true;

        // Notify state change
        if (config_.stateCallback) {
            config_.stateCallback(true, retryCount_);
        }

        // Wake up the worker thread
        cv_.notify_one();

        return true;
    }

    void cancel()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            hasScheduledReconnect_ = false;
            reconnecting_ = false;
        }
        // Wake up worker thread so it can check the cancelled state
        cv_.notify_one();
    }

    void reset()
    {
        cancel();

        std::lock_guard<std::mutex> lock(mutex_);
        retryCount_ = 0;

        // Notify state change
        if (config_.stateCallback) {
            config_.stateCallback(false, 0);
        }
    }

    void onConnectionSuccess()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        retryCount_ = 0;
        reconnecting_ = false;
        hasScheduledReconnect_ = false;

        // Notify state change
        if (config_.stateCallback) {
            config_.stateCallback(false, 0);
        }
    }

    bool isReconnecting() const
    {
        return reconnecting_;
    }

    int getRetryCount() const
    {
        return retryCount_;
    }

    int64_t getNextDelay() const
    {
        return calculateDelay();
    }

private:
    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stopped_ = true;
            hasScheduledReconnect_ = false;
        }
        cv_.notify_one();

        if (workerThread_.joinable()) {
            workerThread_.join();
        }
    }

    void workerLoop()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);

            // Wait until we have a scheduled reconnect or are stopped
            cv_.wait(lock, [this] {
                return stopped_ || hasScheduledReconnect_;
            });

            if (stopped_) {
                break;
            }

            if (hasScheduledReconnect_) {
                // Wait until the reconnect time or cancellation
                auto waitResult = cv_.wait_until(lock, reconnectTime_, [this] {
                    return stopped_ || !hasScheduledReconnect_;
                });

                // Check if we should perform the reconnect
                // waitResult is false if the timeout expired (time to reconnect)
                if (!waitResult && hasScheduledReconnect_ && !stopped_) {
                    hasScheduledReconnect_ = false;

                    // Store callback and state to call outside lock
                    auto callback = config_.reconnectCallback;
                    auto stateCallback = config_.stateCallback;
                    int currentRetryCount = retryCount_;

                    // Release lock before calling callbacks to avoid deadlock
                    lock.unlock();

                    // Call reconnect callback
                    if (callback) {
                        callback();
                    }

                    // Re-acquire lock to update state
                    lock.lock();
                    reconnecting_ = false;

                    // Notify state change
                    if (stateCallback) {
                        stateCallback(false, currentRetryCount);
                    }
                }
            }
        }
    }

    int64_t calculateDelay() const
    {
        // Exponential backoff: delay = initialDelay * 2^retryCount
        int64_t delay = config_.initialDelayMs;

        for (int i = 0; i < retryCount_; i++) {
            delay *= 2;
            // Cap at max delay
            if (delay >= config_.maxDelayMs) {
                delay = config_.maxDelayMs;
                break;
            }
        }

        return std::min(delay, config_.maxDelayMs);
    }

    ReconnectionConfig config_;
    std::atomic<int> retryCount_;
    std::atomic<bool> reconnecting_;
    std::atomic<bool> stopped_;
    bool hasScheduledReconnect_;
    std::chrono::steady_clock::time_point reconnectTime_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread workerThread_;
};

// ReconnectionManager implementation

ReconnectionManager::ReconnectionManager(const ReconnectionConfig& config)
    : impl_(std::make_unique<Impl>(config))
{
}

ReconnectionManager::~ReconnectionManager() = default;

bool ReconnectionManager::scheduleReconnect()
{
    return impl_->scheduleReconnect();
}

void ReconnectionManager::cancel()
{
    impl_->cancel();
}

void ReconnectionManager::reset()
{
    impl_->reset();
}

void ReconnectionManager::onConnectionSuccess()
{
    impl_->onConnectionSuccess();
}

bool ReconnectionManager::isReconnecting() const
{
    return impl_->isReconnecting();
}

int ReconnectionManager::getRetryCount() const
{
    return impl_->getRetryCount();
}

int64_t ReconnectionManager::getNextDelay() const
{
    return impl_->getNextDelay();
}

} // namespace core
} // namespace obswebrtc
