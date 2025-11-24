/**
 * @file performance_test.cpp
 * @brief Performance and resource usage tests
 */

#include "core/whip-client.hpp"
#include "helpers/livekit_docker_manager.hpp"
#include "helpers/test_helpers.hpp"

#include <gtest/gtest.h>

using namespace obswebrtc::core;
using namespace obswebrtc::testing;

class PerformanceTest : public ::testing::Test {
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

TEST_F(PerformanceTest, ConnectionEstablishmentTime) {
    std::string roomName = "perf-room-" + generateRandomString();

    auto start = std::chrono::steady_clock::now();

    std::atomic<bool> connected{false};
    WHIPClientConfig config;
    config.url = livekit_->getWhipUrl(roomName);
    config.bearerToken = livekit_->generateToken(roomName, "perf", true, false);
    config.onConnected = [&connected]() { connected = true; };

    auto client = std::make_unique<WHIPClient>(config);
    client->start();

    ASSERT_TRUE(waitForCondition([&connected]() { return connected.load(); }, 10000));

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 5000) << "Connection took too long: " << duration.count() << "ms";

    client->stop();
}

TEST_F(PerformanceTest, MemoryUsageStability) {
    MemoryUsage memBefore = getCurrentMemoryUsage();

    for (int i = 0; i < 20; ++i) {
        std::string roomName = "mem-test-" + std::to_string(i);

        std::atomic<bool> connected{false};
        WHIPClientConfig config;
        config.url = livekit_->getWhipUrl(roomName);
        config.bearerToken = livekit_->generateToken(roomName, "mem", true, false);
        config.onConnected = [&connected]() { connected = true; };

        auto client = std::make_unique<WHIPClient>(config);
        client->start();

        waitForCondition([&connected]() { return connected.load(); }, 5000);
        client->stop();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    MemoryUsage memAfter = getCurrentMemoryUsage();

    bool noLeak = checkMemoryLeak(memBefore, memAfter, 20 * 1024 * 1024); // 20MB threshold
    EXPECT_TRUE(noLeak) << "Memory increased by " << (memAfter.rss - memBefore.rss) / 1024
                        << " KB";
}

TEST_F(PerformanceTest, ConcurrentConnectionsScalability) {
    std::string roomName = "scale-room-" + generateRandomString();

    const int numClients = 5;
    std::vector<std::unique_ptr<WHIPClient>> clients;
    std::vector<std::atomic<bool>> states(numClients);

    auto startTime = std::chrono::steady_clock::now();

    for (int i = 0; i < numClients; ++i) {
        WHIPClientConfig config;
        config.url = livekit_->getWhipUrl(roomName);
        config.bearerToken =
            livekit_->generateToken(roomName, "client" + std::to_string(i), true, false);
        config.onConnected = [&states, i]() { states[i] = true; };

        auto client = std::make_unique<WHIPClient>(config);
        client->start();
        clients.push_back(std::move(client));
    }

    bool allConnected = waitForCondition(
        [&states, numClients]() {
            for (int i = 0; i < numClients; ++i) {
                if (!states[i].load())
                    return false;
            }
            return true;
        },
        20000);

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    EXPECT_TRUE(allConnected) << "Not all clients connected";
    EXPECT_LT(duration.count(), 15000) << "Scaling took too long: " << duration.count() << "ms";

    for (auto& client : clients)
        client->stop();
}
