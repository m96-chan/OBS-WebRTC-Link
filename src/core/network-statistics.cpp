/**
 * @file network-statistics.cpp
 * @brief Network statistics collection and formatting implementation
 */

#include "network-statistics.hpp"
#include "constants.hpp"

#include <cstdio>
#include <iomanip>
#include <sstream>

namespace obswebrtc {
namespace core {

// =============================================================================
// NetworkStatisticsCollector Implementation
// =============================================================================

class NetworkStatisticsCollector::Impl {
public:
    Impl()
        : lastBitrateCalculation_(std::chrono::steady_clock::now()),
          lastFrameRateCalculation_(std::chrono::steady_clock::now()),
          lastBytesSent_(0),
          lastBytesReceived_(0),
          lastFramesReceived_(0) {}

    NetworkStats getCurrentStats() const {
        std::lock_guard<std::mutex> lock(mutex_);

        NetworkStats stats = stats_;

        // Calculate packet loss rate
        uint64_t totalPackets = stats_.packetsReceived + stats_.packetsLost;
        if (totalPackets > 0) {
            stats.packetLossRate = (static_cast<double>(stats_.packetsLost) / totalPackets) * 100.0;
        }

        return stats;
    }

    void recordBytesSent(uint64_t bytes) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.bytesSent += bytes;
    }

    void recordBytesReceived(uint64_t bytes) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.bytesReceived += bytes;
    }

    void recordPacketSent() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.packetsSent++;
    }

    void recordPacketReceived() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.packetsReceived++;
    }

    void recordPacketLost() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.packetsLost++;
    }

    void updateRTT(uint32_t rttMs) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.rttMs = rttMs;
    }

    void updateJitter(double jitterMs) {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.jitterMs = jitterMs;
    }

    void recordFrameSent() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.framesSent++;
    }

    void recordFrameReceived() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.framesReceived++;
    }

    void recordFrameDropped() {
        std::lock_guard<std::mutex> lock(mutex_);
        stats_.framesDropped++;
    }

    void calculateBitrates() {
        std::lock_guard<std::mutex> lock(mutex_);

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastBitrateCalculation_).count();

        if (elapsed > 0) {
            // Calculate send bitrate in kbps
            // bits/ms = kbps because: 1 bit/ms = 1000 bits/s = 1 kbps
            // Formula: (bytes * 8 bits/byte) / elapsed_ms = bits/ms = kbps
            uint64_t bytesSentDelta = stats_.bytesSent - lastBytesSent_;
            stats_.sendBitrateKbps = static_cast<uint32_t>((bytesSentDelta * 8) / elapsed);

            // Calculate receive bitrate in kbps
            uint64_t bytesReceivedDelta = stats_.bytesReceived - lastBytesReceived_;
            stats_.receiveBitrateKbps = static_cast<uint32_t>((bytesReceivedDelta * 8) / elapsed);

            // Update last values
            lastBytesSent_ = stats_.bytesSent;
            lastBytesReceived_ = stats_.bytesReceived;
            lastBitrateCalculation_ = now;
        }
    }

    void calculateFrameRate() {
        std::lock_guard<std::mutex> lock(mutex_);

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastFrameRateCalculation_).count();

        if (elapsed > 0) {
            uint64_t framesReceivedDelta = stats_.framesReceived - lastFramesReceived_;
            stats_.frameRate = (static_cast<double>(framesReceivedDelta) * 1000.0) / elapsed;

            lastFramesReceived_ = stats_.framesReceived;
            lastFrameRateCalculation_ = now;
        }
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);

        stats_ = NetworkStats{};
        lastBytesSent_ = 0;
        lastBytesReceived_ = 0;
        lastFramesReceived_ = 0;
        lastBitrateCalculation_ = std::chrono::steady_clock::now();
        lastFrameRateCalculation_ = std::chrono::steady_clock::now();
    }

    void setStatsCallback(StatsCallback callback) {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = std::move(callback);
    }

    void notifyStatsUpdate() {
        StatsCallback callback;
        NetworkStats stats;

        {
            std::lock_guard<std::mutex> lock(callbackMutex_);
            callback = callback_;
        }

        if (callback) {
            stats = getCurrentStats();
            callback(stats);
        }
    }

private:
    NetworkStats stats_;
    mutable std::mutex mutex_;

    // For bitrate calculation
    std::chrono::steady_clock::time_point lastBitrateCalculation_;
    uint64_t lastBytesSent_;
    uint64_t lastBytesReceived_;

    // For frame rate calculation
    std::chrono::steady_clock::time_point lastFrameRateCalculation_;
    uint64_t lastFramesReceived_;

    // Callback
    StatsCallback callback_;
    std::mutex callbackMutex_;
};

NetworkStatisticsCollector::NetworkStatisticsCollector()
    : impl_(std::make_unique<Impl>()) {}

NetworkStatisticsCollector::~NetworkStatisticsCollector() = default;

NetworkStats NetworkStatisticsCollector::getCurrentStats() const {
    return impl_->getCurrentStats();
}

void NetworkStatisticsCollector::recordBytesSent(uint64_t bytes) {
    impl_->recordBytesSent(bytes);
}

void NetworkStatisticsCollector::recordBytesReceived(uint64_t bytes) {
    impl_->recordBytesReceived(bytes);
}

void NetworkStatisticsCollector::recordPacketSent() {
    impl_->recordPacketSent();
}

void NetworkStatisticsCollector::recordPacketReceived() {
    impl_->recordPacketReceived();
}

void NetworkStatisticsCollector::recordPacketLost() {
    impl_->recordPacketLost();
}

void NetworkStatisticsCollector::updateRTT(uint32_t rttMs) {
    impl_->updateRTT(rttMs);
}

void NetworkStatisticsCollector::updateJitter(double jitterMs) {
    impl_->updateJitter(jitterMs);
}

void NetworkStatisticsCollector::recordFrameSent() {
    impl_->recordFrameSent();
}

void NetworkStatisticsCollector::recordFrameReceived() {
    impl_->recordFrameReceived();
}

void NetworkStatisticsCollector::recordFrameDropped() {
    impl_->recordFrameDropped();
}

void NetworkStatisticsCollector::calculateBitrates() {
    impl_->calculateBitrates();
}

void NetworkStatisticsCollector::calculateFrameRate() {
    impl_->calculateFrameRate();
}

void NetworkStatisticsCollector::reset() {
    impl_->reset();
}

void NetworkStatisticsCollector::setStatsCallback(StatsCallback callback) {
    impl_->setStatsCallback(std::move(callback));
}

void NetworkStatisticsCollector::notifyStatsUpdate() {
    impl_->notifyStatsUpdate();
}

// =============================================================================
// NetworkStatisticsFormatter Implementation
// =============================================================================

std::string NetworkStatisticsFormatter::formatStats(const NetworkStats& stats) {
    std::ostringstream oss;

    oss << "Network Statistics:\n";
    oss << "  Send Bitrate: " << formatBitrate(stats.sendBitrateKbps) << "\n";
    oss << "  Receive Bitrate: " << formatBitrate(stats.receiveBitrateKbps) << "\n";
    oss << "  RTT: " << formatRTT(stats.rttMs) << "\n";
    oss << "  Jitter: " << std::fixed << std::setprecision(2) << stats.jitterMs << " ms\n";
    oss << "  Packet Loss: " << formatPacketLoss(stats.packetLossRate) << "\n";
    oss << "  Bytes Sent: " << formatBytes(stats.bytesSent) << "\n";
    oss << "  Bytes Received: " << formatBytes(stats.bytesReceived) << "\n";
    oss << "  Packets Sent: " << stats.packetsSent << "\n";
    oss << "  Packets Received: " << stats.packetsReceived << "\n";
    oss << "  Packets Lost: " << stats.packetsLost << "\n";
    oss << "  Frame Rate: " << std::fixed << std::setprecision(1) << stats.frameRate << " fps\n";
    oss << "  Frames Dropped: " << stats.framesDropped << "\n";

    return oss.str();
}

std::string NetworkStatisticsFormatter::formatBitrate(uint32_t bitrateKbps) {
    using namespace constants;
    std::ostringstream oss;

    if (bitrateKbps >= kKbpsPerMbps) {
        oss << std::fixed << std::setprecision(1)
            << kbpsToMbps(bitrateKbps) << " Mbps";
    } else {
        oss << bitrateKbps << " kbps";
    }

    return oss.str();
}

std::string NetworkStatisticsFormatter::formatBytes(uint64_t bytes) {
    using namespace constants;
    std::ostringstream oss;

    if (bytes >= kBytesPerGB) {
        oss << std::fixed << std::setprecision(1)
            << bytesToGB(bytes) << " GB";
    } else if (bytes >= kBytesPerMB) {
        oss << std::fixed << std::setprecision(1)
            << bytesToMB(bytes) << " MB";
    } else if (bytes >= kBytesPerKB) {
        oss << std::fixed << std::setprecision(1)
            << bytesToKB(bytes) << " KB";
    } else {
        oss << bytes << " B";
    }

    return oss.str();
}

std::string NetworkStatisticsFormatter::formatRTT(uint32_t rttMs) {
    std::ostringstream oss;
    oss << rttMs << " ms";
    return oss.str();
}

std::string NetworkStatisticsFormatter::formatPacketLoss(double lossRate) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << lossRate << "%";
    return oss.str();
}

}  // namespace core
}  // namespace obswebrtc
