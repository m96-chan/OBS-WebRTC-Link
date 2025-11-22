/**
 * @file peer-connection.cpp
 * @brief Implementation of WebRTC PeerConnection wrapper
 */

#include "peer-connection.hpp"
#include <mutex>
#include <stdexcept>

namespace obswebrtc {
namespace core {

/**
 * @brief Private implementation (PIMPL pattern)
 */
class PeerConnection::Impl {
public:
    explicit Impl(const PeerConnectionConfig& config)
        : config_(config)
        , state_(ConnectionState::New)
    {
        try {
            // Configure libdatachannel
            rtc::Configuration rtcConfig;

            // Add ICE servers
            for (const auto& server : config.iceServers) {
                rtcConfig.iceServers.emplace_back(server);
            }

            // Create PeerConnection
            peerConnection_ = std::make_shared<rtc::PeerConnection>(rtcConfig);

            // Set up callbacks
            setupCallbacks();

            log(LogLevel::Info, "PeerConnection created successfully");
        } catch (const std::exception& e) {
            log(LogLevel::Error, std::string("Failed to create PeerConnection: ") + e.what());
            throw std::runtime_error(std::string("PeerConnection creation failed: ") + e.what());
        }
    }

    ~Impl() {
        try {
            close();
        } catch (...) {
            // Ignore exceptions in destructor
        }
    }

    void createOffer() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            throw std::runtime_error("PeerConnection is closed");
        }

        try {
            log(LogLevel::Info, "Creating offer");

            // Set local description callback
            peerConnection_->onLocalDescription([this](rtc::Description description) {
                handleLocalDescription(description);
            });

            // Gather local candidates
            peerConnection_->onLocalCandidate([this](rtc::Candidate candidate) {
                handleLocalCandidate(candidate);
            });

            // Create a data channel to trigger negotiation
            // libdatachannel requires creating a data channel or media track to initiate SDP generation
            auto dc = peerConnection_->createDataChannel("negotiation");

            log(LogLevel::Debug, "Offer creation initiated");
        } catch (const std::exception& e) {
            log(LogLevel::Error, std::string("Failed to create offer: ") + e.what());
            throw std::runtime_error(std::string("Failed to create offer: ") + e.what());
        }
    }

    void createAnswer() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            throw std::runtime_error("PeerConnection is closed");
        }

        try {
            log(LogLevel::Info, "Creating answer");

            // Set local description callback
            peerConnection_->onLocalDescription([this](rtc::Description description) {
                handleLocalDescription(description);
            });

            // Gather local candidates
            peerConnection_->onLocalCandidate([this](rtc::Candidate candidate) {
                handleLocalCandidate(candidate);
            });

            // Set up data channel handler to accept incoming data channels from remote offer
            // This triggers the answer generation in libdatachannel
            peerConnection_->onDataChannel([](std::shared_ptr<rtc::DataChannel> dc) {
                // Accept the data channel from the remote peer
                // The answer will be generated automatically
            });

            log(LogLevel::Debug, "Answer creation initiated");
        } catch (const std::exception& e) {
            log(LogLevel::Error, std::string("Failed to create answer: ") + e.what());
            throw std::runtime_error(std::string("Failed to create answer: ") + e.what());
        }
    }

    void setRemoteDescription(SdpType type, const std::string& sdp) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            throw std::runtime_error("PeerConnection is closed");
        }

        try {
            log(LogLevel::Info, "Setting remote description");

            rtc::Description::Type rtcType = (type == SdpType::Offer)
                ? rtc::Description::Type::Offer
                : rtc::Description::Type::Answer;

            rtc::Description description(sdp, rtcType);
            peerConnection_->setRemoteDescription(description);

            log(LogLevel::Debug, "Remote description set successfully");
        } catch (const std::exception& e) {
            log(LogLevel::Error, std::string("Failed to set remote description: ") + e.what());
            throw std::runtime_error(std::string("Failed to set remote description: ") + e.what());
        }
    }

    void addIceCandidate(const std::string& candidate, const std::string& mid) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            throw std::runtime_error("PeerConnection is closed");
        }

        try {
            log(LogLevel::Debug, "Adding ICE candidate");

            rtc::Candidate rtcCandidate(candidate, mid);
            peerConnection_->addRemoteCandidate(rtcCandidate);

            log(LogLevel::Debug, "ICE candidate added successfully");
        } catch (const std::exception& e) {
            log(LogLevel::Error, std::string("Failed to add ICE candidate: ") + e.what());
            throw std::runtime_error(std::string("Failed to add ICE candidate: ") + e.what());
        }
    }

    ConnectionState getState() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_;
    }

    bool isConnected() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return state_ == ConnectionState::Connected || state_ == ConnectionState::Completed;
    }

    void close() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (peerConnection_) {
            log(LogLevel::Info, "Closing PeerConnection");

            try {
                peerConnection_->close();
                peerConnection_.reset();
                setState(ConnectionState::Closed);
            } catch (const std::exception& e) {
                log(LogLevel::Error, std::string("Error during close: ") + e.what());
            }
        }
    }

    std::string getLocalDescription() const {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            return "";
        }

        try {
            auto desc = peerConnection_->localDescription();
            if (desc.has_value()) {
                return std::string(*desc);
            }
        } catch (...) {
            // Ignore exceptions
        }

        return "";
    }

    std::string getRemoteDescription() const {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            return "";
        }

        try {
            auto desc = peerConnection_->remoteDescription();
            if (desc.has_value()) {
                return std::string(*desc);
            }
        } catch (...) {
            // Ignore exceptions
        }

        return "";
    }

private:
    void setupCallbacks() {
        // State change callback
        peerConnection_->onStateChange([this](rtc::PeerConnection::State rtcState) {
            ConnectionState state = mapState(rtcState);
            setState(state);

            std::string stateStr = stateToString(state);
            log(LogLevel::Info, "State changed to: " + stateStr);
        });

        // Gathering state change callback
        peerConnection_->onGatheringStateChange([this](rtc::PeerConnection::GatheringState gatheringState) {
            std::string stateStr;
            switch (gatheringState) {
                case rtc::PeerConnection::GatheringState::New:
                    stateStr = "New";
                    break;
                case rtc::PeerConnection::GatheringState::InProgress:
                    stateStr = "InProgress";
                    break;
                case rtc::PeerConnection::GatheringState::Complete:
                    stateStr = "Complete";
                    break;
            }
            log(LogLevel::Debug, "ICE gathering state: " + stateStr);
        });
    }

    void handleLocalDescription(const rtc::Description& description) {
        log(LogLevel::Info, "Local description generated");

        SdpType type = (description.type() == rtc::Description::Type::Offer)
            ? SdpType::Offer
            : SdpType::Answer;

        std::string sdp = std::string(description);

        if (config_.localDescriptionCallback) {
            config_.localDescriptionCallback(type, sdp);
        }
    }

    void handleLocalCandidate(const rtc::Candidate& candidate) {
        log(LogLevel::Debug, "Local ICE candidate gathered");

        std::string candidateStr = std::string(candidate);
        std::string mid = candidate.mid();

        if (config_.iceCandidateCallback) {
            config_.iceCandidateCallback(candidateStr, mid);
        }
    }

    void setState(ConnectionState newState) {
        state_ = newState;

        if (config_.stateCallback) {
            config_.stateCallback(newState);
        }
    }

    ConnectionState mapState(rtc::PeerConnection::State rtcState) const {
        switch (rtcState) {
            case rtc::PeerConnection::State::New:
                return ConnectionState::New;
            case rtc::PeerConnection::State::Connecting:
                return ConnectionState::Checking;
            case rtc::PeerConnection::State::Connected:
                return ConnectionState::Connected;
            case rtc::PeerConnection::State::Disconnected:
                return ConnectionState::Disconnected;
            case rtc::PeerConnection::State::Failed:
                return ConnectionState::Failed;
            case rtc::PeerConnection::State::Closed:
                return ConnectionState::Closed;
            default:
                return ConnectionState::New;
        }
    }

    std::string stateToString(ConnectionState state) const {
        switch (state) {
            case ConnectionState::New:
                return "New";
            case ConnectionState::Checking:
                return "Checking";
            case ConnectionState::Connected:
                return "Connected";
            case ConnectionState::Completed:
                return "Completed";
            case ConnectionState::Failed:
                return "Failed";
            case ConnectionState::Disconnected:
                return "Disconnected";
            case ConnectionState::Closed:
                return "Closed";
            default:
                return "Unknown";
        }
    }

    void log(LogLevel level, const std::string& message) const {
        if (config_.logCallback) {
            config_.logCallback(level, message);
        }
    }

    PeerConnectionConfig config_;
    std::shared_ptr<rtc::PeerConnection> peerConnection_;
    ConnectionState state_;
    mutable std::mutex mutex_;  // Mutable for const methods
};

// Public interface implementation

PeerConnection::PeerConnection(const PeerConnectionConfig& config)
    : impl_(std::make_unique<Impl>(config))
{
}

PeerConnection::~PeerConnection() = default;

void PeerConnection::createOffer() {
    impl_->createOffer();
}

void PeerConnection::createAnswer() {
    impl_->createAnswer();
}

void PeerConnection::setRemoteDescription(SdpType type, const std::string& sdp) {
    impl_->setRemoteDescription(type, sdp);
}

void PeerConnection::addIceCandidate(const std::string& candidate, const std::string& mid) {
    impl_->addIceCandidate(candidate, mid);
}

ConnectionState PeerConnection::getState() const {
    return impl_->getState();
}

bool PeerConnection::isConnected() const {
    return impl_->isConnected();
}

void PeerConnection::close() {
    impl_->close();
}

std::string PeerConnection::getLocalDescription() const {
    return impl_->getLocalDescription();
}

std::string PeerConnection::getRemoteDescription() const {
    return impl_->getRemoteDescription();
}

} // namespace core
} // namespace obswebrtc
