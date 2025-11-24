/**
 * @file e2e_flow_test.cpp
 * @brief End-to-end flow tests (publish + subscribe)
 */

#include "core/whep-client.hpp"
#include "core/whip-client.hpp"
#include "helpers/livekit_docker_manager.hpp"
#include "helpers/test_helpers.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace obswebrtc::core;
using namespace obswebrtc::testing;

class E2EFlowTest : public ::testing::Test {
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

TEST_F(E2EFlowTest, PublishAndSubscribeFlow) {
    std::string roomName = "e2e-room-" + generateRandomString();

    std::atomic<bool> publisherConnected{false};
    std::atomic<bool> subscriberConnected{false};

    // Create publisher
    WHIPClientConfig pubConfig;
    pubConfig.url = livekit_->getWhipUrl(roomName);
    pubConfig.bearerToken = livekit_->generateToken(roomName, "pub", true, false);
    pubConfig.onConnected = [&publisherConnected]() { publisherConnected = true; };

    auto publisher = std::make_unique<WHIPClient>(pubConfig);
    publisher->start();

    // Wait for publisher
    ASSERT_TRUE(waitForCondition([&publisherConnected]() { return publisherConnected.load(); },
                                  10000));

    // Create subscriber
    WHEPClientConfig subConfig;
    subConfig.url = livekit_->getWhepUrl(roomName);
    subConfig.bearerToken = livekit_->generateToken(roomName, "sub", false, true);
    subConfig.onConnected = [&subscriberConnected]() { subscriberConnected = true; };

    auto subscriber = std::make_unique<WHEPClient>(subConfig);
    subscriber->start();

    // Wait for subscriber
    EXPECT_TRUE(waitForCondition([&subscriberConnected]() { return subscriberConnected.load(); },
                                  10000));

    publisher->stop();
    subscriber->stop();
}

TEST_F(E2EFlowTest, MultiplePublishersAndSubscribers) {
    std::string roomName = "multi-e2e-" + generateRandomString();

    const int numPublishers = 2;
    const int numSubscribers = 2;

    std::vector<std::unique_ptr<WHIPClient>> publishers;
    std::vector<std::unique_ptr<WHEPClient>> subscribers;
    std::vector<std::atomic<bool>> pubStates(numPublishers);
    std::vector<std::atomic<bool>> subStates(numSubscribers);

    // Create publishers
    for (int i = 0; i < numPublishers; ++i) {
        WHIPClientConfig config;
        config.url = livekit_->getWhipUrl(roomName);
        config.bearerToken =
            livekit_->generateToken(roomName, "pub" + std::to_string(i), true, false);
        config.onConnected = [&pubStates, i]() { pubStates[i] = true; };

        auto pub = std::make_unique<WHIPClient>(config);
        pub->start();
        publishers.push_back(std::move(pub));
    }

    // Wait for all publishers
    bool allPubsConnected = waitForCondition(
        [&pubStates, numPublishers]() {
            for (int i = 0; i < numPublishers; ++i) {
                if (!pubStates[i].load())
                    return false;
            }
            return true;
        },
        15000);

    ASSERT_TRUE(allPubsConnected);

    // Create subscribers
    for (int i = 0; i < numSubscribers; ++i) {
        WHEPClientConfig config;
        config.url = livekit_->getWhepUrl(roomName);
        config.bearerToken =
            livekit_->generateToken(roomName, "sub" + std::to_string(i), false, true);
        config.onConnected = [&subStates, i]() { subStates[i] = true; };

        auto sub = std::make_unique<WHEPClient>(config);
        sub->start();
        subscribers.push_back(std::move(sub));
    }

    // Wait for all subscribers
    bool allSubsConnected = waitForCondition(
        [&subStates, numSubscribers]() {
            for (int i = 0; i < numSubscribers; ++i) {
                if (!subStates[i].load())
                    return false;
            }
            return true;
        },
        15000);

    EXPECT_TRUE(allSubsConnected);

    // Cleanup
    for (auto& pub : publishers)
        pub->stop();
    for (auto& sub : subscribers)
        sub->stop();
}
