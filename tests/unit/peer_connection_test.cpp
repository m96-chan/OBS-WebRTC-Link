/**
 * @file peer_connection_test.cpp
 * @brief Unit tests for PeerConnection class
 */

#include "../../src/core/peer-connection.hpp"

#include <atomic>
#include <chrono>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

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

    // Helper struct to hold independent callback state
    struct CallbackState {
        std::vector<std::string> logMessages;
        std::vector<ConnectionState> stateChanges;
        std::vector<std::pair<std::string, std::string>> iceCandidates;
        std::vector<std::pair<SdpType, std::string>> localDescriptions;
    };

    PeerConnectionConfig createTestConfig() {
        PeerConnectionConfig config;
        config.iceServers = {"stun:stun.l.google.com:19302"};

        config.logCallback = [this](LogLevel level, const std::string& message) {
            logMessages.push_back(message);
        };

        config.stateCallback = [this](ConnectionState state) { stateChanges.push_back(state); };

        config.iceCandidateCallback = [this](const std::string& candidate,
                                             const std::string& mid) {
            iceCandidates.push_back({candidate, mid});
        };

        config.localDescriptionCallback = [this](SdpType type, const std::string& sdp) {
            localDescriptions.push_back({type, sdp});
        };

        return config;
    }

    // Create config with independent callback state (for tests using multiple PeerConnections)
    PeerConnectionConfig createTestConfigWithState(CallbackState& state) {
        PeerConnectionConfig config;
        config.iceServers = {"stun:stun.l.google.com:19302"};

        config.logCallback = [&state](LogLevel level, const std::string& message) {
            state.logMessages.push_back(message);
        };

        config.stateCallback = [&state](ConnectionState s) {
            state.stateChanges.push_back(s);
        };

        config.iceCandidateCallback = [&state](const std::string& candidate,
                                              const std::string& mid) {
            state.iceCandidates.push_back({candidate, mid});
        };

        config.localDescriptionCallback = [&state](SdpType type, const std::string& sdp) {
            state.localDescriptions.push_back({type, sdp});
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
    // Create offerer with independent state
    CallbackState offererState, answererState;

    auto config1 = createTestConfigWithState(offererState);
    auto offerer = std::make_unique<PeerConnection>(config1);
    offerer->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(offererState.localDescriptions.empty());

    std::string offerSdp = offererState.localDescriptions[0].second;

    // Create answerer with independent state
    auto config2 = createTestConfigWithState(answererState);
    auto answerer = std::make_unique<PeerConnection>(config2);
    answerer->setRemoteDescription(SdpType::Offer, offerSdp);
    answerer->createAnswer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ASSERT_FALSE(answererState.localDescriptions.empty());
    EXPECT_EQ(answererState.localDescriptions[0].first, SdpType::Answer);
    EXPECT_FALSE(answererState.localDescriptions[0].second.empty());
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
    CallbackState offererState, answererState;

    auto config1 = createTestConfigWithState(offererState);
    auto offerer = std::make_unique<PeerConnection>(config1);
    offerer->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(offererState.localDescriptions.empty());

    std::string offerSdp = offererState.localDescriptions[0].second;

    auto config2 = createTestConfigWithState(answererState);
    auto answerer = std::make_unique<PeerConnection>(config2);

    EXPECT_NO_THROW({ answerer->setRemoteDescription(SdpType::Offer, offerSdp); });

    // Wait for cleanup before destroying connections
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

// Test: Add ICE candidate succeeds
TEST_F(PeerConnectionTest, AddIceCandidateSucceeds) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Add a valid ICE candidate
    EXPECT_NO_THROW({
        pc->addIceCandidate("candidate:1 1 UDP 2130706431 192.168.1.1 54321 typ host",
                            "0");
    });

    // Wait for cleanup before destroying connection
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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
    // Create separate state for each connection to avoid shared callback issues
    CallbackState state1, state2;

    auto config1 = createTestConfigWithState(state1);
    auto config2 = createTestConfigWithState(state2);

    auto pc1 = std::make_unique<PeerConnection>(config1);
    auto pc2 = std::make_unique<PeerConnection>(config2);

    pc1->createOffer();
    pc2->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_FALSE(pc1->getLocalDescription().empty());
    EXPECT_FALSE(pc2->getLocalDescription().empty());

    // Explicitly close connections before destruction to avoid double-free
    pc1->close();
    pc2->close();

    // Allow time for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
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

// ========== Edge Cases and Error Handling Tests ==========

// Test: Empty ICE servers configuration
TEST_F(PeerConnectionTest, EmptyIceServersConfiguration) {
    auto config = createTestConfig();
    config.iceServers.clear();

    EXPECT_NO_THROW({
        auto pc = std::make_unique<PeerConnection>(config);
        pc->createOffer();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
}

// Test: Multiple ICE servers
TEST_F(PeerConnectionTest, MultipleIceServers) {
    auto config = createTestConfig();
    config.iceServers = {
        "stun:stun.l.google.com:19302",
        "stun:stun1.l.google.com:19302",
        "stun:stun2.l.google.com:19302"
    };

    EXPECT_NO_THROW({
        auto pc = std::make_unique<PeerConnection>(config);
        pc->createOffer();
    });
}

// Test: Invalid SDP format for remote description
TEST_F(PeerConnectionTest, InvalidSdpFormatThrows) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    EXPECT_THROW({
        pc->setRemoteDescription(SdpType::Offer, "invalid sdp");
    }, std::runtime_error);
}

// Test: Empty SDP for remote description
TEST_F(PeerConnectionTest, EmptySdpThrows) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    EXPECT_THROW({
        pc->setRemoteDescription(SdpType::Offer, "");
    }, std::runtime_error);
}

// Test: Invalid ICE candidate format
TEST_F(PeerConnectionTest, InvalidIceCandidateThrows) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_THROW({
        pc->addIceCandidate("invalid candidate", "0");
    }, std::runtime_error);
}

// Test: Empty ICE candidate
TEST_F(PeerConnectionTest, EmptyIceCandidateThrows) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_THROW({
        pc->addIceCandidate("", "0");
    }, std::runtime_error);
}

// Test: Create answer without remote offer throws
TEST_F(PeerConnectionTest, CreateAnswerWithoutRemoteOfferThrows) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    EXPECT_THROW({
        pc->createAnswer();
    }, std::runtime_error);
}

// Test: Create offer twice is allowed
TEST_F(PeerConnectionTest, CreateOfferTwiceIsAllowed) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    EXPECT_NO_THROW({
        pc->createOffer();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        pc->createOffer();
    });
}

// Test: Close can be called multiple times
TEST_F(PeerConnectionTest, CloseCanBeCalledMultipleTimes) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_NO_THROW({
        pc->close();
        pc->close();
        pc->close();
    });

    EXPECT_EQ(pc->getState(), ConnectionState::Closed);
}

// Test: Operations after close throw or are no-op
TEST_F(PeerConnectionTest, OperationsAfterCloseAreNoOp) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    pc->close();

    // Wait for close to complete before attempting operations
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // These should not crash, but may throw or be no-op
    EXPECT_NO_THROW({
        pc->createOffer();
    });

    EXPECT_EQ(pc->getState(), ConnectionState::Closed);
}

// Test: Get remote description before setting returns empty
TEST_F(PeerConnectionTest, GetRemoteDescriptionBeforeSettingReturnsEmpty) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    std::string remoteDesc = pc->getRemoteDescription();
    EXPECT_TRUE(remoteDesc.empty());
}

// Test: Get remote description after setting returns non-empty
TEST_F(PeerConnectionTest, GetRemoteDescriptionAfterSetting) {
    CallbackState offererState, answererState;

    auto config1 = createTestConfigWithState(offererState);
    auto offerer = std::make_unique<PeerConnection>(config1);
    offerer->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(offererState.localDescriptions.empty());

    std::string offerSdp = offererState.localDescriptions[0].second;

    auto config2 = createTestConfigWithState(answererState);
    auto answerer = std::make_unique<PeerConnection>(config2);
    answerer->setRemoteDescription(SdpType::Offer, offerSdp);

    std::string remoteDesc = answerer->getRemoteDescription();
    EXPECT_FALSE(remoteDesc.empty());
    EXPECT_EQ(remoteDesc, offerSdp);
}

// ========== State Transition Tests ==========

// Test: State transitions from New to Checking
TEST_F(PeerConnectionTest, StateTransitionsFromNewToChecking) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    EXPECT_EQ(pc->getState(), ConnectionState::New);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // State should have changed
    EXPECT_NE(pc->getState(), ConnectionState::New);
}

// Test: State change callback receives all transitions
TEST_F(PeerConnectionTest, StateChangeCallbackReceivesAllTransitions) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Should have at least one state change
    EXPECT_GE(stateChanges.size(), 1);

    // First state should not be New (already transitioned)
    if (!stateChanges.empty()) {
        EXPECT_TRUE(
            stateChanges[0] == ConnectionState::Checking ||
            stateChanges[0] == ConnectionState::Connected ||
            stateChanges[0] == ConnectionState::Failed
        );
    }
}

// Test: isConnected returns true only when connected or completed
TEST_F(PeerConnectionTest, IsConnectedOnlyWhenConnectedOrCompleted) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    EXPECT_FALSE(pc->isConnected());

    pc->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // May or may not be connected yet (depends on timing)
    // But should be consistent with state
    bool connected = pc->isConnected();
    ConnectionState state = pc->getState();

    if (connected) {
        EXPECT_TRUE(
            state == ConnectionState::Connected ||
            state == ConnectionState::Completed
        );
    } else {
        EXPECT_TRUE(
            state != ConnectionState::Connected &&
            state != ConnectionState::Completed
        );
    }
}

// ========== Concurrent Operations Tests ==========

// Test: Concurrent offer creation
TEST_F(PeerConnectionTest, ConcurrentOfferCreation) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    std::thread t1([&pc]() { pc->createOffer(); });
    std::thread t2([&pc]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pc->createOffer();
    });

    t1.join();
    t2.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should not crash and should have local description
    EXPECT_FALSE(pc->getLocalDescription().empty());
}

// Test: Callback safety under concurrent operations
TEST_F(PeerConnectionTest, CallbackSafetyUnderConcurrency) {
    auto config = createTestConfig();
    std::atomic<int> callbackCount{0};

    config.localDescriptionCallback = [&callbackCount](SdpType type, const std::string& sdp) {
        callbackCount++;
    };

    auto pc = std::make_unique<PeerConnection>(config);

    std::thread t1([&pc]() { pc->createOffer(); });
    std::thread t2([&pc]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pc->createOffer();
    });

    t1.join();
    t2.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Should have received at least one callback
    EXPECT_GT(callbackCount.load(), 0);
}

// ========== Performance Tests ==========

// Test: Offer creation completes quickly (< 1 second)
TEST_F(PeerConnectionTest, OfferCreationPerformance) {
    auto config = createTestConfig();
    auto pc = std::make_unique<PeerConnection>(config);

    auto start = std::chrono::steady_clock::now();
    pc->createOffer();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete within 1 second
    EXPECT_LT(duration.count(), 1000);
    EXPECT_FALSE(localDescriptions.empty());
}

// Test: Multiple offer-answer exchanges
TEST_F(PeerConnectionTest, MultipleOfferAnswerExchanges) {
    auto config1 = createTestConfig();
    auto pc1 = std::make_unique<PeerConnection>(config1);

    // First offer
    pc1->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(localDescriptions.empty());
    std::string offer1 = localDescriptions[0].second;
    localDescriptions.clear();

    // Second offer
    pc1->createOffer();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_FALSE(localDescriptions.empty());
    std::string offer2 = localDescriptions[0].second;

    // Both offers should be valid
    EXPECT_FALSE(offer1.empty());
    EXPECT_FALSE(offer2.empty());
    EXPECT_NE(offer1.find("v=0"), std::string::npos);
    EXPECT_NE(offer2.find("v=0"), std::string::npos);
}
