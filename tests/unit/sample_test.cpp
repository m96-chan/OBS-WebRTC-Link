/**
 * Sample unit test for OBS WebRTC Link plugin
 *
 * This is a basic example test to verify that the test framework is properly configured.
 * Real tests should be added as the plugin functionality is implemented.
 */

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

// Sample test to verify Google Test is working
TEST(SampleTest, BasicAssertion) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

// Sample test to verify nlohmann-json is accessible
TEST(SampleTest, JsonLibraryWorks) {
    nlohmann::json j;
    j["name"] = "OBS WebRTC Link";
    j["version"] = "0.1.0";
    j["enabled"] = true;

    EXPECT_EQ(j["name"], "OBS WebRTC Link");
    EXPECT_EQ(j["version"], "0.1.0");
    EXPECT_TRUE(j["enabled"]);
}

// Sample test with multiple assertions
TEST(SampleTest, MultipleAssertions) {
    std::string plugin_name = "obs-webrtc-link";

    EXPECT_FALSE(plugin_name.empty());
    EXPECT_EQ(plugin_name.length(), 15);  // "obs-webrtc-link" is 15 characters
    EXPECT_EQ(plugin_name.find("webrtc"), 4);
}

// Sample test demonstrating test fixture
class PluginTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code that runs before each test
        test_value = 42;
    }

    void TearDown() override {
        // Cleanup code that runs after each test
    }

    int test_value;
};

TEST_F(PluginTestFixture, FixtureTest) {
    EXPECT_EQ(test_value, 42);
    test_value = 100;
    EXPECT_EQ(test_value, 100);
}

// TODO: Add real tests for plugin functionality:
// - WebRTC connection initialization
// - Signaling message parsing
// - WHIP/WHEP endpoint handling
// - P2P session management
// - Error handling
// - Configuration validation
