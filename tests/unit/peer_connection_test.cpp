/**
 * @file peer_connection_test.cpp
 * @brief Unit tests for PeerConnection class
 */

#include "../../src/core/peer-connection.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <atomic>
#include <chrono>
#include <thread>

using namespace obswebrtc::core;
using ::testing::_;
using ::testing::AtLeast;

class PeerConnectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset shared state
        logMessages.clear();
        stateChanges.clear();
        iceCandidates.clear();
        localDescriptions.clear();
    }

    // Shared test state
    std::vector<std::string> logMessages;
    std::vector<ConnectionState> stateChanges;
    std::vector<std::pair<std::string, std::string>> iceCandidates;
    std::vector<std::pair<SdpType, std::string>> localDescriptions;

    PeerConnectionConfig createTestConfig() {
        PeerConnectionConfig config;
        config.iceServers = {"stun:stun.l.google.com:19302"};

        config.logCallback = [this](LogLevel level, const std::string& message) {
            logMessages.push_back(message);
        };

        config.stateCallback = [this](ConnectionState state) {
            stateChanges.push_back(state);
        };

        config.iceCandidateCallback = [this](const std::string& candidate, const std::string& mid) {
            iceCandidates.push_back({candidate, mid});
        };

        config.localDescriptionCallback = [this](SdpType type, const std::string& sdp) {
            localDescriptions.push_back({type, sdp});
        };

        return config;
    }
};

// Test: PeerConnection construction
TEST_F(PeerConnectionTest, ConstructionSucceeds) {
    auto config = createTestConfig();
    EXPECT_NO_THROW({
        auto pc = std::make_unique<PeerConnection>(config);
    });
}

// Test: Initial state is New
TEST_F(PeerConnectionTest, InitialStateIsNew) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    EXPECT_EQ(pc->getState(), ConnectionState::New);
    EXPECT_FALSE(pc->isConnected());
}

// Test: Create offer generates local description
TEST_F(PeerConnectionTest, CreateOfferGeneratesLocalDescription) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();

    // Wait a bit for async operations
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_FALSE(localDescriptions.empty());
    EXPECT_EQ(localDescriptions[0].first, SdpType::Offer);
    EXPECT_FALSE(localDescriptions[0].second.empty());
}

// Test: Create answer after setting remote offer
TEST_F(PeerConnectionTest, CreateAnswerAfterRemoteOffer) {
    // Create offerer
    auto config1 = createTestConfig();
    auto offerer = std::make_unique<PeerConnection>(config1);
    offerer->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(localDescriptions.empty());

    std::string offerSdp = localDescriptions[0].second;
    localDescriptions.clear();

    // Create answerer
    auto config2 = createTestConfig();
    auto answerer = std::make_unique<PeerConnection>(config2);
    answerer->setRemoteDescription(SdpType::Offer, offerSdp);
    answerer->createAnswer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_FALSE(localDescriptions.empty());
    EXPECT_EQ(localDescriptions[0].first, SdpType::Answer);
    EXPECT_FALSE(localDescriptions[0].second.empty());
}

// Test: ICE candidates are collected
TEST_F(PeerConnectionTest, IceCandidatesAreCollected) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();

    // Wait for ICE candidates to be gathered
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Should have at least one ICE candidate
    EXPECT_FALSE(iceCandidates.empty());
}

// Test: State changes are reported
TEST_F(PeerConnectionTest, StateChangesAreReported) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should have some state changes
    EXPECT_FALSE(stateChanges.empty());
}

// Test: Close changes state to Closed
TEST_F(PeerConnectionTest, CloseChangesStateToClosed) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pc->close();

    EXPECT_EQ(pc->getState(), ConnectionState::Closed);
    EXPECT_FALSE(pc->isConnected());
}

// Test: Get local description returns non-empty after offer
TEST_F(PeerConnectionTest, GetLocalDescriptionAfterOffer) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::string localDesc = pc->getLocalDescription();
    EXPECT_FALSE(localDesc.empty());
    EXPECT_NE(localDesc.find("v=0"), std::string::npos); // SDP should start with v=0
}

// Test: Set remote description succeeds
TEST_F(PeerConnectionTest, SetRemoteDescriptionSucceeds) {
    auto config1 = createTestConfig();
    auto offerer = std::make_unique<PeerConnection>(config1);
    offerer->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(localDescriptions.empty());

    std::string offerSdp = localDescriptions[0].second;

    auto config2 = createTestConfig();
    auto answerer = std::make_unique<PeerConnection>(config2);

    EXPECT_NO_THROW({
        answerer->setRemoteDescription(SdpType::Offer, offerSdp);
    });
}

// Test: Add ICE candidate succeeds
TEST_F(PeerConnectionTest, AddIceCandidateSucceeds) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Add a valid ICE candidate
    EXPECT_NO_THROW({
        pc->addIceCandidate(
            "candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host",
            "0"
        );
    });
}

// Test: Move semantics work correctly
TEST_F(PeerConnectionTest, MoveSemantics) {
    auto config = createTestConfig();
    auto pc1 = std::make_unique<PeerConnection>(config);

    pc1->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Move construction
    auto pc2 = std::move(pc1);
    EXPECT_NE(pc2, nullptr);
    EXPECT_FALSE(pc2->getLocalDescription().empty());

    // Move assignment
    auto pc3 = std::make_unique<PeerConnection>(createTestConfig());
    pc3 = std::move(pc2);
    EXPECT_FALSE(pc3->getLocalDescription().empty());
}

// Test: Multiple peer connections can coexist
TEST_F(PeerConnectionTest, MultiplePeerConnectionsCoexist) {
    auto config1 = createTestConfig();
    auto config2 = createTestConfig();

    auto pc1 = std::make_unique<PeerConnection>(config1);
    auto pc2 = std::make_unique<PeerConnection>(config2);

    pc1->createOffer();
    pc2->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_FALSE(pc1->getLocalDescription().empty());
    EXPECT_FALSE(pc2->getLocalDescription().empty());
}

// Test: Logging callback is invoked
TEST_F(PeerConnectionTest, LoggingCallbackIsInvoked) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should have some log messages
    EXPECT_FALSE(logMessages.empty());
}
