/**
 * @file http-client.hpp
 * @brief HTTP client utility for WHIP/WHEP protocols
 *
 * This class provides a shared HTTP client implementation for both
 * WHIPClient and WHEPClient to reduce code duplication.
 */

#pragma once

#include <map>
#include <string>

namespace obswebrtc {
namespace core {

/**
 * @brief HTTP Request structure
 */
struct HTTPRequest {
    std::map<std::string, std::string> headers;
    std::string body;
    std::string contentType;
};

/**
 * @brief HTTP Response structure
 */
struct HTTPResponse {
    int statusCode = 0;
    std::map<std::string, std::string> headers;
    std::string body;
};

/**
 * @brief HTTP client utility class
 *
 * Provides static methods for making HTTP requests.
 * Currently implemented as a stub for testing.
 * In production, replace with actual HTTP client library (e.g., libcurl).
 */
class HTTPClient {
public:
    /**
     * @brief Send HTTP POST request
     * @param url Target URL
     * @param request Request data including headers and body
     * @return HTTP response
     * @throws std::runtime_error on network errors
     */
    static HTTPResponse post(const std::string& url, const HTTPRequest& request);

    /**
     * @brief Send HTTP PATCH request
     * @param url Target URL
     * @param request Request data including headers and body
     * @return HTTP response
     * @throws std::runtime_error on network errors
     */
    static HTTPResponse patch(const std::string& url, const HTTPRequest& request);

    /**
     * @brief Send HTTP DELETE request
     * @param url Target URL
     * @param request Request data including headers
     * @return HTTP response
     * @throws std::runtime_error on network errors
     */
    static HTTPResponse del(const std::string& url, const HTTPRequest& request);
};

}  // namespace core
}  // namespace obswebrtc
