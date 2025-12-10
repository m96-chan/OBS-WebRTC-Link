/**
 * @file network_statistics_test.cpp
 * @brief Unit tests for NetworkStatistics
 */

#include "core/network-statistics.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>
#include <thread>

using namespace obswebrtc::core;
using namespace testing;

/**
 * @brief Test fixture for NetworkStatistics tests
 */
class NetworkStatisticsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed
    }

    void TearDown() override {
        // No teardown needed
    }
};

// =============================================================================
// NetworkStats Structure Tests
// =============================================================================

/**
 * @brief Test default values of NetworkStats
 */
TEST_F(NetworkStatisticsTest, NetworkStatsDefaultValues) {
    NetworkStats stats;

    EXPECT_EQ(stats.bytesSent, 0);
    EXPECT_EQ(stats.bytesReceived, 0);
    EXPECT_EQ(stats.packetsSent, 0);
    EXPECT_EQ(stats.packetsReceived, 0);
    EXPECT_EQ(stats.packetsLost, 0);
    EXPECT_DOUBLE_EQ(stats.packetLossRate, 0.0);
    EXPECT_EQ(stats.rttMs, 0);
    EXPECT_DOUBLE_EQ(stats.jitterMs, 0.0);
    EXPECT_EQ(stats.sendBitrateKbps, 0);
    EXPECT_EQ(stats.receiveBitrateKbps, 0);
    EXPECT_EQ(stats.framesSent, 0);
    EXPECT_EQ(stats.framesReceived, 0);
    EXPECT_EQ(stats.framesDropped, 0);
    EXPECT_DOUBLE_EQ(stats.frameRate, 0.0);
}

// =============================================================================
// NetworkStatisticsCollector Tests
// =============================================================================

/**
 * @brief Test collector construction
 */
TEST_F(NetworkStatisticsTest, CollectorConstruction) {
    EXPECT_NO_THROW({
        NetworkStatisticsCollector collector;
    });
}

/**
 * @brief Test getting current stats
 */
TEST_F(NetworkStatisticsTest, GetCurrentStats) {
    NetworkStatisticsCollector collector;
    NetworkStats stats = collector.getCurrentStats();

    // Initial stats should be all zeros
    EXPECT_EQ(stats.bytesSent, 0);
    EXPECT_EQ(stats.bytesReceived, 0);
}

/**
 * @brief Test recording sent bytes
 */
TEST_F(NetworkStatisticsTest, RecordSentBytes) {
    NetworkStatisticsCollector collector;

    collector.recordBytesSent(1000);
    collector.recordBytesSent(500);

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.bytesSent, 1500);
}

/**
 * @brief Test recording received bytes
 */
TEST_F(NetworkStatisticsTest, RecordReceivedBytes) {
    NetworkStatisticsCollector collector;

    collector.recordBytesReceived(2000);
    collector.recordBytesReceived(800);

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.bytesReceived, 2800);
}

/**
 * @brief Test recording sent packets
 */
TEST_F(NetworkStatisticsTest, RecordSentPackets) {
    NetworkStatisticsCollector collector;

    collector.recordPacketSent();
    collector.recordPacketSent();
    collector.recordPacketSent();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.packetsSent, 3);
}

/**
 * @brief Test recording received packets
 */
TEST_F(NetworkStatisticsTest, RecordReceivedPackets) {
    NetworkStatisticsCollector collector;

    collector.recordPacketReceived();
    collector.recordPacketReceived();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.packetsReceived, 2);
}

/**
 * @brief Test recording lost packets
 */
TEST_F(NetworkStatisticsTest, RecordLostPackets) {
    NetworkStatisticsCollector collector;

    collector.recordPacketLost();
    collector.recordPacketLost();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.packetsLost, 2);
}

/**
 * @brief Test packet loss rate calculation
 */
TEST_F(NetworkStatisticsTest, PacketLossRateCalculation) {
    NetworkStatisticsCollector collector;

    // Receive 90 packets, lose 10
    for (int i = 0; i < 90; i++) {
        collector.recordPacketReceived();
    }
    for (int i = 0; i < 10; i++) {
        collector.recordPacketLost();
    }

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_NEAR(stats.packetLossRate, 10.0, 0.1);  // 10% loss
}

/**
 * @brief Test packet loss rate with no packets
 */
TEST_F(NetworkStatisticsTest, PacketLossRateNoPackets) {
    NetworkStatisticsCollector collector;

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_DOUBLE_EQ(stats.packetLossRate, 0.0);
}

/**
 * @brief Test updating RTT
 */
TEST_F(NetworkStatisticsTest, UpdateRTT) {
    NetworkStatisticsCollector collector;

    collector.updateRTT(50);
    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.rttMs, 50);

    collector.updateRTT(30);
    stats = collector.getCurrentStats();
    EXPECT_EQ(stats.rttMs, 30);
}

/**
 * @brief Test updating jitter
 */
TEST_F(NetworkStatisticsTest, UpdateJitter) {
    NetworkStatisticsCollector collector;

    collector.updateJitter(5.5);
    NetworkStats stats = collector.getCurrentStats();
    EXPECT_DOUBLE_EQ(stats.jitterMs, 5.5);

    collector.updateJitter(3.2);
    stats = collector.getCurrentStats();
    EXPECT_DOUBLE_EQ(stats.jitterMs, 3.2);
}

/**
 * @brief Test recording sent frames
 */
TEST_F(NetworkStatisticsTest, RecordSentFrames) {
    NetworkStatisticsCollector collector;

    collector.recordFrameSent();
    collector.recordFrameSent();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.framesSent, 2);
}

/**
 * @brief Test recording received frames
 */
TEST_F(NetworkStatisticsTest, RecordReceivedFrames) {
    NetworkStatisticsCollector collector;

    collector.recordFrameReceived();
    collector.recordFrameReceived();
    collector.recordFrameReceived();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.framesReceived, 3);
}

/**
 * @brief Test recording dropped frames
 */
TEST_F(NetworkStatisticsTest, RecordDroppedFrames) {
    NetworkStatisticsCollector collector;

    collector.recordFrameDropped();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.framesDropped, 1);
}

/**
 * @brief Test resetting statistics
 */
TEST_F(NetworkStatisticsTest, ResetStatistics) {
    NetworkStatisticsCollector collector;

    collector.recordBytesSent(1000);
    collector.recordBytesReceived(2000);
    collector.recordPacketSent();
    collector.recordPacketReceived();
    collector.recordPacketLost();
    collector.updateRTT(50);
    collector.updateJitter(5.0);

    collector.reset();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.bytesSent, 0);
    EXPECT_EQ(stats.bytesReceived, 0);
    EXPECT_EQ(stats.packetsSent, 0);
    EXPECT_EQ(stats.packetsReceived, 0);
    EXPECT_EQ(stats.packetsLost, 0);
    EXPECT_EQ(stats.rttMs, 0);
    EXPECT_DOUBLE_EQ(stats.jitterMs, 0.0);
}

// =============================================================================
// Statistics Callback Tests
// =============================================================================

/**
 * @brief Test statistics callback invocation
 */
TEST_F(NetworkStatisticsTest, StatisticsCallbackIsInvoked) {
    NetworkStatisticsCollector collector;
    bool callbackInvoked = false;
    NetworkStats receivedStats;

    collector.setStatsCallback([&](const NetworkStats& stats) {
        callbackInvoked = true;
        receivedStats = stats;
    });

    collector.recordBytesSent(1000);
    collector.notifyStatsUpdate();

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(receivedStats.bytesSent, 1000);
}

/**
 * @brief Test clearing statistics callback
 */
TEST_F(NetworkStatisticsTest, ClearStatisticsCallback) {
    NetworkStatisticsCollector collector;
    int callbackCount = 0;

    collector.setStatsCallback([&](const NetworkStats& stats) {
        callbackCount++;
    });

    collector.notifyStatsUpdate();
    EXPECT_EQ(callbackCount, 1);

    collector.setStatsCallback(nullptr);
    collector.notifyStatsUpdate();
    EXPECT_EQ(callbackCount, 1);  // Should not increase
}

// =============================================================================
// Bitrate Calculation Tests
// =============================================================================

/**
 * @brief Test send bitrate calculation
 */
TEST_F(NetworkStatisticsTest, SendBitrateCalculation) {
    NetworkStatisticsCollector collector;

    // Record 125,000 bytes in ~100ms
    // Expected bitrate: (125000 * 8) / 100 = 10,000 kbps = 10 Mbps
    // (because bits/ms = kbps: 1 bit/ms = 1000 bits/s = 1 kbps)
    collector.recordBytesSent(125000);

    // Wait a bit and calculate bitrate
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    collector.calculateBitrates();
    NetworkStats stats = collector.getCurrentStats();

    // Bitrate should be approximately 10,000 kbps (10 Mbps)
    // Allow wide range due to timing variance: 5,000 - 20,000 kbps
    EXPECT_GT(stats.sendBitrateKbps, 5000);
    EXPECT_LT(stats.sendBitrateKbps, 20000);
}

/**
 * @brief Test receive bitrate calculation
 */
TEST_F(NetworkStatisticsTest, ReceiveBitrateCalculation) {
    NetworkStatisticsCollector collector;

    // Record 250,000 bytes in ~100ms
    // Expected bitrate: (250000 * 8) / 100 = 20,000 kbps = 20 Mbps
    collector.recordBytesReceived(250000);

    // Wait a bit and calculate bitrate
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    collector.calculateBitrates();
    NetworkStats stats = collector.getCurrentStats();

    // Bitrate should be approximately 20,000 kbps (20 Mbps)
    // Allow wide range due to timing variance on CI: 5,000 - 80,000 kbps
    EXPECT_GT(stats.receiveBitrateKbps, 5000);
    EXPECT_LT(stats.receiveBitrateKbps, 80000);
}

// =============================================================================
// Frame Rate Calculation Tests
// =============================================================================

/**
 * @brief Test frame rate calculation
 */
TEST_F(NetworkStatisticsTest, FrameRateCalculation) {
    NetworkStatisticsCollector collector;

    // Record 30 frames
    for (int i = 0; i < 30; i++) {
        collector.recordFrameReceived();
    }

    // Wait a bit and calculate frame rate
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    collector.calculateFrameRate();
    NetworkStats stats = collector.getCurrentStats();

    // Frame rate should be positive
    EXPECT_GT(stats.frameRate, 0.0);
}

// =============================================================================
// Thread Safety Tests
// =============================================================================

/**
 * @brief Test concurrent access to statistics
 */
TEST_F(NetworkStatisticsTest, ConcurrentAccess) {
    NetworkStatisticsCollector collector;
    std::atomic<bool> running{true};
    const int iterations = 100;

    // Writer thread
    std::thread writer([&]() {
        for (int i = 0; i < iterations && running; i++) {
            collector.recordBytesSent(100);
            collector.recordBytesReceived(100);
            collector.recordPacketSent();
            collector.recordPacketReceived();
        }
    });

    // Reader thread
    std::thread reader([&]() {
        for (int i = 0; i < iterations && running; i++) {
            NetworkStats stats = collector.getCurrentStats();
            // Just verify we can read without crash
            (void)stats.bytesSent;
        }
    });

    writer.join();
    running = false;
    reader.join();

    NetworkStats stats = collector.getCurrentStats();
    EXPECT_EQ(stats.bytesSent, iterations * 100);
    EXPECT_EQ(stats.bytesReceived, iterations * 100);
}

// =============================================================================
// NetworkStatisticsFormatter Tests
// =============================================================================

/**
 * @brief Test formatting stats to string
 */
TEST_F(NetworkStatisticsTest, FormatStatsToString) {
    NetworkStats stats;
    stats.bytesSent = 1000000;
    stats.bytesReceived = 2000000;
    stats.sendBitrateKbps = 2500;
    stats.receiveBitrateKbps = 5000;
    stats.rttMs = 50;
    stats.packetLossRate = 1.5;

    std::string formatted = NetworkStatisticsFormatter::formatStats(stats);

    EXPECT_FALSE(formatted.empty());
    EXPECT_NE(formatted.find("2.5 Mbps"), std::string::npos);  // Send bitrate
    EXPECT_NE(formatted.find("5.0 Mbps"), std::string::npos);  // Receive bitrate
    EXPECT_NE(formatted.find("50 ms"), std::string::npos);     // RTT
}

/**
 * @brief Test formatting bitrate
 */
TEST_F(NetworkStatisticsTest, FormatBitrate) {
    EXPECT_EQ(NetworkStatisticsFormatter::formatBitrate(500), "500 kbps");
    EXPECT_EQ(NetworkStatisticsFormatter::formatBitrate(1500), "1.5 Mbps");
    EXPECT_EQ(NetworkStatisticsFormatter::formatBitrate(10000), "10.0 Mbps");
}

/**
 * @brief Test formatting bytes
 */
TEST_F(NetworkStatisticsTest, FormatBytes) {
    EXPECT_EQ(NetworkStatisticsFormatter::formatBytes(500), "500 B");
    EXPECT_EQ(NetworkStatisticsFormatter::formatBytes(1500), "1.5 KB");
    EXPECT_EQ(NetworkStatisticsFormatter::formatBytes(1500000), "1.5 MB");
    EXPECT_EQ(NetworkStatisticsFormatter::formatBytes(1500000000), "1.5 GB");
}

/**
 * @brief Test formatting RTT
 */
TEST_F(NetworkStatisticsTest, FormatRTT) {
    EXPECT_EQ(NetworkStatisticsFormatter::formatRTT(50), "50 ms");
    EXPECT_EQ(NetworkStatisticsFormatter::formatRTT(0), "0 ms");
}

/**
 * @brief Test formatting packet loss
 */
TEST_F(NetworkStatisticsTest, FormatPacketLoss) {
    EXPECT_EQ(NetworkStatisticsFormatter::formatPacketLoss(0.0), "0.00%");
    EXPECT_EQ(NetworkStatisticsFormatter::formatPacketLoss(5.5), "5.50%");
    EXPECT_EQ(NetworkStatisticsFormatter::formatPacketLoss(100.0), "100.00%");
}
