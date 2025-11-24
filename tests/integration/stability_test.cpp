/**
 * @file stability_test.cpp
 * @brief Long-duration stability tests
 */

#include "core/whip-client.hpp"
#include "helpers/livekit_docker_manager.hpp"
#include "helpers/test_helpers.hpp"

#include <gtest/gtest.h>

using namespace obswebrtc::core;
using namespace obswebrtc::testing;

class StabilityTest : public ::testing::Test {
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

TEST_F(StabilityTest, LongDurationConnection) {
    std::string roomName = "stability-room-" + generateRandomString();

    std::atomic<bool> connected{false};
    std::atomic<bool> disconnected{false};

    WHIPClientConfig config;
    config.url = livekit_->getWhipUrl(roomName);
    config.bearerToken = livekit_->generateToken(roomName, "stable", true, false);
    config.onConnected = [&connected]() { connected = true; };
    config.onDisconnected = [&disconnected]() { disconnected = true; };

    auto client = std::make_unique<WHIPClient>(config);
    client->start();

    ASSERT_TRUE(waitForCondition([&connected]() { return connected.load(); }, 10000));

    // Stay connected for 30 seconds
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // Should still be connected
    EXPECT_TRUE(connected.load());
    EXPECT_FALSE(disconnected.load());
    EXPECT_EQ(client->getState(), ConnectionState::Connected);

    client->stop();
}

TEST_F(StabilityTest, RepeatedConnectDisconnect) {
    std::string roomName = "reconnect-room-" + generateRandomString();

    const int iterations = 10;

    for (int i = 0; i < iterations; ++i) {
        std::atomic<bool> connected{false};

        WHIPClientConfig config;
        config.url = livekit_->getWhipUrl(roomName);
        config.bearerToken = livekit_->generateToken(roomName, "iter" + std::to_string(i), true, false);
        config.onConnected = [&connected]() { connected = true; };

        auto client = std::make_unique<WHIPClient>(config);
        client->start();

        ASSERT_TRUE(waitForCondition([&connected]() { return connected.load(); }, 10000))
            << "Iteration " << i << " failed to connect";

        client->stop();

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    SUCCEED() << "Completed " << iterations << " connect/disconnect cycles";
}
