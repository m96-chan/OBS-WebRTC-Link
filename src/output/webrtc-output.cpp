/**
 * @file webrtc-output.cpp
 * @brief WebRTC Output implementation
 */

#include "output/webrtc-output.hpp"
#include "core/whip-client.hpp"
#include "core/peer-connection.hpp"
#include "core/reconnection-manager.hpp"
#include <stdexcept>
#include <mutex>

namespace obswebrtc {
namespace output {

/**
 * @brief Implementation class for WebRTCOutput
 */
class WebRTCOutput::Impl {
public:
    explicit Impl(const WebRTCOutputConfig& config)
        : config_(config), active_(false), starting_(false), videoBitrate_(config.videoBitrate),
          audioBitrate_(config.audioBitrate) {
        if (config_.serverUrl.empty()) {
            throw std::runtime_error("Server URL cannot be empty");
        }

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

    ~Impl() {
        stop();
    }

    bool start() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (active_ || starting_) {
            return false; // Already active or starting
        }

        starting_ = true;

        try {
            // Create WHIP client configuration
            core::WHIPConfig whipConfig;
            whipConfig.url = config_.serverUrl;
            whipConfig.onConnected = [this]() {
                if (config_.stateCallback) {
                    config_.stateCallback(true);
                }
            };
            whipConfig.onDisconnected = [this]() {
                if (config_.stateCallback) {
                    config_.stateCallback(false);
                }
                active_ = false;
            };
            whipConfig.onError = [this](const std::string& error) {
                if (config_.errorCallback) {
                    config_.errorCallback(error);
                }
            };

            // Create WHIP client
            whipClient_ = std::make_unique<core::WHIPClient>(whipConfig);

            // Create peer connection configuration
            core::PeerConnectionConfig pcConfig;
            pcConfig.iceServers = {"stun:stun.l.google.com:19302"};
            pcConfig.localDescriptionCallback = [this](core::SdpType type, const std::string& sdp) {
                if (type == core::SdpType::Offer && whipClient_) {
                    try {
                        std::string answer = whipClient_->sendOffer(sdp);
                        if (peerConnection_) {
                            peerConnection_->setRemoteDescription(core::SdpType::Answer, answer);
                        }
                    } catch (const std::exception& e) {
                        if (config_.errorCallback) {
                            config_.errorCallback(std::string("Failed to send offer: ") + e.what());
                        }
                    }
                }
            };
            pcConfig.iceCandidateCallback = [this](const std::string& candidate, const std::string& mid) {
                if (whipClient_ && whipClient_->isConnected()) {
                    try {
                        whipClient_->sendIceCandidate(candidate, mid);
                    } catch (const std::exception& e) {
                        // Ignore ICE candidate errors (non-critical)
                    }
                }
            };
            pcConfig.stateCallback = [this](core::ConnectionState state) {
                if (state == core::ConnectionState::Connected || state == core::ConnectionState::Completed) {
                    active_ = true;
                    if (config_.stateCallback) {
                        config_.stateCallback(true);
                    }
                    // Reset reconnection manager on successful connection
                    if (reconnectionManager_) {
                        reconnectionManager_->onConnectionSuccess();
                    }
                } else if (state == core::ConnectionState::Failed || state == core::ConnectionState::Disconnected) {
                    active_ = false;
                    if (config_.stateCallback) {
                        config_.stateCallback(false);
                    }
                    // Schedule reconnection on failure
                    if (reconnectionManager_ && config_.enableAutoReconnect) {
                        reconnectionManager_->scheduleReconnect();
                    }
                }
            };

            // Create peer connection
            peerConnection_ = std::make_unique<core::PeerConnection>(pcConfig);

            // Create offer to initiate connection
            peerConnection_->createOffer();

            return true;
        } catch (const std::exception& e) {
            starting_ = false;
            if (config_.errorCallback) {
                config_.errorCallback(std::string("Failed to start output: ") + e.what());
            }
            return false;
        }
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!active_ && !starting_) {
            return;
        }

        active_ = false;
        starting_ = false;

        // Cancel reconnection
        if (reconnectionManager_) {
            reconnectionManager_->cancel();
        }

        // Close WHIP client
        if (whipClient_) {
            whipClient_->disconnect();
            whipClient_.reset();
        }

        // Close peer connection
        if (peerConnection_) {
            peerConnection_->close();
            peerConnection_.reset();
        }

        if (config_.stateCallback) {
            config_.stateCallback(false);
        }
    }

    bool isActive() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return active_;
    }

    void sendPacket(const EncodedPacket& packet) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!active_) {
            throw std::runtime_error("Output is not active");
        }

        // TODO: Implement actual packet sending via peer connection
        // This will require adding track support to PeerConnection
        // For now, just validate the packet
        if (packet.data.empty()) {
            throw std::runtime_error("Packet data is empty");
        }
    }

    int getVideoBitrate() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return videoBitrate_;
    }

    int getAudioBitrate() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return audioBitrate_;
    }

    void setVideoBitrate(int bitrate) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (bitrate <= 0) {
            throw std::invalid_argument("Video bitrate must be positive");
        }
        videoBitrate_ = bitrate;
    }

    void setAudioBitrate(int bitrate) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (bitrate <= 0) {
            throw std::invalid_argument("Audio bitrate must be positive");
        }
        audioBitrate_ = bitrate;
    }

private:
    void attemptReconnect() {
        std::lock_guard<std::mutex> lock(mutex_);

        // Clean up existing connections
        if (whipClient_) {
            whipClient_->disconnect();
            whipClient_.reset();
        }
        if (peerConnection_) {
            peerConnection_->close();
            peerConnection_.reset();
        }

        // Attempt to start again
        // Note: We need to unlock before calling start() to avoid deadlock
        // So we'll just set a flag to restart
        active_ = false;
        starting_ = false;
    }

    WebRTCOutputConfig config_;
    std::unique_ptr<core::WHIPClient> whipClient_;
    std::unique_ptr<core::PeerConnection> peerConnection_;
    std::unique_ptr<core::ReconnectionManager> reconnectionManager_;
    bool active_;
    bool starting_;
    int videoBitrate_;
    int audioBitrate_;
    mutable std::mutex mutex_;
};

// WebRTCOutput public interface implementation

WebRTCOutput::WebRTCOutput(const WebRTCOutputConfig& config)
    : impl_(std::make_unique<Impl>(config)) {}

WebRTCOutput::~WebRTCOutput() = default;

bool WebRTCOutput::start() {
    return impl_->start();
}

void WebRTCOutput::stop() {
    impl_->stop();
}

bool WebRTCOutput::isActive() const {
    return impl_->isActive();
}

void WebRTCOutput::sendPacket(const EncodedPacket& packet) {
    impl_->sendPacket(packet);
}

int WebRTCOutput::getVideoBitrate() const {
    return impl_->getVideoBitrate();
}

int WebRTCOutput::getAudioBitrate() const {
    return impl_->getAudioBitrate();
}

void WebRTCOutput::setVideoBitrate(int bitrate) {
    impl_->setVideoBitrate(bitrate);
}

void WebRTCOutput::setAudioBitrate(int bitrate) {
    impl_->setAudioBitrate(bitrate);
}

} // namespace output
} // namespace obswebrtc
