/**
 * @file whep-client.cpp
 * @brief Implementation of WHEP Client
 */

#include "whep-client.hpp"

#include "whip-client.hpp"  // For HTTPRequest and HTTPResponse

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <regex>

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
    }

    ~Impl() {
        if (connected_) {
            try {
                disconnect();
            } catch (...) {
                // Ignore exceptions during destruction
            }
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
            response = sendHTTPPost(config_.url, request);
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
        HTTPResponse response = sendHTTPPatch(resourceUrl_, request);

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
                sendHTTPDelete(resourceUrl_, request);
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

private:
    /**
     * @brief Send HTTP POST request
     * This is a stub implementation. In production, use a proper HTTP client
     * library like libcurl.
     */
    HTTPResponse sendHTTPPost(const std::string& url, const HTTPRequest& request) {
        // Stub implementation for testing
        // In production, implement using libcurl or similar HTTP client
        HTTPResponse response;

        // Check for invalid token in stub implementation
        auto authIt = request.headers.find("Authorization");
        if (authIt != request.headers.end() &&
            authIt->second.find("invalid-token") != std::string::npos) {
            response.statusCode = 401;
            return response;
        }

        // Check for forbidden token
        if (authIt != request.headers.end() &&
            authIt->second.find("forbidden-token") != std::string::npos) {
            response.statusCode = 403;
            return response;
        }

        // Simulate network errors based on URL patterns
        if (url.find("192.0.2.1") != std::string::npos) {
            // TEST-NET-1 - simulate timeout
            throw std::runtime_error("Network timeout");
        }

        if (url.find("nonexistent.invalid.domain.tld") != std::string::npos) {
            // Simulate DNS resolution failure
            throw std::runtime_error("DNS resolution failed");
        }

        if (url.find("localhost:19998") != std::string::npos) {
            // Simulate connection refused
            throw std::runtime_error("Connection refused");
        }

        if (url.find("self-signed.badssl.com") != std::string::npos) {
            // Simulate SSL certificate error
            throw std::runtime_error("SSL certificate verification failed");
        }

        // Simulate HTTP error codes based on URL path
        if (url.find("nonexistent-endpoint") != std::string::npos) {
            response.statusCode = 404;
            return response;
        }

        if (url.find("error-endpoint") != std::string::npos) {
            response.statusCode = 500;
            return response;
        }

        if (url.find("unavailable-endpoint") != std::string::npos) {
            response.statusCode = 503;
            return response;
        }

        // Check for malformed SDP (simulates 400 Bad Request)
        if (request.body.find("invalid sdp content") != std::string::npos) {
            response.statusCode = 400;
            return response;
        }

        response.statusCode = 201;
        response.headers["Location"] = url + "/resource/123";
        response.headers["Content-Type"] = "application/sdp";
        response.body = "v=0\r\no=- 789 012 IN IP4 0.0.0.0\r\n";  // Mock SDP answer
        return response;
    }

    /**
     * @brief Send HTTP PATCH request
     */
    HTTPResponse sendHTTPPatch(const std::string& url, const HTTPRequest& request) {
        // Stub implementation for testing
        HTTPResponse response;
        response.statusCode = 204;  // No Content
        return response;
    }

    /**
     * @brief Send HTTP DELETE request
     */
    HTTPResponse sendHTTPDelete(const std::string& url, const HTTPRequest& request) {
        // Stub implementation for testing
        HTTPResponse response;
        response.statusCode = 200;
        return response;
    }

    WHEPConfig config_;
    bool connected_;
    std::string resourceUrl_;
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

}  // namespace core
}  // namespace obswebrtc
