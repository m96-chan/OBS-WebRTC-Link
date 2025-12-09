/**
 * @file whep-client.cpp
 * @brief Implementation of WHEP Client
 */

#include "whep-client.hpp"
#include "http-client.hpp"
#include "peer-connection.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <regex>
#include <mutex>

namespace obswebrtc {
namespace core {

using json = nlohmann::json;

/**
 * @brief Validate URL format
 */
static bool isValidUrl(const std::string& url) {
    // Check if URL has a valid scheme (http:// or https://)
    std::regex urlPattern("^https?://[^\\s/$.?#].[^\\s]*$", std::regex::icase);
    return std::regex_match(url, urlPattern);
}

/**
 * @brief Internal implementation of WHEPClient
 */
class WHEPClient::Impl {
public:
    explicit Impl(const WHEPConfig& config)
        : config_(config)
        , connected_(false) {
        // Validate configuration
        if (config_.url.empty()) {
            throw std::invalid_argument("WHEP URL cannot be empty");
        }

        // Validate URL format
        if (!isValidUrl(config_.url)) {
            throw std::invalid_argument("Invalid URL format. URL must start with http:// or https://");
        }

        // Create internal PeerConnection if frame callbacks are set
        if (config_.videoFrameCallback || config_.audioFrameCallback) {
            initPeerConnection();
        }
    }

    ~Impl() {
        if (connected_) {
            try {
                disconnect();
            } catch (...) {
                // Ignore exceptions during destruction
            }
        }

        // Clean up PeerConnection
        if (peerConnection_) {
            try {
                peerConnection_->close();
            } catch (...) {
                // Ignore exceptions during destruction
            }
            peerConnection_.reset();
        }
    }

    std::string sendOffer(const std::string& sdp) {
        if (sdp.empty()) {
            throw std::invalid_argument("SDP offer cannot be empty");
        }

        // Prepare HTTP POST request
        HTTPRequest request;
        request.contentType = "application/sdp";
        request.body = sdp;

        // Add bearer token if provided
        if (!config_.bearerToken.empty()) {
            request.headers["Authorization"] = "Bearer " + config_.bearerToken;
        }

        // Add required WHEP headers
        request.headers["Content-Type"] = "application/sdp";

        HTTPResponse response;
        try {
            // Send POST request (in real implementation, use HTTP client library)
            response = HTTPClient::post(config_.url, request);
        } catch (const std::exception& e) {
            // Handle network errors (timeout, DNS, connection refused, SSL)
            if (config_.onError) {
                config_.onError("Network error: " + std::string(e.what()));
            }
            throw;
        }

        // Check response status
        if (response.statusCode == 401) {
            if (config_.onError) {
                config_.onError("Unauthorized: Invalid bearer token");
            }
            throw std::runtime_error("Unauthorized: Invalid bearer token");
        }

        if (response.statusCode < 200 || response.statusCode >= 300) {
            if (config_.onError) {
                config_.onError("WHEP server returned error: " +
                                std::to_string(response.statusCode));
            }
            throw std::runtime_error("WHEP server returned error: " +
                                     std::to_string(response.statusCode));
        }

        // Extract Location header for resource URL
        auto locationIt = response.headers.find("Location");
        if (locationIt != response.headers.end()) {
            resourceUrl_ = locationIt->second;
            connected_ = true;

            if (config_.onConnected) {
                config_.onConnected();
            }
        }

        // Return SDP answer from response body
        return response.body;
    }

    void sendIceCandidate(const std::string& candidate, const std::string& mid) {
        if (!connected_) {
            throw std::runtime_error("Not connected to WHEP server");
        }

        if (resourceUrl_.empty()) {
            throw std::runtime_error("No resource URL available");
        }

        // Validate ICE candidate
        if (candidate.empty()) {
            throw std::invalid_argument("ICE candidate cannot be empty");
        }

        // Prepare ICE candidate in Trickle ICE format
        // (application/trickle-ice-sdpfrag)
        std::string iceSdpFrag = "a=" + candidate;

        HTTPRequest request;
        request.contentType = "application/trickle-ice-sdpfrag";
        request.body = iceSdpFrag;

        // Add bearer token if provided
        if (!config_.bearerToken.empty()) {
            request.headers["Authorization"] = "Bearer " + config_.bearerToken;
        }

        request.headers["Content-Type"] = "application/trickle-ice-sdpfrag";

        // Send PATCH request to resource URL
        HTTPResponse response = HTTPClient::patch(resourceUrl_, request);

        if (response.statusCode < 200 || response.statusCode >= 300) {
            if (config_.onError) {
                config_.onError("Failed to send ICE candidate: " +
                                std::to_string(response.statusCode));
            }
            throw std::runtime_error("Failed to send ICE candidate");
        }
    }

    void disconnect() {
        if (!connected_) {
            return;
        }

        if (!resourceUrl_.empty()) {
            // Send DELETE request to resource URL
            HTTPRequest request;

            // Add bearer token if provided
            if (!config_.bearerToken.empty()) {
                request.headers["Authorization"] = "Bearer " + config_.bearerToken;
            }

            try {
                HTTPClient::del(resourceUrl_, request);
            } catch (const std::exception& e) {
                if (config_.onError) {
                    config_.onError("Error during disconnect: " + std::string(e.what()));
                }
            }
        }

        connected_ = false;
        resourceUrl_.clear();

        if (config_.onDisconnected) {
            config_.onDisconnected();
        }
    }

    bool isConnected() const {
        return connected_;
    }

    bool hasPeerConnection() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return peerConnection_ != nullptr;
    }

    void connect() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!peerConnection_) {
            throw std::runtime_error("PeerConnection not initialized. Set frame callbacks to enable media reception.");
        }

        // Create offer via PeerConnection
        // The offer will be sent to the WHEP server via localDescriptionCallback
        peerConnection_->createOffer();
    }

private:
    void initPeerConnection() {
        PeerConnectionConfig pcConfig;

        // Configure ICE servers
        pcConfig.iceServers = config_.iceServers;

        // Set up video frame callback
        pcConfig.videoFrameCallback = config_.videoFrameCallback;

        // Set up audio frame callback
        pcConfig.audioFrameCallback = config_.audioFrameCallback;

        // Set up local description callback to send offer to WHEP server
        pcConfig.localDescriptionCallback = [this](SdpType type, const std::string& sdp) {
            handleLocalDescription(type, sdp);
        };

        // Set up ICE candidate callback to send candidates to WHEP server
        pcConfig.iceCandidateCallback = [this](const std::string& candidate, const std::string& mid) {
            handleLocalIceCandidate(candidate, mid);
        };

        // Set up state change callback
        pcConfig.stateCallback = [this](ConnectionState state) {
            handleStateChange(state);
        };

        // Set up log callback (optional)
        pcConfig.logCallback = [this](LogLevel level, const std::string& message) {
            // Could be wired to external logging if needed
        };

        peerConnection_ = std::make_unique<PeerConnection>(pcConfig);
    }

    void handleLocalDescription(SdpType type, const std::string& sdp) {
        if (type != SdpType::Offer) {
            return;  // WHEP client only sends offers
        }

        try {
            // Send offer to WHEP server and get answer
            std::string answer = sendOffer(sdp);

            // Apply answer to PeerConnection
            if (!answer.empty() && peerConnection_) {
                peerConnection_->setRemoteDescription(SdpType::Answer, answer);
            }
        } catch (const std::exception& e) {
            if (config_.onError) {
                config_.onError("Failed to send offer: " + std::string(e.what()));
            }
        }
    }

    void handleLocalIceCandidate(const std::string& candidate, const std::string& mid) {
        if (!connected_) {
            // Buffer candidate until connected
            pendingIceCandidates_.push_back({candidate, mid});
            return;
        }

        try {
            sendIceCandidate(candidate, mid);
        } catch (const std::exception& e) {
            if (config_.onError) {
                config_.onError("Failed to send ICE candidate: " + std::string(e.what()));
            }
        }
    }

    void handleStateChange(ConnectionState state) {
        if (state == ConnectionState::Connected || state == ConnectionState::Completed) {
            // Send any pending ICE candidates
            for (const auto& pending : pendingIceCandidates_) {
                try {
                    sendIceCandidate(pending.first, pending.second);
                } catch (const std::exception& e) {
                    if (config_.onError) {
                        config_.onError("Failed to send buffered ICE candidate: " + std::string(e.what()));
                    }
                }
            }
            pendingIceCandidates_.clear();
        }
    }

    WHEPConfig config_;
    bool connected_;
    std::string resourceUrl_;
    std::unique_ptr<PeerConnection> peerConnection_;
    std::vector<std::pair<std::string, std::string>> pendingIceCandidates_;
    mutable std::mutex mutex_;
};

// WHEPClient implementation

WHEPClient::WHEPClient(const WHEPConfig& config)
    : impl_(std::make_unique<Impl>(config)) {
}

WHEPClient::~WHEPClient() = default;

std::string WHEPClient::sendOffer(const std::string& sdp) {
    return impl_->sendOffer(sdp);
}

void WHEPClient::sendIceCandidate(const std::string& candidate, const std::string& mid) {
    impl_->sendIceCandidate(candidate, mid);
}

void WHEPClient::disconnect() {
    impl_->disconnect();
}

bool WHEPClient::isConnected() const {
    return impl_->isConnected();
}

bool WHEPClient::hasPeerConnection() const {
    return impl_->hasPeerConnection();
}

void WHEPClient::connect() {
    impl_->connect();
}

}  // namespace core
}  // namespace obswebrtc
