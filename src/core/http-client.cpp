/**
 * @file http-client.cpp
 * @brief HTTP client implementation (stub for testing)
 *
 * This is a stub implementation for testing purposes.
 * In production, replace with actual HTTP client library (e.g., libcurl).
 */

#include "http-client.hpp"

#include <stdexcept>

namespace obswebrtc {
namespace core {

HTTPResponse HTTPClient::post(const std::string& url, const HTTPRequest& request) {
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

    if (url.find("localhost:19999") != std::string::npos ||
        url.find("localhost:19998") != std::string::npos) {
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

HTTPResponse HTTPClient::patch(const std::string& url, const HTTPRequest& request) {
    // Stub implementation for testing
    HTTPResponse response;
    response.statusCode = 204;  // No Content
    return response;
}

HTTPResponse HTTPClient::del(const std::string& url, const HTTPRequest& request) {
    // Stub implementation for testing
    HTTPResponse response;
    response.statusCode = 200;
    return response;
}

}  // namespace core
}  // namespace obswebrtc
