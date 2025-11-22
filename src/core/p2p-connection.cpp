/**
 * @file p2p-connection.cpp
 * @brief Implementation of P2P Direct Connection
 */

#include "p2p-connection.hpp"

#include <rtc/rtc.hpp>

#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>

namespace obswebrtc {
namespace core {

/**
 * @brief Internal implementation of P2PConnection (Pimpl idiom)
 */
class P2PConnection::Impl {
public:
    explicit Impl(const P2PConnectionConfig& config)
        : config_(config)
        , role_(P2PRole::None)
        , connected_(false) {
    }

    ~Impl() {
        disconnect();
    }

    std::string generateSessionId() {
        // Generate 8-character alphanumeric session ID
        static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

        std::string sessionId;
        sessionId.reserve(8);
        for (int i = 0; i < 8; ++i) {
            sessionId += alphanum[dis(gen)];
        }

        sessionId_ = sessionId;

        // Call callback
        if (config_.onSessionIdGenerated) {
            config_.onSessionIdGenerated(sessionId);
        }

        return sessionId;
    }

    void initializeAsHost() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (role_ != P2PRole::None) {
            throw std::runtime_error("Already initialized as " +
                                     roleToString(role_));
        }

        role_ = P2PRole::Host;

        // Create PeerConnection with ICE servers
        createPeerConnection();
    }

    void initializeAsClient(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (sessionId.empty()) {
            throw std::invalid_argument("Session ID cannot be empty");
        }

        if (role_ != P2PRole::None) {
            throw std::runtime_error("Already initialized as " +
                                     roleToString(role_));
        }

        role_ = P2PRole::Client;
        sessionId_ = sessionId;

        // Create PeerConnection with ICE servers
        createPeerConnection();
    }

    std::string createOffer() {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (role_ != P2PRole::Host) {
                throw std::runtime_error("Can only create offer as Host");
            }

            if (!peerConnection_) {
                throw std::runtime_error("PeerConnection not initialized");
            }

            // Create datachannel (required for offer)
            auto dc = peerConnection_->createDataChannel("control");
        }

        // Set local description callback to capture offer
        std::promise<std::string> offerPromise;
        auto offerFuture = offerPromise.get_future();

        peerConnection_->onLocalDescription(
            [this, &offerPromise](rtc::Description description) {
                std::string sdp = std::string(description);
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    offer_ = sdp;
                }

                // Call callback
                if (config_.onOfferGenerated) {
                    config_.onOfferGenerated(sdp);
                }

                offerPromise.set_value(sdp);
            });

        // Wait for offer to be generated (without holding mutex)
        auto status = offerFuture.wait_for(std::chrono::seconds(5));
        if (status == std::future_status::timeout) {
            throw std::runtime_error("Timeout waiting for offer generation");
        }

        return offerFuture.get();
    }

    void setRemoteAnswer(const std::string& answer) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (answer.empty()) {
            throw std::invalid_argument("Answer cannot be empty");
        }

        if (role_ != P2PRole::Host) {
            throw std::runtime_error("Can only set remote answer as Host");
        }

        if (!peerConnection_) {
            throw std::runtime_error("PeerConnection not initialized");
        }

        // Set remote description
        rtc::Description remoteDesc(answer, rtc::Description::Type::Answer);
        peerConnection_->setRemoteDescription(remoteDesc);

        answer_ = answer;
    }

    std::string setRemoteOffer(const std::string& offer) {
        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (offer.empty()) {
                throw std::invalid_argument("Offer cannot be empty");
            }

            if (role_ != P2PRole::Client) {
                throw std::runtime_error("Can only set remote offer as Client");
            }

            if (!peerConnection_) {
                throw std::runtime_error("PeerConnection not initialized");
            }

            offer_ = offer;

            // Set remote description
            rtc::Description remoteDesc(offer, rtc::Description::Type::Offer);
            peerConnection_->setRemoteDescription(remoteDesc);
        }

        // Set local description callback to capture answer
        std::promise<std::string> answerPromise;
        auto answerFuture = answerPromise.get_future();

        peerConnection_->onLocalDescription(
            [this, &answerPromise](rtc::Description description) {
                std::string sdp = std::string(description);
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    answer_ = sdp;
                }

                // Call callback
                if (config_.onAnswerGenerated) {
                    config_.onAnswerGenerated(sdp);
                }

                answerPromise.set_value(sdp);
            });

        // Wait for answer to be generated (without holding mutex)
        auto status = answerFuture.wait_for(std::chrono::seconds(5));
        if (status == std::future_status::timeout) {
            throw std::runtime_error("Timeout waiting for answer generation");
        }

        return answerFuture.get();
    }

    void addRemoteIceCandidate(const std::string& candidate,
                               const std::string& mid) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            throw std::runtime_error("PeerConnection not initialized");
        }

        rtc::Candidate rtcCandidate(candidate, mid);
        peerConnection_->addRemoteCandidate(rtcCandidate);
    }

    void disconnect() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (peerConnection_) {
            peerConnection_->close();
            peerConnection_.reset();
        }

        connected_ = false;

        if (config_.onDisconnected) {
            config_.onDisconnected();
        }
    }

    P2PRole getRole() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return role_;
    }

    std::string getSessionId() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return sessionId_;
    }

    P2PSessionInfo getSessionInfo() const {
        std::lock_guard<std::mutex> lock(mutex_);

        P2PSessionInfo info;
        info.sessionId = sessionId_;
        info.role = role_;
        info.offer = offer_;
        info.answer = answer_;

        return info;
    }

    bool isConnected() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return connected_;
    }

private:
    void createPeerConnection() {
        // Configure ICE servers
        rtc::Configuration rtcConfig;

        // Add STUN servers
        for (const auto& stunServer : config_.stunServers) {
            rtcConfig.iceServers.push_back(stunServer);
        }

        // Add TURN servers
        for (const auto& turnServer : config_.turnServers) {
            std::string turnUrl = turnServer.url;
            if (!turnServer.username.empty()) {
                turnUrl += "?username=" + turnServer.username;
            }
            if (!turnServer.password.empty()) {
                turnUrl += "&credential=" + turnServer.password;
            }
            rtcConfig.iceServers.push_back(turnUrl);
        }

        // Create PeerConnection
        peerConnection_ = std::make_shared<rtc::PeerConnection>(rtcConfig);

        // Set up ICE candidate callback
        peerConnection_->onLocalCandidate(
            [this](rtc::Candidate candidate) {
                if (config_.onIceCandidate) {
                    config_.onIceCandidate(candidate.candidate(),
                                           candidate.mid());
                }
            });

        // Set up state change callback
        peerConnection_->onStateChange(
            [this](rtc::PeerConnection::State state) {
                if (state == rtc::PeerConnection::State::Connected) {
                    connected_ = true;
                    if (config_.onConnected) {
                        config_.onConnected();
                    }
                } else if (state ==
                           rtc::PeerConnection::State::Disconnected ||
                           state == rtc::PeerConnection::State::Failed ||
                           state == rtc::PeerConnection::State::Closed) {
                    connected_ = false;
                    if (config_.onDisconnected) {
                        config_.onDisconnected();
                    }
                }
            });
    }

    std::string roleToString(P2PRole role) const {
        switch (role) {
            case P2PRole::Host:
                return "Host";
            case P2PRole::Client:
                return "Client";
            default:
                return "None";
        }
    }

    P2PConnectionConfig config_;
    P2PRole role_;
    std::string sessionId_;
    std::string offer_;
    std::string answer_;
    bool connected_;
    std::shared_ptr<rtc::PeerConnection> peerConnection_;
    mutable std::mutex mutex_;
};

// P2PConnection public interface implementation

P2PConnection::P2PConnection(const P2PConnectionConfig& config)
    : impl_(std::make_unique<Impl>(config)) {
}

P2PConnection::~P2PConnection() = default;

std::string P2PConnection::generateSessionId() {
    return impl_->generateSessionId();
}

void P2PConnection::initializeAsHost() {
    impl_->initializeAsHost();
}

void P2PConnection::initializeAsClient(const std::string& sessionId) {
    impl_->initializeAsClient(sessionId);
}

std::string P2PConnection::createOffer() {
    return impl_->createOffer();
}

void P2PConnection::setRemoteAnswer(const std::string& answer) {
    impl_->setRemoteAnswer(answer);
}

std::string P2PConnection::setRemoteOffer(const std::string& offer) {
    return impl_->setRemoteOffer(offer);
}

void P2PConnection::addRemoteIceCandidate(const std::string& candidate,
                                          const std::string& mid) {
    impl_->addRemoteIceCandidate(candidate, mid);
}

void P2PConnection::disconnect() {
    impl_->disconnect();
}

P2PRole P2PConnection::getRole() const {
    return impl_->getRole();
}

std::string P2PConnection::getSessionId() const {
    return impl_->getSessionId();
}

P2PSessionInfo P2PConnection::getSessionInfo() const {
    return impl_->getSessionInfo();
}

bool P2PConnection::isConnected() const {
    return impl_->isConnected();
}

}  // namespace core
}  // namespace obswebrtc
