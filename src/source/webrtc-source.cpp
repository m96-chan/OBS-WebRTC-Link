/**
 * @file webrtc-source.cpp
 * @brief WebRTC Source implementation for receiving streams
 */

#include "webrtc-source.hpp"
#include "core/whep-client.hpp"
#include <stdexcept>
#include <atomic>
#include <mutex>

namespace obswebrtc {
namespace source {

/**
 * @brief Private implementation of WebRTCSource
 */
class WebRTCSource::Impl {
public:
    explicit Impl(const WebRTCSourceConfig& config)
        : config_(config)
        , active_(false)
        , connectionState_(ConnectionState::Disconnected)
    {
    }

    ~Impl()
    {
        stop();
    }

    bool start()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (active_) {
            return false;
        }

        try {
            // Initialize WHEP client for receiving stream
            core::WHEPConfig whepConfig;
            whepConfig.url = config_.serverUrl;
            whepConfig.onConnected = [this]() {
                setConnectionState(ConnectionState::Connected);
            };
            whepConfig.onDisconnected = [this]() {
                setConnectionState(ConnectionState::Disconnected);
            };
            whepConfig.onError = [this](const std::string& error) {
                if (config_.errorCallback) {
                    config_.errorCallback(error);
                }
                setConnectionState(ConnectionState::Failed);
            };

            whepClient_ = std::make_unique<core::WHEPClient>(whepConfig);

            // Start connection
            setConnectionState(ConnectionState::Connecting);
            active_ = true;

            return true;
        } catch (const std::exception& e) {
            if (config_.errorCallback) {
                config_.errorCallback(std::string("Failed to start source: ") + e.what());
            }
            active_ = false;
            setConnectionState(ConnectionState::Failed);
            return false;
        }
    }

    void stop()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!active_) {
            return;
        }

        if (whepClient_) {
            whepClient_.reset();
        }

        active_ = false;
        setConnectionState(ConnectionState::Disconnected);
    }

    bool isActive() const
    {
        return active_;
    }

    ConnectionState getConnectionState() const
    {
        return connectionState_;
    }

private:
    void setConnectionState(ConnectionState state)
    {
        connectionState_ = state;
        if (config_.stateCallback) {
            config_.stateCallback(state);
        }
    }

    WebRTCSourceConfig config_;
    std::unique_ptr<core::WHEPClient> whepClient_;
    std::atomic<bool> active_;
    std::atomic<ConnectionState> connectionState_;
    std::mutex mutex_;
};

// WebRTCSource implementation

WebRTCSource::WebRTCSource(const WebRTCSourceConfig& config)
    : pImpl(std::make_unique<Impl>(config))
{
}

WebRTCSource::~WebRTCSource() = default;

bool WebRTCSource::start()
{
    return pImpl->start();
}

void WebRTCSource::stop()
{
    pImpl->stop();
}

bool WebRTCSource::isActive() const
{
    return pImpl->isActive();
}

ConnectionState WebRTCSource::getConnectionState() const
{
    return pImpl->getConnectionState();
}

} // namespace source
} // namespace obswebrtc
