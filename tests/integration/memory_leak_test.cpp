/**
 * @file memory_leak_test.cpp
 * @brief Memory leak detection tests
 *
 * Requires AddressSanitizer or Valgrind for accurate detection.
 * Tests exercise components repeatedly to expose potential leaks.
 */

#include "core/peer-connection.hpp"
#include "core/whip-client.hpp"
#include "helpers/livekit_docker_manager.hpp"
#include "helpers/test_helpers.hpp"

#include <gtest/gtest.h>

using namespace obswebrtc::core;
using namespace obswebrtc::testing;

class MemoryLeakTest : public ::testing::Test {
protected:
    void SetUp() override {
        try {
            livekit_ = std::make_unique<LiveKitDockerManager>();
            livekit_->start();
            if (!livekit_->waitForReady(30000)) {
                GTEST_SKIP() << "LiveKit not ready";
            }
        } catch (const std::exception& e) {
            GTEST_SKIP() << "Docker unavailable: " << e.what();
        }
    }

    std::unique_ptr<LiveKitDockerManager> livekit_;
};

TEST_F(MemoryLeakTest, PeerConnectionCreationDeletion) {
    MemoryUsage memBefore = getCurrentMemoryUsage();

    const int iterations = 100;

    for (int i = 0; i < iterations; ++i) {
        PeerConnectionConfig config;
        config.iceServers = {"stun:stun.l.google.com:19302"};
        config.logCallback = [](LogLevel, const std::string&) {};

        auto pc = std::make_unique<PeerConnection>(config);
        pc->createOffer();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pc->close();
    }

    MemoryUsage memAfter = getCurrentMemoryUsage();

    bool noLeak = checkMemoryLeak(memBefore, memAfter, 5 * 1024 * 1024); // 5MB
    EXPECT_TRUE(noLeak) << "Potential leak: RSS increased by "
                        << (memAfter.rss - memBefore.rss) / 1024 << " KB after " << iterations
                        << " iterations";
}

TEST_F(MemoryLeakTest, WhipClientConnectDisconnect) {
    MemoryUsage memBefore = getCurrentMemoryUsage();

    const int iterations = 50;

    for (int i = 0; i < iterations; ++i) {
        std::string roomName = "leak-test-" + std::to_string(i);

        std::atomic<bool> connected{false};

        WHIPClientConfig config;
        config.url = livekit_->getWhipUrl(roomName);
        config.bearerToken = livekit_->generateToken(roomName, "leak", true, false);
        config.onConnected = [&connected]() { connected = true; };

        auto client = std::make_unique<WHIPClient>(config);
        client->start();

        waitForCondition([&connected]() { return connected.load(); }, 5000);

        client->stop();
    }

    MemoryUsage memAfter = getCurrentMemoryUsage();

    bool noLeak = checkMemoryLeak(memBefore, memAfter, 10 * 1024 * 1024); // 10MB
    EXPECT_TRUE(noLeak) << "Potential leak: RSS increased by "
                        << (memAfter.rss - memBefore.rss) / 1024 << " KB after " << iterations
                        << " iterations";
}
