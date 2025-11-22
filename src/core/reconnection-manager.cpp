/**
 * @file reconnection-manager.cpp
 * @brief Automatic reconnection manager implementation
 */

#include "reconnection-manager.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <algorithm>

namespace obswebrtc {
namespace core {

/**
 * @brief Private implementation of ReconnectionManager
 */
class ReconnectionManager::Impl {
public:
    explicit Impl(const ReconnectionConfig& config)
        : config_(config)
        , retryCount_(0)
        , reconnecting_(false)
        , cancelled_(false)
    {
    }

    ~Impl()
    {
        cancel();
    }

    bool scheduleReconnect()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Check if max retries reached
        if (retryCount_ >= config_.maxRetries) {
            return false;
        }

        // Cancel any pending reconnection
        cancelled_ = true;
        if (reconnectThread_.joinable()) {
            reconnectThread_.join();
        }

        // Calculate delay using exponential backoff
        int64_t delay = calculateDelay();

        // Increment retry count
        retryCount_++;

        // Update state
        reconnecting_ = true;
        cancelled_ = false;

        // Notify state change
        if (config_.stateCallback) {
            config_.stateCallback(true, retryCount_);
        }

        // Schedule reconnection on a new thread
        reconnectThread_ = std::thread([this, delay]() {
            // Wait for delay
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));

            // Check if cancelled
            if (cancelled_) {
                return;
            }

            // Call reconnect callback
            if (config_.reconnectCallback) {
                config_.reconnectCallback();
            }

            // Update state
            std::lock_guard<std::mutex> lock(mutex_);
            reconnecting_ = false;

            // Notify state change
            if (config_.stateCallback) {
                config_.stateCallback(false, retryCount_);
            }
        });

        return true;
    }

    void cancel()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        cancelled_ = true;
        reconnecting_ = false;

        if (reconnectThread_.joinable()) {
            reconnectThread_.join();
        }
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
    std::atomic<bool> cancelled_;
    std::mutex mutex_;
    std::thread reconnectThread_;
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
