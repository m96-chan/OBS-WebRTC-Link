/**
 * @file version_test.cpp
 * @brief Unit tests for version management
 */

#include <gtest/gtest.h>
#include <fstream>
#include <regex>
#include <string>

/**
 * @brief Test fixture for version management tests
 */
class VersionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }

    /**
     * @brief Read version from VERSION file
     * @return Version string
     */
    std::string readVersionFile() {
        std::ifstream file("../VERSION");
        if (!file.is_open()) {
            // Try alternative path for different build configurations
            file.open("../../VERSION");
            if (!file.is_open()) {
                return "";
            }
        }
        std::string version;
        std::getline(file, version);
        return version;
    }

    /**
     * @brief Check if version string follows semantic versioning
     * @param version Version string to validate
     * @return true if version is valid semantic version, false otherwise
     */
    bool isValidSemanticVersion(const std::string& version) {
        // Regex for semantic versioning: MAJOR.MINOR.PATCH
        std::regex semver_regex(R"(^\d+\.\d+\.\d+$)");
        return std::regex_match(version, semver_regex);
    }

    /**
     * @brief Parse version components
     * @param version Version string (e.g., "1.2.3")
     * @param major Output major version
     * @param minor Output minor version
     * @param patch Output patch version
     * @return true if parsing succeeded, false otherwise
     */
    bool parseVersion(const std::string& version, int& major, int& minor, int& patch) {
        std::regex version_regex(R"(^(\d+)\.(\d+)\.(\d+)$)");
        std::smatch matches;

        if (!std::regex_match(version, matches, version_regex)) {
            return false;
        }

        major = std::stoi(matches[1].str());
        minor = std::stoi(matches[2].str());
        patch = std::stoi(matches[3].str());
        return true;
    }
};

/**
 * @brief Test that VERSION file exists
 */
TEST_F(VersionTest, VersionFileExists) {
    std::string version = readVersionFile();
    EXPECT_FALSE(version.empty()) << "VERSION file should exist and be readable";
}

/**
 * @brief Test that version follows semantic versioning format
 */
TEST_F(VersionTest, VersionFollowsSemanticVersioning) {
    std::string version = readVersionFile();
    ASSERT_FALSE(version.empty()) << "VERSION file must exist";

    EXPECT_TRUE(isValidSemanticVersion(version))
        << "Version '" << version << "' should follow semantic versioning format (MAJOR.MINOR.PATCH)";
}

/**
 * @brief Test that version components are non-negative
 */
TEST_F(VersionTest, VersionComponentsAreNonNegative) {
    std::string version = readVersionFile();
    ASSERT_FALSE(version.empty()) << "VERSION file must exist";

    int major, minor, patch;
    ASSERT_TRUE(parseVersion(version, major, minor, patch))
        << "Version should be parseable";

    EXPECT_GE(major, 0) << "Major version should be non-negative";
    EXPECT_GE(minor, 0) << "Minor version should be non-negative";
    EXPECT_GE(patch, 0) << "Patch version should be non-negative";
}

/**
 * @brief Test that PLUGIN_VERSION macro is defined
 */
#ifdef PLUGIN_VERSION
TEST_F(VersionTest, PluginVersionMacroIsDefined) {
    EXPECT_NE(PLUGIN_VERSION, nullptr) << "PLUGIN_VERSION macro should be defined";
    EXPECT_GT(strlen(PLUGIN_VERSION), 0) << "PLUGIN_VERSION should not be empty";
}

/**
 * @brief Test that PLUGIN_VERSION matches VERSION file
 */
TEST_F(VersionTest, PluginVersionMatchesVersionFile) {
    std::string file_version = readVersionFile();
    ASSERT_FALSE(file_version.empty()) << "VERSION file must exist";

    std::string plugin_version(PLUGIN_VERSION);
    EXPECT_EQ(plugin_version, file_version)
        << "PLUGIN_VERSION macro should match VERSION file content";
}

/**
 * @brief Test that PLUGIN_VERSION follows semantic versioning
 */
TEST_F(VersionTest, PluginVersionFollowsSemanticVersioning) {
    std::string plugin_version(PLUGIN_VERSION);
    EXPECT_TRUE(isValidSemanticVersion(plugin_version))
        << "PLUGIN_VERSION '" << plugin_version << "' should follow semantic versioning format";
}
#else
TEST_F(VersionTest, PluginVersionMacroNotDefined) {
    GTEST_SKIP() << "PLUGIN_VERSION macro is not defined (expected when building tests only)";
}
#endif

/**
 * @brief Test version string format edge cases
 */
TEST_F(VersionTest, VersionFormatEdgeCases) {
    // Valid versions
    EXPECT_TRUE(isValidSemanticVersion("0.0.0"));
    EXPECT_TRUE(isValidSemanticVersion("1.0.0"));
    EXPECT_TRUE(isValidSemanticVersion("0.1.0"));
    EXPECT_TRUE(isValidSemanticVersion("0.0.1"));
    EXPECT_TRUE(isValidSemanticVersion("10.20.30"));
    EXPECT_TRUE(isValidSemanticVersion("999.999.999"));

    // Invalid versions
    EXPECT_FALSE(isValidSemanticVersion("1.0"));        // Missing patch
    EXPECT_FALSE(isValidSemanticVersion("1"));          // Missing minor and patch
    EXPECT_FALSE(isValidSemanticVersion("1.0.0.0"));    // Too many components
    EXPECT_FALSE(isValidSemanticVersion("1.0.0-alpha")); // Pre-release tag
    EXPECT_FALSE(isValidSemanticVersion("v1.0.0"));     // Version prefix
    EXPECT_FALSE(isValidSemanticVersion("1.0.a"));      // Non-numeric
    EXPECT_FALSE(isValidSemanticVersion(""));           // Empty
    EXPECT_FALSE(isValidSemanticVersion("1.0.0 "));     // Trailing space
}

/**
 * @brief Test version parsing with various inputs
 */
TEST_F(VersionTest, VersionParsing) {
    int major, minor, patch;

    // Valid version
    ASSERT_TRUE(parseVersion("1.2.3", major, minor, patch));
    EXPECT_EQ(major, 1);
    EXPECT_EQ(minor, 2);
    EXPECT_EQ(patch, 3);

    // Version with zeros
    ASSERT_TRUE(parseVersion("0.0.0", major, minor, patch));
    EXPECT_EQ(major, 0);
    EXPECT_EQ(minor, 0);
    EXPECT_EQ(patch, 0);

    // Large version numbers
    ASSERT_TRUE(parseVersion("100.200.300", major, minor, patch));
    EXPECT_EQ(major, 100);
    EXPECT_EQ(minor, 200);
    EXPECT_EQ(patch, 300);

    // Invalid versions
    EXPECT_FALSE(parseVersion("1.0", major, minor, patch));
    EXPECT_FALSE(parseVersion("1.0.0.0", major, minor, patch));
    EXPECT_FALSE(parseVersion("a.b.c", major, minor, patch));
}
