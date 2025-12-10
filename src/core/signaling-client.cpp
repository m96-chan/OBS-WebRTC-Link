/**
 * @file signaling-client.cpp
 * @brief Implementation of WebRTC Signaling Client
 */

#include "core/signaling-client.hpp"

#include <nlohmann/json.hpp>

#include <mutex>
#include <stdexcept>

namespace obswebrtc {
namespace core {

/**
 * @brief Default WebSocket-based signaling transport
 *
 * This is a simple implementation that can be replaced with
 * a real WebSocket implementation (e.g., using libdatachannel's WebSocket).
 * For now, it provides a mock implementation for testing.
 */
class DefaultSignalingTransport : public SignalingTransport {
public:
    DefaultSignalingTransport() : connected_(false) {}

    void connect(const std::string& url) override {
        url_ = url;
        connected_ = true;
        // In a real implementation, this would establish a WebSocket connection
    }

    void disconnect() override {
        connected_ = false;
        // In a real implementation, this would close the WebSocket connection
    }

    void sendMessage(const std::string& message) override {
        if (!connected_) {
            throw std::runtime_error("Not connected to signaling server");
        }
        // In a real implementation, this would send the message over WebSocket
        lastSentMessage_ = message;
    }

    bool isConnected() const override {
        return connected_;
    }

    std::string getLastSentMessage() const {
        return lastSentMessage_;
    }

private:
    std::string url_;
    bool connected_;
    std::string lastSentMessage_;
};

/**
 * @brief Private implementation of SignalingClient (Pimpl idiom)
 */
class SignalingClient::Impl {
public:
    explicit Impl(const SignalingConfig& config, std::unique_ptr<SignalingTransport> transport)
        : config_(config), transport_(std::move(transport)) {
        // Validate configuration
        if (config_.url.empty()) {
            throw std::invalid_argument("Signaling URL cannot be empty");
        }

        // If no transport provided, use default
        if (!transport_) {
            transport_ = std::make_unique<DefaultSignalingTransport>();
        }
    }

    ~Impl() {
        disconnect();
    }

    void connect() {
        std::lock_guard<std::mutex> lock(mutex_);

        try {
            transport_->connect(config_.url);

            // Call connected callback
            if (config_.onConnected) {
                config_.onConnected();
            }
        } catch (const std::exception& e) {
            if (config_.onError) {
                config_.onError(std::string("Connection failed: ") + e.what());
            }
            throw;
        }
    }

    void disconnect() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (transport_ && transport_->isConnected()) {
            transport_->disconnect();

            // Call disconnected callback
            if (config_.onDisconnected) {
                config_.onDisconnected();
            }
        }
    }

    bool isConnected() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return transport_ && transport_->isConnected();
    }

    void sendOffer(const std::string& sdp) {
        std::lock_guard<std::mutex> lock(mutex_);

        validateSdp(sdp, "offer");
        validateConnected();

        nlohmann::json message;
        message["type"] = "offer";
        message["sdp"] = sdp;

        try {
            transport_->sendMessage(message.dump());
        } catch (const std::exception& e) {
            if (config_.onError) {
                config_.onError(std::string("Failed to send offer: ") + e.what());
            }
            throw;
        }
    }

    void sendAnswer(const std::string& sdp) {
        std::lock_guard<std::mutex> lock(mutex_);

        validateSdp(sdp, "answer");
        validateConnected();

        nlohmann::json message;
        message["type"] = "answer";
        message["sdp"] = sdp;

        try {
            transport_->sendMessage(message.dump());
        } catch (const std::exception& e) {
            if (config_.onError) {
                config_.onError(std::string("Failed to send answer: ") + e.what());
            }
            throw;
        }
    }

    void sendIceCandidate(const std::string& candidate, const std::string& mid) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (candidate.empty()) {
            throw std::invalid_argument("ICE candidate cannot be empty");
        }
        validateConnected();

        nlohmann::json message;
        message["type"] = "candidate";
        message["candidate"] = candidate;
        message["mid"] = mid;

        try {
            transport_->sendMessage(message.dump());
        } catch (const std::exception& e) {
            if (config_.onError) {
                config_.onError(std::string("Failed to send ICE candidate: ") + e.what());
            }
            throw;
        }
    }

    void handleMessage(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);

        try {
            auto json = nlohmann::json::parse(message);

            if (!json.contains("type")) {
                if (config_.onError) {
                    config_.onError("Received message without 'type' field");
                }
                return;
            }

            std::string type = json["type"];

            if (type == "offer") {
                if (!json.contains("sdp")) {
                    if (config_.onError) {
                        config_.onError("Received offer message without 'sdp' field");
                    }
                    return;
                }
                if (config_.onOffer) {
                    config_.onOffer(json["sdp"].get<std::string>());
                }
            } else if (type == "answer") {
                if (!json.contains("sdp")) {
                    if (config_.onError) {
                        config_.onError("Received answer message without 'sdp' field");
                    }
                    return;
                }
                if (config_.onAnswer) {
                    config_.onAnswer(json["sdp"].get<std::string>());
                }
            } else if (type == "candidate") {
                if (!json.contains("candidate") || !json.contains("mid")) {
                    if (config_.onError) {
                        config_.onError("Received candidate message without 'candidate' or 'mid' field");
                    }
                    return;
                }
                if (config_.onIceCandidate) {
                    config_.onIceCandidate(json["candidate"].get<std::string>(), json["mid"].get<std::string>());
                }
            } else {
                if (config_.onError) {
                    config_.onError("Unknown message type: " + type);
                }
            }
        } catch (const nlohmann::json::exception& e) {
            if (config_.onError) {
                config_.onError(std::string("Failed to parse message: ") + e.what());
            }
        } catch (const std::exception& e) {
            if (config_.onError) {
                config_.onError(std::string("Error handling message: ") + e.what());
            }
        }
    }

private:
    // Validation helper: throws if not connected (must be called with mutex held)
    void validateConnected() const {
        if (!transport_ || !transport_->isConnected()) {
            throw std::runtime_error("Not connected to signaling server");
        }
    }

    // Validation helper: throws if SDP is empty (must be called with mutex held)
    void validateSdp(const std::string& sdp, const char* type) const {
        if (sdp.empty()) {
            throw std::invalid_argument(std::string("SDP ") + type + " cannot be empty");
        }
    }

    SignalingConfig config_;
    std::unique_ptr<SignalingTransport> transport_;
    mutable std::mutex mutex_;
};

// SignalingClient public interface implementation

SignalingClient::SignalingClient(const SignalingConfig& config)
    : impl_(std::make_unique<Impl>(config, nullptr)) {
}

SignalingClient::SignalingClient(const SignalingConfig& config, std::unique_ptr<SignalingTransport> transport)
    : impl_(std::make_unique<Impl>(config, std::move(transport))) {
}

SignalingClient::~SignalingClient() = default;

void SignalingClient::connect() {
    impl_->connect();
}

void SignalingClient::disconnect() {
    impl_->disconnect();
}

bool SignalingClient::isConnected() const {
    return impl_->isConnected();
}

void SignalingClient::sendOffer(const std::string& sdp) {
    impl_->sendOffer(sdp);
}

void SignalingClient::sendAnswer(const std::string& sdp) {
    impl_->sendAnswer(sdp);
}

void SignalingClient::sendIceCandidate(const std::string& candidate, const std::string& mid) {
    impl_->sendIceCandidate(candidate, mid);
}

void SignalingClient::handleMessage(const std::string& message) {
    impl_->handleMessage(message);
}

}  // namespace core
}  // namespace obswebrtc
