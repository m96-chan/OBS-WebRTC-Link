/**
 * @file peer-connection.cpp
 * @brief Implementation of WebRTC PeerConnection wrapper
 */

#include "peer-connection.hpp"

#include <mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace obswebrtc {
namespace core {

/**
 * @brief Private implementation (PIMPL pattern)
 */
class PeerConnection::Impl {
public:
    explicit Impl(const PeerConnectionConfig& config)
        : config_(config), state_(ConnectionState::New), hasRemoteDescription_(false),
          remoteDescriptionSdp_(""), pendingCandidates_(), offerCount_(0) {
        try {
            // Configure libdatachannel
            rtc::Configuration rtcConfig;

            // Add ICE servers
            for (const auto& server : config.iceServers) {
                rtcConfig.iceServers.emplace_back(server);
            }

            // Enable automatic renegotiation for proper handling of multiple offers
            rtcConfig.disableAutoNegotiation = false;

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
        std::shared_ptr<rtc::PeerConnection> pc;
        bool isRenegotiation = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (!peerConnection_) {
                return;  // NoOp if closed
            }

            pc = peerConnection_;

            // Check if this is a renegotiation (not the first offer)
            isRenegotiation = (offerCount_ > 0);
            offerCount_++;
        }

        try {
            if (isRenegotiation) {
                log(LogLevel::Info, "Creating renegotiation offer");
            } else {
                log(LogLevel::Info, "Creating initial offer");
            }

            // Create a data channel to trigger negotiation
            // libdatachannel requires creating a data channel or media track to initiate SDP generation
            // We must keep a reference to the data channel, otherwise it will be destroyed immediately
            // Use a unique label for each offer to ensure renegotiation works
            // Note: We release the mutex before creating the data channel to avoid potential deadlocks
            // if libdatachannel calls our callbacks synchronously
            int currentOfferCount;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                currentOfferCount = offerCount_;
            }

            auto dc = pc->createDataChannel("negotiation-" + std::to_string(currentOfferCount));

            {
                std::lock_guard<std::mutex> lock(mutex_);
                // Store the new data channel
                if (isRenegotiation) {
                    // For renegotiation, keep both old and new channels
                    additionalDataChannels_.push_back(dc);
                } else {
                    // For initial offer, use the main data channel
                    dataChannel_ = dc;
                }
            }

            // Trigger local description generation
            // This will invoke the onLocalDescription callback
            // Both initial and renegotiation use the same approach:
            // creating a new data channel automatically triggers negotiationNeeded,
            // and setLocalDescription will generate a new offer including all channels
            pc->setLocalDescription(rtc::Description::Type::Offer);

            log(LogLevel::Debug, "Offer creation initiated");
        } catch (const std::exception& e) {
            log(LogLevel::Error, std::string("Failed to create offer: ") + e.what());
            throw std::runtime_error(std::string("Failed to create offer: ") + e.what());
        }
    }

    void createAnswer() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            return;  // NoOp if closed
        }

        if (!hasRemoteDescription_) {
            throw std::runtime_error("Cannot create answer without remote offer");
        }

        // For libdatachannel, the answer is automatically generated when setRemoteDescription
        // is called with an offer AND the onDataChannel callback is set.
        // This method exists for API compatibility, but the actual answer generation
        // happens in setRemoteDescription().
        log(LogLevel::Info, "Answer will be generated automatically by libdatachannel");
    }

    void setRemoteDescription(SdpType type, const std::string& sdp) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            return;  // NoOp if closed
        }

        try {
            log(LogLevel::Info, "Setting remote description");

            rtc::Description::Type rtcType = (type == SdpType::Offer)
                ? rtc::Description::Type::Offer
                : rtc::Description::Type::Answer;

            rtc::Description description(sdp, rtcType);
            peerConnection_->setRemoteDescription(description);

            hasRemoteDescription_ = true;
            remoteDescriptionSdp_ = sdp;  // Store original SDP

            // Add any buffered ICE candidates now that we have a remote description
            for (const auto& pending : pendingCandidates_) {
                try {
                    log(LogLevel::Debug, "Adding buffered ICE candidate");
                    rtc::Candidate rtcCandidate(pending.first, pending.second);
                    peerConnection_->addRemoteCandidate(rtcCandidate);
                } catch (const std::exception& e) {
                    log(LogLevel::Warning, std::string("Failed to add buffered candidate: ") + e.what());
                }
            }
            pendingCandidates_.clear();

            log(LogLevel::Debug, "Remote description set successfully");
        } catch (const std::exception& e) {
            log(LogLevel::Error, std::string("Failed to set remote description: ") + e.what());
            throw std::runtime_error(std::string("Failed to set remote description: ") + e.what());
        }
    }

    void addIceCandidate(const std::string& candidate, const std::string& mid) {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            return;  // NoOp if closed
        }

        // Validate candidate format first
        if (candidate.empty()) {
            throw std::runtime_error("ICE candidate cannot be empty");
        }

        // Basic validation: candidate string should start with "candidate:"
        if (candidate.find("candidate:") == std::string::npos) {
            throw std::runtime_error("Invalid ICE candidate format");
        }

        // Buffer candidates if remote description hasn't been set yet
        if (!hasRemoteDescription_) {
            log(LogLevel::Debug, "Buffering ICE candidate (no remote description yet)");
            pendingCandidates_.push_back({candidate, mid});
            return;
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
                // Close and clear all data channels
                if (dataChannel_) {
                    dataChannel_->close();
                    dataChannel_.reset();
                }

                for (auto& dc : additionalDataChannels_) {
                    if (dc) {
                        dc->close();
                    }
                }
                additionalDataChannels_.clear();

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
        return remoteDescriptionSdp_;
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

        // Set local description callback - must be set before any negotiation
        peerConnection_->onLocalDescription(
            [this](rtc::Description description) { handleLocalDescription(description); });

        // Gather local candidates
        peerConnection_->onLocalCandidate(
            [this](rtc::Candidate candidate) { handleLocalCandidate(candidate); });

        // Set up data channel handler - must be set before setRemoteDescription
        peerConnection_->onDataChannel([this](std::shared_ptr<rtc::DataChannel> dc) {
            // Accept the data channel from the remote peer
            // The answer will be generated automatically
            // Keep a reference to prevent it from being destroyed
            dataChannel_ = dc;
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
    std::shared_ptr<rtc::DataChannel> dataChannel_;  // Keep reference to data channel
    std::vector<std::shared_ptr<rtc::DataChannel>> additionalDataChannels_;  // Additional data channels for renegotiation
    ConnectionState state_;
    bool hasRemoteDescription_;
    std::string remoteDescriptionSdp_;
    std::vector<std::pair<std::string, std::string>> pendingCandidates_;  // Buffered candidates
    int offerCount_;  // Track number of offers for renegotiation detection
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

}  // namespace core
}  // namespace obswebrtc
