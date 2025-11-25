/**
 * @file webrtc-source.cpp
 * @brief WebRTC Source implementation for receiving streams
 */

#include "webrtc-source.hpp"
#include "core/whep-client.hpp"
#include "core/signaling-client.hpp"
#include "core/peer-connection.hpp"
#include "core/reconnection-manager.hpp"
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
        // Initialize reconnection manager if enabled
        if (config_.enableAutoReconnect) {
            core::ReconnectionConfig reconnectConfig;
            reconnectConfig.maxRetries = config_.maxReconnectRetries;
            reconnectConfig.initialDelayMs = config_.reconnectInitialDelayMs;
            reconnectConfig.maxDelayMs = config_.reconnectMaxDelayMs;
            reconnectConfig.reconnectCallback = [this]() {
                attemptReconnect();
            };
            reconnectionManager_ = std::make_unique<core::ReconnectionManager>(reconnectConfig);
        }
    }

    ~Impl()
    {
        stop();
    }

    bool start()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Check if already started
        if (active_ || whepClient_ || (signalingClient_ && peerConnection_)) {
            return false;
        }

        try {
            if (config_.connectionMode == ConnectionMode::WHEP) {
                return startWHEPMode();
            } else if (config_.connectionMode == ConnectionMode::P2P) {
                return startP2PMode();
            } else {
                throw std::runtime_error("Unknown connection mode");
            }
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

        // Cancel reconnection
        if (reconnectionManager_) {
            reconnectionManager_->cancel();
        }

        // Clean up WHEP client
        if (whepClient_) {
            whepClient_.reset();
        }

        // Clean up P2P connection
        if (peerConnection_) {
            peerConnection_.reset();
        }

        // Clean up signaling client
        if (signalingClient_) {
            signalingClient_->disconnect();
            signalingClient_.reset();
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
    bool startWHEPMode()
    {
        // Initialize WHEP client for receiving stream
        core::WHEPConfig whepConfig;
        whepConfig.url = config_.serverUrl;
        whepConfig.onConnected = [this]() {
            active_ = true;
            setConnectionState(ConnectionState::Connected);
            if (reconnectionManager_) {
                reconnectionManager_->onConnectionSuccess();
            }
        };
        whepConfig.onDisconnected = [this]() {
            active_ = false;
            setConnectionState(ConnectionState::Disconnected);
            if (reconnectionManager_ && config_.enableAutoReconnect) {
                reconnectionManager_->scheduleReconnect();
            }
        };
        whepConfig.onError = [this](const std::string& error) {
            if (config_.errorCallback) {
                config_.errorCallback(error);
            }
            setConnectionState(ConnectionState::Failed);
            if (reconnectionManager_ && config_.enableAutoReconnect) {
                reconnectionManager_->scheduleReconnect();
            }
        };

        // Create WHEP client
        whepClient_ = std::make_unique<core::WHEPClient>(whepConfig);
        setConnectionState(ConnectionState::Connecting);
        return true;
    }

    bool startP2PMode()
    {
        if (config_.signalingUrl.empty()) {
            throw std::runtime_error("Signaling URL is required for P2P mode");
        }

        if (config_.sessionId.empty()) {
            throw std::runtime_error("Session ID is required for P2P mode");
        }

        // Initialize signaling client
        core::SignalingConfig signalingConfig;
        signalingConfig.url = config_.signalingUrl;
        signalingConfig.onConnected = [this]() {
            // Signaling connected, now initiate WebRTC connection
            initP2PPeerConnection();
        };
        signalingConfig.onDisconnected = [this]() {
            active_ = false;
            setConnectionState(ConnectionState::Disconnected);
            if (reconnectionManager_ && config_.enableAutoReconnect) {
                reconnectionManager_->scheduleReconnect();
            }
        };
        signalingConfig.onError = [this](const std::string& error) {
            if (config_.errorCallback) {
                config_.errorCallback(std::string("Signaling error: ") + error);
            }
            setConnectionState(ConnectionState::Failed);
            if (reconnectionManager_ && config_.enableAutoReconnect) {
                reconnectionManager_->scheduleReconnect();
            }
        };
        signalingConfig.onOffer = [this](const std::string& sdp) {
            handleRemoteOffer(sdp);
        };
        signalingConfig.onAnswer = [this](const std::string& sdp) {
            handleRemoteAnswer(sdp);
        };
        signalingConfig.onIceCandidate = [this](const std::string& candidate, const std::string& mid) {
            if (peerConnection_) {
                try {
                    peerConnection_->addIceCandidate(candidate, mid);
                } catch (const std::exception& e) {
                    if (config_.errorCallback) {
                        config_.errorCallback(std::string("Failed to add ICE candidate: ") + e.what());
                    }
                }
            }
        };

        signalingClient_ = std::make_unique<core::SignalingClient>(signalingConfig);
        signalingClient_->connect();
        setConnectionState(ConnectionState::Connecting);
        return true;
    }

    void initP2PPeerConnection()
    {
        // Initialize peer connection config
        core::PeerConnectionConfig pcConfig;

        // Setup state change callback
        pcConfig.stateCallback = [this](core::ConnectionState state) {
            switch (state) {
                case core::ConnectionState::Connected:
                case core::ConnectionState::Completed:
                    active_ = true;
                    setConnectionState(ConnectionState::Connected);
                    if (reconnectionManager_) {
                        reconnectionManager_->onConnectionSuccess();
                    }
                    break;

                case core::ConnectionState::Disconnected:
                case core::ConnectionState::Closed:
                    active_ = false;
                    setConnectionState(ConnectionState::Disconnected);
                    if (reconnectionManager_ && config_.enableAutoReconnect) {
                        reconnectionManager_->scheduleReconnect();
                    }
                    break;

                case core::ConnectionState::Failed:
                    active_ = false;
                    setConnectionState(ConnectionState::Failed);
                    if (reconnectionManager_ && config_.enableAutoReconnect) {
                        reconnectionManager_->scheduleReconnect();
                    }
                    break;

                default:
                    break;
            }
        };

        // Setup ICE candidate callback
        pcConfig.iceCandidateCallback = [this](const std::string& candidate, const std::string& mid) {
            if (signalingClient_ && signalingClient_->isConnected()) {
                try {
                    signalingClient_->sendIceCandidate(candidate, mid);
                } catch (const std::exception& e) {
                    if (config_.errorCallback) {
                        config_.errorCallback(std::string("Failed to send ICE candidate: ") + e.what());
                    }
                }
            }
        };

        // Setup local description callback (for both offer and answer)
        pcConfig.localDescriptionCallback = [this](core::SdpType type, const std::string& sdp) {
            if (signalingClient_ && signalingClient_->isConnected()) {
                try {
                    if (type == core::SdpType::Offer) {
                        signalingClient_->sendOffer(sdp);
                    } else {
                        signalingClient_->sendAnswer(sdp);
                    }
                } catch (const std::exception& e) {
                    if (config_.errorCallback) {
                        config_.errorCallback(std::string("Failed to send SDP: ") + e.what());
                    }
                }
            }
        };

        // Setup log callback
        pcConfig.logCallback = [this](core::LogLevel level, const std::string& message) {
            // Only report errors via error callback
            if (level == core::LogLevel::Error && config_.errorCallback) {
                config_.errorCallback(message);
            }
        };

        // Setup video frame callback
        pcConfig.videoFrameCallback = [this](const core::VideoFrame& coreFrame) {
            if (config_.videoCallback) {
                // Convert core::VideoFrame to source::VideoFrame
                source::VideoFrame sourceFrame;
                sourceFrame.data = coreFrame.data;
                sourceFrame.width = coreFrame.width;
                sourceFrame.height = coreFrame.height;
                sourceFrame.timestamp = coreFrame.timestamp;
                sourceFrame.keyframe = coreFrame.keyframe;
                config_.videoCallback(sourceFrame);
            }
        };

        // Setup audio frame callback
        pcConfig.audioFrameCallback = [this](const core::AudioFrame& coreFrame) {
            if (config_.audioCallback) {
                // Convert core::AudioFrame to source::AudioFrame
                source::AudioFrame sourceFrame;
                sourceFrame.data = coreFrame.data;
                sourceFrame.sampleRate = coreFrame.sampleRate;
                sourceFrame.channels = coreFrame.channels;
                sourceFrame.timestamp = coreFrame.timestamp;
                config_.audioCallback(sourceFrame);
            }
        };

        // Create peer connection
        peerConnection_ = std::make_unique<core::PeerConnection>(pcConfig);
    }

    void handleRemoteOffer(const std::string& sdp)
    {
        if (!peerConnection_) {
            initP2PPeerConnection();
        }

        try {
            peerConnection_->setRemoteDescription(core::SdpType::Offer, sdp);
            // Create answer after setting remote offer
            peerConnection_->createAnswer();
            // Answer will be sent via localDescriptionCallback
        } catch (const std::exception& e) {
            if (config_.errorCallback) {
                config_.errorCallback(std::string("Failed to handle remote offer: ") + e.what());
            }
        }
    }

    void handleRemoteAnswer(const std::string& sdp)
    {
        if (peerConnection_) {
            try {
                peerConnection_->setRemoteDescription(core::SdpType::Answer, sdp);
            } catch (const std::exception& e) {
                if (config_.errorCallback) {
                    config_.errorCallback(std::string("Failed to handle remote answer: ") + e.what());
                }
            }
        }
    }

    void attemptReconnect()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Clean up existing connections
        if (whepClient_) {
            whepClient_.reset();
        }
        if (peerConnection_) {
            peerConnection_.reset();
        }
        if (signalingClient_) {
            signalingClient_->disconnect();
            signalingClient_.reset();
        }

        // Reset state
        active_ = false;
        setConnectionState(ConnectionState::Disconnected);
    }

    void setConnectionState(ConnectionState state)
    {
        connectionState_ = state;
        if (config_.stateCallback) {
            config_.stateCallback(state);
        }
    }

    WebRTCSourceConfig config_;
    std::unique_ptr<core::WHEPClient> whepClient_;
    std::unique_ptr<core::SignalingClient> signalingClient_;
    std::unique_ptr<core::PeerConnection> peerConnection_;
    std::unique_ptr<core::ReconnectionManager> reconnectionManager_;
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
