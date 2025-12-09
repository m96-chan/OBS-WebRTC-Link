/**
 * @file network-statistics.hpp
 * @brief Network statistics collection and formatting for WebRTC connections
 *
 * This module provides:
 * - Real-time network statistics collection
 * - Bitrate, packet loss, RTT, and jitter monitoring
 * - Statistics formatting for display
 * - Thread-safe statistics access
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace obswebrtc {
namespace core {

/**
 * @brief Network statistics data structure
 */
struct NetworkStats {
    // Byte counters
    uint64_t bytesSent = 0;
    uint64_t bytesReceived = 0;

    // Packet counters
    uint64_t packetsSent = 0;
    uint64_t packetsReceived = 0;
    uint64_t packetsLost = 0;
    double packetLossRate = 0.0;  // Percentage (0-100)

    // Latency metrics
    uint32_t rttMs = 0;       // Round-trip time in milliseconds
    double jitterMs = 0.0;    // Jitter in milliseconds

    // Bitrate (calculated)
    uint32_t sendBitrateKbps = 0;
    uint32_t receiveBitrateKbps = 0;

    // Frame statistics
    uint64_t framesSent = 0;
    uint64_t framesReceived = 0;
    uint64_t framesDropped = 0;
    double frameRate = 0.0;  // Frames per second
};

/**
 * @brief Callback type for statistics updates
 */
using StatsCallback = std::function<void(const NetworkStats&)>;

/**
 * @brief Collects and manages network statistics
 *
 * This class provides thread-safe statistics collection for WebRTC connections.
 * Statistics are updated incrementally and can be retrieved at any time.
 */
class NetworkStatisticsCollector {
public:
    /**
     * @brief Construct a new Network Statistics Collector
     */
    NetworkStatisticsCollector();

    /**
     * @brief Destructor
     */
    ~NetworkStatisticsCollector();

    // Delete copy constructor and assignment (non-copyable)
    NetworkStatisticsCollector(const NetworkStatisticsCollector&) = delete;
    NetworkStatisticsCollector& operator=(const NetworkStatisticsCollector&) = delete;

    /**
     * @brief Get current statistics snapshot
     * @return Current network statistics
     */
    NetworkStats getCurrentStats() const;

    /**
     * @brief Record bytes sent
     * @param bytes Number of bytes sent
     */
    void recordBytesSent(uint64_t bytes);

    /**
     * @brief Record bytes received
     * @param bytes Number of bytes received
     */
    void recordBytesReceived(uint64_t bytes);

    /**
     * @brief Record a packet sent
     */
    void recordPacketSent();

    /**
     * @brief Record a packet received
     */
    void recordPacketReceived();

    /**
     * @brief Record a packet lost
     */
    void recordPacketLost();

    /**
     * @brief Update RTT measurement
     * @param rttMs Round-trip time in milliseconds
     */
    void updateRTT(uint32_t rttMs);

    /**
     * @brief Update jitter measurement
     * @param jitterMs Jitter in milliseconds
     */
    void updateJitter(double jitterMs);

    /**
     * @brief Record a frame sent
     */
    void recordFrameSent();

    /**
     * @brief Record a frame received
     */
    void recordFrameReceived();

    /**
     * @brief Record a frame dropped
     */
    void recordFrameDropped();

    /**
     * @brief Calculate current bitrates
     *
     * Call periodically to update bitrate calculations.
     */
    void calculateBitrates();

    /**
     * @brief Calculate current frame rate
     *
     * Call periodically to update frame rate calculation.
     */
    void calculateFrameRate();

    /**
     * @brief Reset all statistics to initial values
     */
    void reset();

    /**
     * @brief Set callback for statistics updates
     * @param callback Function to call when statistics are updated
     */
    void setStatsCallback(StatsCallback callback);

    /**
     * @brief Notify that statistics have been updated
     *
     * This triggers the callback if one is set.
     */
    void notifyStatsUpdate();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Formats network statistics for display
 */
class NetworkStatisticsFormatter {
public:
    /**
     * @brief Format complete statistics to string
     * @param stats Statistics to format
     * @return Formatted string
     */
    static std::string formatStats(const NetworkStats& stats);

    /**
     * @brief Format bitrate for display
     * @param bitrateKbps Bitrate in kbps
     * @return Formatted string (e.g., "2.5 Mbps")
     */
    static std::string formatBitrate(uint32_t bitrateKbps);

    /**
     * @brief Format bytes for display
     * @param bytes Number of bytes
     * @return Formatted string (e.g., "1.5 GB")
     */
    static std::string formatBytes(uint64_t bytes);

    /**
     * @brief Format RTT for display
     * @param rttMs RTT in milliseconds
     * @return Formatted string (e.g., "50 ms")
     */
    static std::string formatRTT(uint32_t rttMs);

    /**
     * @brief Format packet loss for display
     * @param lossRate Packet loss rate as percentage
     * @return Formatted string (e.g., "5.50%")
     */
    static std::string formatPacketLoss(double lossRate);
};

}  // namespace core
}  // namespace obswebrtc
