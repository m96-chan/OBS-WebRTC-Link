/**
 * @file test_helpers.hpp
 * @brief Common helper utilities for integration tests
 */

#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace obswebrtc {
namespace testing {

/**
 * @brief Wait for a condition to become true with timeout
 * @param condition Function that returns true when condition is met
 * @param timeoutMs Maximum time to wait in milliseconds
 * @param checkIntervalMs How often to check the condition
 * @return true if condition was met, false if timeout
 */
inline bool waitForCondition(std::function<bool()> condition, int timeoutMs = 5000,
                              int checkIntervalMs = 100) {
    auto startTime = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(timeoutMs);

    while (!condition()) {
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (elapsed >= timeout) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
    }

    return true;
}

/**
 * @brief Generate a random string for test identifiers
 * @param length Length of the string
 * @return Random string
 */
std::string generateRandomString(int length = 8);

/**
 * @brief Get current timestamp as string
 * @return Timestamp string (ISO 8601 format)
 */
std::string getCurrentTimestamp();

/**
 * @brief Check if a port is available for listening
 * @param port Port number
 * @return true if port is available
 */
bool isPortAvailable(int port);

/**
 * @brief Find an available port in a range
 * @param startPort Starting port number
 * @param endPort Ending port number
 * @return Available port number, or 0 if none found
 */
int findAvailablePort(int startPort = 30000, int endPort = 40000);

/**
 * @brief Memory usage information
 */
struct MemoryUsage {
    size_t rss = 0;      // Resident Set Size (physical memory)
    size_t vms = 0;      // Virtual Memory Size
    size_t shared = 0;   // Shared memory
    size_t text = 0;     // Text (code) segment
    size_t data = 0;     // Data segment
};

/**
 * @brief Get current process memory usage
 * @return Memory usage information
 */
MemoryUsage getCurrentMemoryUsage();

/**
 * @brief Check for memory leaks by comparing memory usage before/after
 * @param before Memory usage before operation
 * @param after Memory usage after operation
 * @param thresholdBytes Maximum allowed increase in bytes
 * @return true if no significant leak detected
 */
bool checkMemoryLeak(const MemoryUsage& before, const MemoryUsage& after,
                     size_t thresholdBytes = 1024 * 1024); // 1MB default

/**
 * @brief CPU usage information
 */
struct CpuUsage {
    double userPercent = 0.0;
    double systemPercent = 0.0;
    double totalPercent = 0.0;
};

/**
 * @brief Get current process CPU usage
 * @return CPU usage information
 */
CpuUsage getCurrentCpuUsage();

} // namespace testing
} // namespace obswebrtc
