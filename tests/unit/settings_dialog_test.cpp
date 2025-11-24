// Copyright (c) 2025 OBS-WebRTC-Link Project
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QClipboard>
#include <QGuiApplication>
#include <QRegularExpression>
#include <QCheckBox>
#include <QGroupBox>
#include "ui/settings-dialog.hpp"

namespace {

// Mock obs_data_t for testing
class MockObsData {
public:
    std::string server_url;
    std::string connection_mode;
    std::string video_codec;
    int video_bitrate;
    std::string audio_codec;
    int audio_bitrate;
    std::string token;
};

class SettingsDialogTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QApplication if not already created
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }
    }

    void TearDown() override {
        // Clean up
    }

    QApplication* app = nullptr;
};

// Test 1: Settings dialog can be created
TEST_F(SettingsDialogTest, CanCreateDialog) {
    SettingsDialog dialog(nullptr);
    EXPECT_FALSE(dialog.windowTitle().isEmpty());
}

// Test 2: Dialog has all required input fields
TEST_F(SettingsDialogTest, HasAllRequiredFields) {
    SettingsDialog dialog(nullptr);

    // Check for connection mode combo box
    QComboBox* connectionMode = dialog.findChild<QComboBox*>("connectionModeCombo");
    ASSERT_NE(connectionMode, nullptr);
    EXPECT_GE(connectionMode->count(), 2); // At least SFU and P2P

    // Check for server URL input
    QLineEdit* serverUrl = dialog.findChild<QLineEdit*>("serverUrlEdit");
    ASSERT_NE(serverUrl, nullptr);

    // Check for token input
    QLineEdit* token = dialog.findChild<QLineEdit*>("tokenEdit");
    ASSERT_NE(token, nullptr);

    // Check for video codec combo box
    QComboBox* videoCodec = dialog.findChild<QComboBox*>("videoCodecCombo");
    ASSERT_NE(videoCodec, nullptr);
    EXPECT_GT(videoCodec->count(), 0);

    // Check for audio codec combo box
    QComboBox* audioCodec = dialog.findChild<QComboBox*>("audioCodecCombo");
    ASSERT_NE(audioCodec, nullptr);
    EXPECT_GT(audioCodec->count(), 0);

    // Check for video bitrate spin box
    QSpinBox* videoBitrate = dialog.findChild<QSpinBox*>("videoBitrateSpin");
    ASSERT_NE(videoBitrate, nullptr);
    EXPECT_GE(videoBitrate->minimum(), 100);
    EXPECT_LE(videoBitrate->maximum(), 50000);

    // Check for audio bitrate spin box
    QSpinBox* audioBitrate = dialog.findChild<QSpinBox*>("audioBitrateSpin");
    ASSERT_NE(audioBitrate, nullptr);
}

// Test 3: Connection mode has correct options
TEST_F(SettingsDialogTest, ConnectionModeOptions) {
    SettingsDialog dialog(nullptr);

    QComboBox* connectionMode = dialog.findChild<QComboBox*>("connectionModeCombo");
    ASSERT_NE(connectionMode, nullptr);

    QStringList items;
    for (int i = 0; i < connectionMode->count(); ++i) {
        items.append(connectionMode->itemText(i));
    }

    EXPECT_TRUE(items.contains("SFU"));
    EXPECT_TRUE(items.contains("P2P"));
}

// Test 4: Video codec has correct options
TEST_F(SettingsDialogTest, VideoCodecOptions) {
    SettingsDialog dialog(nullptr);

    QComboBox* videoCodec = dialog.findChild<QComboBox*>("videoCodecCombo");
    ASSERT_NE(videoCodec, nullptr);

    QStringList items;
    for (int i = 0; i < videoCodec->count(); ++i) {
        items.append(videoCodec->itemData(i).toString());
    }

    EXPECT_TRUE(items.contains("h264"));
    EXPECT_TRUE(items.contains("vp8"));
    EXPECT_TRUE(items.contains("vp9"));
    EXPECT_TRUE(items.contains("av1"));
}

// Test 5: Audio codec has correct options
TEST_F(SettingsDialogTest, AudioCodecOptions) {
    SettingsDialog dialog(nullptr);

    QComboBox* audioCodec = dialog.findChild<QComboBox*>("audioCodecCombo");
    ASSERT_NE(audioCodec, nullptr);

    QStringList items;
    for (int i = 0; i < audioCodec->count(); ++i) {
        items.append(audioCodec->itemData(i).toString());
    }

    EXPECT_TRUE(items.contains("opus"));
    EXPECT_TRUE(items.contains("aac"));
}

// Test 6: Can set and get server URL
TEST_F(SettingsDialogTest, CanSetAndGetServerUrl) {
    SettingsDialog dialog(nullptr);

    QString testUrl = "https://example.com/webrtc";
    dialog.setServerUrl(testUrl);

    EXPECT_EQ(dialog.getServerUrl(), testUrl);
}

// Test 7: Can set and get connection mode
TEST_F(SettingsDialogTest, CanSetAndGetConnectionMode) {
    SettingsDialog dialog(nullptr);

    dialog.setConnectionMode("P2P");
    EXPECT_EQ(dialog.getConnectionMode(), "P2P");

    dialog.setConnectionMode("SFU");
    EXPECT_EQ(dialog.getConnectionMode(), "SFU");
}

// Test 8: Can set and get video codec
TEST_F(SettingsDialogTest, CanSetAndGetVideoCodec) {
    SettingsDialog dialog(nullptr);

    dialog.setVideoCodec("vp9");
    EXPECT_EQ(dialog.getVideoCodec(), "vp9");

    dialog.setVideoCodec("h264");
    EXPECT_EQ(dialog.getVideoCodec(), "h264");
}

// Test 9: Can set and get audio codec
TEST_F(SettingsDialogTest, CanSetAndGetAudioCodec) {
    SettingsDialog dialog(nullptr);

    dialog.setAudioCodec("opus");
    EXPECT_EQ(dialog.getAudioCodec(), "opus");

    dialog.setAudioCodec("aac");
    EXPECT_EQ(dialog.getAudioCodec(), "aac");
}

// Test 10: Can set and get video bitrate
TEST_F(SettingsDialogTest, CanSetAndGetVideoBitrate) {
    SettingsDialog dialog(nullptr);

    dialog.setVideoBitrate(5000);
    EXPECT_EQ(dialog.getVideoBitrate(), 5000);
}

// Test 11: Can set and get audio bitrate
TEST_F(SettingsDialogTest, CanSetAndGetAudioBitrate) {
    SettingsDialog dialog(nullptr);

    dialog.setAudioBitrate(192);
    EXPECT_EQ(dialog.getAudioBitrate(), 192);
}

// Test 12: Can set and get token
TEST_F(SettingsDialogTest, CanSetAndGetToken) {
    SettingsDialog dialog(nullptr);

    QString testToken = "test-token-123";
    dialog.setToken(testToken);

    EXPECT_EQ(dialog.getToken(), testToken);
}

// Test 13: Validates empty server URL
TEST_F(SettingsDialogTest, ValidatesEmptyServerUrl) {
    SettingsDialog dialog(nullptr);

    dialog.setServerUrl("");
    EXPECT_FALSE(dialog.validateSettings());
}

// Test 14: Validates invalid server URL format
TEST_F(SettingsDialogTest, ValidatesInvalidServerUrlFormat) {
    SettingsDialog dialog(nullptr);

    dialog.setServerUrl("not-a-valid-url");
    EXPECT_FALSE(dialog.validateSettings());

    dialog.setServerUrl("ftp://invalid-protocol.com");
    EXPECT_FALSE(dialog.validateSettings());
}

// Test 15: Accepts valid HTTPS URL
TEST_F(SettingsDialogTest, AcceptsValidHttpsUrl) {
    SettingsDialog dialog(nullptr);

    dialog.setServerUrl("https://example.com/webrtc");
    EXPECT_TRUE(dialog.validateSettings());
}

// Test 16: Accepts valid HTTP URL
TEST_F(SettingsDialogTest, AcceptsValidHttpUrl) {
    SettingsDialog dialog(nullptr);

    dialog.setServerUrl("http://localhost:8080/webrtc");
    EXPECT_TRUE(dialog.validateSettings());
}

// Test 17: Validates video bitrate range
TEST_F(SettingsDialogTest, ValidatesVideoBitrateRange) {
    SettingsDialog dialog(nullptr);

    QSpinBox* videoBitrate = dialog.findChild<QSpinBox*>("videoBitrateSpin");
    ASSERT_NE(videoBitrate, nullptr);

    // Test minimum value
    videoBitrate->setValue(videoBitrate->minimum() - 1);
    EXPECT_EQ(videoBitrate->value(), videoBitrate->minimum());

    // Test maximum value
    videoBitrate->setValue(videoBitrate->maximum() + 1);
    EXPECT_EQ(videoBitrate->value(), videoBitrate->maximum());
}

// Test 18: Validates audio bitrate range
TEST_F(SettingsDialogTest, ValidatesAudioBitrateRange) {
    SettingsDialog dialog(nullptr);

    QSpinBox* audioBitrate = dialog.findChild<QSpinBox*>("audioBitrateSpin");
    ASSERT_NE(audioBitrate, nullptr);

    // Test minimum value
    audioBitrate->setValue(audioBitrate->minimum() - 1);
    EXPECT_EQ(audioBitrate->value(), audioBitrate->minimum());

    // Test maximum value
    audioBitrate->setValue(audioBitrate->maximum() + 1);
    EXPECT_EQ(audioBitrate->value(), audioBitrate->maximum());
}

// Test 19: Has OK and Cancel buttons
TEST_F(SettingsDialogTest, HasOkAndCancelButtons) {
    SettingsDialog dialog(nullptr);

    QPushButton* okButton = dialog.findChild<QPushButton*>("okButton");
    ASSERT_NE(okButton, nullptr);

    QPushButton* cancelButton = dialog.findChild<QPushButton*>("cancelButton");
    ASSERT_NE(cancelButton, nullptr);
}

// Test 20: Default values are set correctly
TEST_F(SettingsDialogTest, DefaultValuesAreCorrect) {
    SettingsDialog dialog(nullptr);

    EXPECT_EQ(dialog.getServerUrl(), "");
    EXPECT_EQ(dialog.getConnectionMode(), "SFU");
    EXPECT_EQ(dialog.getVideoCodec(), "h264");
    EXPECT_EQ(dialog.getAudioCodec(), "opus");
    EXPECT_EQ(dialog.getVideoBitrate(), 2500);
    EXPECT_EQ(dialog.getAudioBitrate(), 128);
    EXPECT_EQ(dialog.getToken(), "");
}

// Issue #15 Tests: Dynamic field visibility based on connection mode

// Test 21: SFU mode shows server URL field
TEST_F(SettingsDialogTest, SfuModeShowsServerUrlField) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("SFU");

    QLineEdit* serverUrl = dialog.findChild<QLineEdit*>("serverUrlEdit");
    ASSERT_NE(serverUrl, nullptr);
    EXPECT_TRUE(serverUrl->isEnabled());
    EXPECT_TRUE(serverUrl->isVisible());
}

// Test 22: SFU mode shows token field
TEST_F(SettingsDialogTest, SfuModeShowsTokenField) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("SFU");

    QLineEdit* token = dialog.findChild<QLineEdit*>("tokenEdit");
    ASSERT_NE(token, nullptr);
    EXPECT_TRUE(token->isEnabled());
    EXPECT_TRUE(token->isVisible());
}

// Test 23: P2P mode hides server URL field
TEST_F(SettingsDialogTest, P2pModeHidesServerUrlField) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");

    QLineEdit* serverUrl = dialog.findChild<QLineEdit*>("serverUrlEdit");
    ASSERT_NE(serverUrl, nullptr);
    EXPECT_FALSE(serverUrl->isEnabled()) << "Server URL should be disabled in P2P mode";
}

// Test 24: P2P mode hides token field
TEST_F(SettingsDialogTest, P2pModeHidesTokenField) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");

    QLineEdit* token = dialog.findChild<QLineEdit*>("tokenEdit");
    ASSERT_NE(token, nullptr);
    EXPECT_FALSE(token->isEnabled()) << "Token should be disabled in P2P mode";
}

// Test 25: P2P mode shows session ID field
TEST_F(SettingsDialogTest, P2pModeShowsSessionIdField) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");

    QLineEdit* sessionId = dialog.findChild<QLineEdit*>("sessionIdEdit");
    ASSERT_NE(sessionId, nullptr) << "Session ID field should exist";
    EXPECT_TRUE(sessionId->isEnabled()) << "Session ID should be enabled in P2P mode";
    EXPECT_TRUE(sessionId->isVisible()) << "Session ID should be visible in P2P mode";
}

// Test 26: SFU mode hides session ID field
TEST_F(SettingsDialogTest, SfuModeHidesSessionIdField) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("SFU");

    QLineEdit* sessionId = dialog.findChild<QLineEdit*>("sessionIdEdit");
    if (sessionId) {
        EXPECT_FALSE(sessionId->isEnabled()) << "Session ID should be disabled in SFU mode";
    }
}

// Test 27: Can set and get session ID
TEST_F(SettingsDialogTest, CanSetAndGetSessionId) {
    SettingsDialog dialog(nullptr);

    QString testSessionId = "test-session-12345";
    dialog.setSessionId(testSessionId);

    EXPECT_EQ(dialog.getSessionId(), testSessionId);
}

// Test 28: Switching from SFU to P2P updates field visibility
TEST_F(SettingsDialogTest, SwitchingFromSfuToP2pUpdatesVisibility) {
    SettingsDialog dialog(nullptr);

    // Start with SFU mode
    dialog.setConnectionMode("SFU");
    QLineEdit* serverUrl = dialog.findChild<QLineEdit*>("serverUrlEdit");
    QLineEdit* sessionId = dialog.findChild<QLineEdit*>("sessionIdEdit");

    ASSERT_NE(serverUrl, nullptr);
    EXPECT_TRUE(serverUrl->isEnabled());

    // Switch to P2P mode
    dialog.setConnectionMode("P2P");

    EXPECT_FALSE(serverUrl->isEnabled()) << "Server URL should be disabled after switching to P2P";
    ASSERT_NE(sessionId, nullptr);
    EXPECT_TRUE(sessionId->isEnabled()) << "Session ID should be enabled after switching to P2P";
}

// Test 29: Switching from P2P to SFU updates field visibility
TEST_F(SettingsDialogTest, SwitchingFromP2pToSfuUpdatesVisibility) {
    SettingsDialog dialog(nullptr);

    // Start with P2P mode
    dialog.setConnectionMode("P2P");
    QLineEdit* serverUrl = dialog.findChild<QLineEdit*>("serverUrlEdit");
    QLineEdit* sessionId = dialog.findChild<QLineEdit*>("sessionIdEdit");

    ASSERT_NE(sessionId, nullptr);
    EXPECT_TRUE(sessionId->isEnabled());

    // Switch to SFU mode
    dialog.setConnectionMode("SFU");

    ASSERT_NE(serverUrl, nullptr);
    EXPECT_TRUE(serverUrl->isEnabled()) << "Server URL should be enabled after switching to SFU";
    if (sessionId) {
        EXPECT_FALSE(sessionId->isEnabled()) << "Session ID should be disabled after switching to SFU";
    }
}

// Test 30: P2P mode validates session ID is not empty
TEST_F(SettingsDialogTest, P2pModeValidatesSessionIdNotEmpty) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");
    dialog.setSessionId("");

    // In P2P mode, session ID should be required
    EXPECT_FALSE(dialog.validateSettings()) << "Validation should fail with empty session ID in P2P mode";
}

// Test 31: P2P mode does not require server URL
TEST_F(SettingsDialogTest, P2pModeDoesNotRequireServerUrl) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");
    dialog.setServerUrl("");
    dialog.setSessionId("valid-session-id");

    // In P2P mode, server URL is optional
    EXPECT_TRUE(dialog.validateSettings()) << "Validation should pass without server URL in P2P mode";
}

// Issue #16 Tests: Session ID Display/Input UI Implementation

// Test 32: Can generate session ID
TEST_F(SettingsDialogTest, CanGenerateSessionId) {
    SettingsDialog dialog(nullptr);

    QString generatedId = dialog.generateSessionId();

    EXPECT_FALSE(generatedId.isEmpty()) << "Generated session ID should not be empty";
    EXPECT_GE(generatedId.length(), 8) << "Session ID should be at least 8 characters";
}

// Test 33: Generated session IDs are unique
TEST_F(SettingsDialogTest, GeneratedSessionIdsAreUnique) {
    SettingsDialog dialog(nullptr);

    QString id1 = dialog.generateSessionId();
    QString id2 = dialog.generateSessionId();
    QString id3 = dialog.generateSessionId();

    EXPECT_NE(id1, id2) << "First and second session IDs should be different";
    EXPECT_NE(id2, id3) << "Second and third session IDs should be different";
    EXPECT_NE(id1, id3) << "First and third session IDs should be different";
}

// Test 34: Session ID contains only valid characters
TEST_F(SettingsDialogTest, SessionIdContainsValidCharacters) {
    SettingsDialog dialog(nullptr);

    QString sessionId = dialog.generateSessionId();

    // Should only contain alphanumeric characters and hyphens
    QRegularExpression validPattern("^[A-Za-z0-9-]+$");
    EXPECT_TRUE(validPattern.match(sessionId).hasMatch())
        << "Session ID should only contain alphanumeric characters and hyphens";
}

// Test 35: Has generate session ID button
TEST_F(SettingsDialogTest, HasGenerateSessionIdButton) {
    SettingsDialog dialog(nullptr);

    QPushButton* generateButton = dialog.findChild<QPushButton*>("generateSessionIdButton");
    ASSERT_NE(generateButton, nullptr) << "Generate session ID button should exist";
    EXPECT_FALSE(generateButton->text().isEmpty()) << "Button should have text";
}

// Test 36: Has copy session ID button
TEST_F(SettingsDialogTest, HasCopySessionIdButton) {
    SettingsDialog dialog(nullptr);

    QPushButton* copyButton = dialog.findChild<QPushButton*>("copySessionIdButton");
    ASSERT_NE(copyButton, nullptr) << "Copy session ID button should exist";
    EXPECT_FALSE(copyButton->text().isEmpty()) << "Button should have text";
}

// Test 37: Generate button creates new session ID
TEST_F(SettingsDialogTest, GenerateButtonCreatesNewSessionId) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");

    // Set initial session ID
    dialog.setSessionId("initial-session-id");
    QString initialId = dialog.getSessionId();

    // Click generate button
    QPushButton* generateButton = dialog.findChild<QPushButton*>("generateSessionIdButton");
    ASSERT_NE(generateButton, nullptr);

    // Simulate button click
    generateButton->click();

    QString newId = dialog.getSessionId();
    EXPECT_NE(initialId, newId) << "Session ID should change after clicking generate button";
    EXPECT_FALSE(newId.isEmpty()) << "New session ID should not be empty";
}

// Test 38: Copy button copies session ID to clipboard
TEST_F(SettingsDialogTest, CopyButtonCopiesSessionIdToClipboard) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");

    QString testSessionId = "test-session-12345";
    dialog.setSessionId(testSessionId);

    QPushButton* copyButton = dialog.findChild<QPushButton*>("copySessionIdButton");
    ASSERT_NE(copyButton, nullptr);

    // Click copy button
    copyButton->click();

    // Check clipboard content
    QClipboard* clipboard = QGuiApplication::clipboard();
    EXPECT_EQ(clipboard->text(), testSessionId) << "Clipboard should contain the session ID";
}

// Test 39: Generate and copy buttons visible in P2P mode
TEST_F(SettingsDialogTest, GenerateAndCopyButtonsVisibleInP2pMode) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");

    QPushButton* generateButton = dialog.findChild<QPushButton*>("generateSessionIdButton");
    QPushButton* copyButton = dialog.findChild<QPushButton*>("copySessionIdButton");

    ASSERT_NE(generateButton, nullptr);
    ASSERT_NE(copyButton, nullptr);

    EXPECT_TRUE(generateButton->isVisible()) << "Generate button should be visible in P2P mode";
    EXPECT_TRUE(generateButton->isEnabled()) << "Generate button should be enabled in P2P mode";
    EXPECT_TRUE(copyButton->isVisible()) << "Copy button should be visible in P2P mode";
    EXPECT_TRUE(copyButton->isEnabled()) << "Copy button should be enabled in P2P mode";
}

// Test 40: Generate and copy buttons hidden in SFU mode
TEST_F(SettingsDialogTest, GenerateAndCopyButtonsHiddenInSfuMode) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("SFU");

    QPushButton* generateButton = dialog.findChild<QPushButton*>("generateSessionIdButton");
    QPushButton* copyButton = dialog.findChild<QPushButton*>("copySessionIdButton");

    if (generateButton) {
        EXPECT_FALSE(generateButton->isVisible()) << "Generate button should be hidden in SFU mode";
    }
    if (copyButton) {
        EXPECT_FALSE(copyButton->isVisible()) << "Copy button should be hidden in SFU mode";
    }
}

// Test 41: Session ID auto-generated when switching to P2P mode
TEST_F(SettingsDialogTest, SessionIdAutoGeneratedWhenSwitchingToP2pMode) {
    SettingsDialog dialog(nullptr);

    // Start in SFU mode
    dialog.setConnectionMode("SFU");
    EXPECT_TRUE(dialog.getSessionId().isEmpty() || !dialog.getSessionId().isEmpty());

    // Switch to P2P mode
    dialog.setConnectionMode("P2P");

    // Session ID should be auto-generated
    QString sessionId = dialog.getSessionId();
    EXPECT_FALSE(sessionId.isEmpty()) << "Session ID should be auto-generated when switching to P2P mode";
    EXPECT_GE(sessionId.length(), 8) << "Auto-generated session ID should be at least 8 characters";
}

// Test 42: Copy button disabled when session ID is empty
TEST_F(SettingsDialogTest, CopyButtonDisabledWhenSessionIdEmpty) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");
    dialog.setSessionId("");

    QPushButton* copyButton = dialog.findChild<QPushButton*>("copySessionIdButton");
    ASSERT_NE(copyButton, nullptr);

    EXPECT_FALSE(copyButton->isEnabled()) << "Copy button should be disabled when session ID is empty";
}

// Test 43: Copy button enabled when session ID is not empty
TEST_F(SettingsDialogTest, CopyButtonEnabledWhenSessionIdNotEmpty) {
    SettingsDialog dialog(nullptr);
    dialog.setConnectionMode("P2P");
    dialog.setSessionId("valid-session-id");

    QPushButton* copyButton = dialog.findChild<QPushButton*>("copySessionIdButton");
    ASSERT_NE(copyButton, nullptr);

    EXPECT_TRUE(copyButton->isEnabled()) << "Copy button should be enabled when session ID is not empty";
}

// Issue #17 Tests: Connection Status Display Implementation

// Test 44: Has connection status indicator
TEST_F(SettingsDialogTest, HasConnectionStatusIndicator) {
    SettingsDialog dialog(nullptr);

    QLabel* statusIndicator = dialog.findChild<QLabel*>("connectionStatusIndicator");
    ASSERT_NE(statusIndicator, nullptr) << "Connection status indicator should exist";
    EXPECT_FALSE(statusIndicator->text().isEmpty()) << "Status indicator should have text";
}

// Test 45: Can set connection status to Disconnected
TEST_F(SettingsDialogTest, CanSetConnectionStatusDisconnected) {
    SettingsDialog dialog(nullptr);

    dialog.setConnectionStatus("Disconnected");

    QString status = dialog.getConnectionStatus();
    EXPECT_EQ(status, "Disconnected");
}

// Test 46: Can set connection status to Connecting
TEST_F(SettingsDialogTest, CanSetConnectionStatusConnecting) {
    SettingsDialog dialog(nullptr);

    dialog.setConnectionStatus("Connecting");

    QString status = dialog.getConnectionStatus();
    EXPECT_EQ(status, "Connecting");
}

// Test 47: Can set connection status to Connected
TEST_F(SettingsDialogTest, CanSetConnectionStatusConnected) {
    SettingsDialog dialog(nullptr);

    dialog.setConnectionStatus("Connected");

    QString status = dialog.getConnectionStatus();
    EXPECT_EQ(status, "Connected");
}

// Test 48: Status indicator color changes based on status
TEST_F(SettingsDialogTest, StatusIndicatorColorChangesWithStatus) {
    SettingsDialog dialog(nullptr);

    QLabel* statusIndicator = dialog.findChild<QLabel*>("connectionStatusIndicator");
    ASSERT_NE(statusIndicator, nullptr);

    // Test Disconnected (red)
    dialog.setConnectionStatus("Disconnected");
    QString disconnectedStyle = statusIndicator->styleSheet();
    EXPECT_TRUE(disconnectedStyle.contains("red") || disconnectedStyle.contains("#") || disconnectedStyle.contains("rgb"))
        << "Disconnected status should have color styling";

    // Test Connecting (yellow/amber)
    dialog.setConnectionStatus("Connecting");
    QString connectingStyle = statusIndicator->styleSheet();
    EXPECT_TRUE(connectingStyle.contains("yellow") || connectingStyle.contains("#") || connectingStyle.contains("rgb"))
        << "Connecting status should have color styling";

    // Test Connected (green)
    dialog.setConnectionStatus("Connected");
    QString connectedStyle = statusIndicator->styleSheet();
    EXPECT_TRUE(connectedStyle.contains("green") || connectedStyle.contains("#") || connectedStyle.contains("rgb"))
        << "Connected status should have color styling";
}

// Test 49: Has connection statistics display area
TEST_F(SettingsDialogTest, HasConnectionStatisticsDisplay) {
    SettingsDialog dialog(nullptr);

    QLabel* statsDisplay = dialog.findChild<QLabel*>("connectionStatsLabel");
    ASSERT_NE(statsDisplay, nullptr) << "Connection statistics display should exist";
}

// Test 50: Can update connection statistics
TEST_F(SettingsDialogTest, CanUpdateConnectionStatistics) {
    SettingsDialog dialog(nullptr);

    dialog.updateConnectionStats(2500, 0.5);

    QLabel* statsLabel = dialog.findChild<QLabel*>("connectionStatsLabel");
    ASSERT_NE(statsLabel, nullptr);

    QString statsText = statsLabel->text();
    EXPECT_TRUE(statsText.contains("2500") || statsText.contains("2.5"))
        << "Stats should display bitrate";
    EXPECT_TRUE(statsText.contains("0.5") || statsText.contains("0.50"))
        << "Stats should display packet loss";
}

// Test 51: Has error message display area
TEST_F(SettingsDialogTest, HasErrorMessageDisplayArea) {
    SettingsDialog dialog(nullptr);

    QLabel* errorDisplay = dialog.findChild<QLabel*>("connectionErrorLabel");
    ASSERT_NE(errorDisplay, nullptr) << "Error message display area should exist";
}

// Test 52: Can display error message
TEST_F(SettingsDialogTest, CanDisplayErrorMessage) {
    SettingsDialog dialog(nullptr);

    QString errorMsg = "Failed to connect to server";
    dialog.setConnectionError(errorMsg);

    QLabel* errorLabel = dialog.findChild<QLabel*>("connectionErrorLabel");
    ASSERT_NE(errorLabel, nullptr);

    EXPECT_TRUE(errorLabel->isVisible()) << "Error label should be visible when error is set";
    EXPECT_EQ(errorLabel->text(), errorMsg) << "Error label should display the error message";
}

// Test 53: Error message display is hidden by default
TEST_F(SettingsDialogTest, ErrorMessageHiddenByDefault) {
    SettingsDialog dialog(nullptr);

    QLabel* errorLabel = dialog.findChild<QLabel*>("connectionErrorLabel");
    ASSERT_NE(errorLabel, nullptr);

    EXPECT_FALSE(errorLabel->isVisible()) << "Error label should be hidden by default";
}

// Test 54: Can clear error message
TEST_F(SettingsDialogTest, CanClearErrorMessage) {
    SettingsDialog dialog(nullptr);

    // First set an error
    dialog.setConnectionError("Test error");

    // Then clear it
    dialog.clearConnectionError();

    QLabel* errorLabel = dialog.findChild<QLabel*>("connectionErrorLabel");
    ASSERT_NE(errorLabel, nullptr);

    EXPECT_FALSE(errorLabel->isVisible()) << "Error label should be hidden after clearing";
    EXPECT_TRUE(errorLabel->text().isEmpty()) << "Error text should be empty after clearing";
}

// Test 55: Default connection status is Disconnected
TEST_F(SettingsDialogTest, DefaultConnectionStatusIsDisconnected) {
    SettingsDialog dialog(nullptr);

    QString status = dialog.getConnectionStatus();
    EXPECT_EQ(status, "Disconnected") << "Default connection status should be Disconnected";
}

// Test 56: Connection statistics display shows zero values initially
TEST_F(SettingsDialogTest, ConnectionStatsShowZeroInitially) {
    SettingsDialog dialog(nullptr);

    QLabel* statsLabel = dialog.findChild<QLabel*>("connectionStatsLabel");
    ASSERT_NE(statsLabel, nullptr);

    QString statsText = statsLabel->text();
    EXPECT_TRUE(statsText.contains("0")) << "Initial stats should show zero values";
}

// Test 57: Audio-only mode checkbox exists
TEST_F(SettingsDialogTest, AudioOnlyModeCheckboxExists) {
    SettingsDialog dialog(nullptr);

    QCheckBox* audioOnlyCheckbox = dialog.findChild<QCheckBox*>("audioOnlyModeCheckbox");
    ASSERT_NE(audioOnlyCheckbox, nullptr) << "Audio-only mode checkbox should exist";
}

// Test 58: Audio-only mode is disabled by default
TEST_F(SettingsDialogTest, AudioOnlyModeDisabledByDefault) {
    SettingsDialog dialog(nullptr);

    EXPECT_FALSE(dialog.isAudioOnlyMode()) << "Audio-only mode should be disabled by default";
}

// Test 59: Can enable audio-only mode
TEST_F(SettingsDialogTest, CanEnableAudioOnlyMode) {
    SettingsDialog dialog(nullptr);

    dialog.setAudioOnlyMode(true);
    EXPECT_TRUE(dialog.isAudioOnlyMode()) << "Audio-only mode should be enabled";
}

// Test 60: Audio quality preset combo box exists
TEST_F(SettingsDialogTest, AudioQualityPresetComboExists) {
    SettingsDialog dialog(nullptr);

    QComboBox* qualityCombo = dialog.findChild<QComboBox*>("audioQualityPresetCombo");
    ASSERT_NE(qualityCombo, nullptr) << "Audio quality preset combo should exist";
    EXPECT_EQ(qualityCombo->count(), 3) << "Should have 3 quality presets (Low, Medium, High)";
}

// Test 61: Default audio quality preset is Medium
TEST_F(SettingsDialogTest, DefaultAudioQualityPresetIsMedium) {
    SettingsDialog dialog(nullptr);

    QString preset = dialog.getAudioQualityPreset();
    EXPECT_EQ(preset, "medium") << "Default audio quality preset should be Medium";
}

// Test 62: Can set audio quality preset to High
TEST_F(SettingsDialogTest, CanSetAudioQualityPresetHigh) {
    SettingsDialog dialog(nullptr);

    dialog.setAudioQualityPreset("high");
    EXPECT_EQ(dialog.getAudioQualityPreset(), "high") << "Audio quality preset should be High";
}

// Test 63: Can set audio quality preset to Low
TEST_F(SettingsDialogTest, CanSetAudioQualityPresetLow) {
    SettingsDialog dialog(nullptr);

    dialog.setAudioQualityPreset("low");
    EXPECT_EQ(dialog.getAudioQualityPreset(), "low") << "Audio quality preset should be Low";
}

// Test 64: Echo cancellation checkbox exists
TEST_F(SettingsDialogTest, EchoCancellationCheckboxExists) {
    SettingsDialog dialog(nullptr);

    QCheckBox* echoCheckbox = dialog.findChild<QCheckBox*>("echoCancellationCheckbox");
    ASSERT_NE(echoCheckbox, nullptr) << "Echo cancellation checkbox should exist";
}

// Test 65: Echo cancellation is enabled by default
TEST_F(SettingsDialogTest, EchoCancellationEnabledByDefault) {
    SettingsDialog dialog(nullptr);

    EXPECT_TRUE(dialog.isEchoCancellationEnabled()) << "Echo cancellation should be enabled by default";
}

// Test 66: Can disable echo cancellation
TEST_F(SettingsDialogTest, CanDisableEchoCancellation) {
    SettingsDialog dialog(nullptr);

    dialog.setEchoCancellation(false);
    EXPECT_FALSE(dialog.isEchoCancellationEnabled()) << "Echo cancellation should be disabled";
}

// Test 67: Noise suppression checkbox exists
TEST_F(SettingsDialogTest, NoiseSuppressionCheckboxExists) {
    SettingsDialog dialog(nullptr);

    QCheckBox* noiseCheckbox = dialog.findChild<QCheckBox*>("noiseSuppressionCheckbox");
    ASSERT_NE(noiseCheckbox, nullptr) << "Noise suppression checkbox should exist";
}

// Test 68: Noise suppression is enabled by default
TEST_F(SettingsDialogTest, NoiseSuppressionEnabledByDefault) {
    SettingsDialog dialog(nullptr);

    EXPECT_TRUE(dialog.isNoiseSuppressionEnabled()) << "Noise suppression should be enabled by default";
}

// Test 69: Can disable noise suppression
TEST_F(SettingsDialogTest, CanDisableNoiseSuppression) {
    SettingsDialog dialog(nullptr);

    dialog.setNoiseSuppression(false);
    EXPECT_FALSE(dialog.isNoiseSuppressionEnabled()) << "Noise suppression should be disabled";
}

// Test 70: Automatic gain control checkbox exists
TEST_F(SettingsDialogTest, AutomaticGainControlCheckboxExists) {
    SettingsDialog dialog(nullptr);

    QCheckBox* agcCheckbox = dialog.findChild<QCheckBox*>("automaticGainControlCheckbox");
    ASSERT_NE(agcCheckbox, nullptr) << "Automatic gain control checkbox should exist";
}

// Test 71: Automatic gain control is disabled by default
TEST_F(SettingsDialogTest, AutomaticGainControlDisabledByDefault) {
    SettingsDialog dialog(nullptr);

    EXPECT_FALSE(dialog.isAutomaticGainControlEnabled()) << "Automatic gain control should be disabled by default";
}

// Test 72: Can enable automatic gain control
TEST_F(SettingsDialogTest, CanEnableAutomaticGainControl) {
    SettingsDialog dialog(nullptr);

    dialog.setAutomaticGainControl(true);
    EXPECT_TRUE(dialog.isAutomaticGainControlEnabled()) << "Automatic gain control should be enabled";
}

// Test 73: Audio-only settings group box exists
TEST_F(SettingsDialogTest, AudioOnlySettingsGroupBoxExists) {
    SettingsDialog dialog(nullptr);

    QGroupBox* groupBox = dialog.findChild<QGroupBox*>("audioOnlyGroupBox");
    ASSERT_NE(groupBox, nullptr) << "Audio-only settings group box should exist";
}

// Test 74: Audio-only settings group box is hidden by default
TEST_F(SettingsDialogTest, AudioOnlySettingsGroupBoxHiddenByDefault) {
    SettingsDialog dialog(nullptr);

    QGroupBox* groupBox = dialog.findChild<QGroupBox*>("audioOnlyGroupBox");
    ASSERT_NE(groupBox, nullptr);

    EXPECT_FALSE(groupBox->isVisible()) << "Audio-only settings group box should be hidden by default";
}

// Test 75: Enabling audio-only mode shows settings group box
TEST_F(SettingsDialogTest, EnablingAudioOnlyModeShowsGroupBox) {
    SettingsDialog dialog(nullptr);

    QGroupBox* groupBox = dialog.findChild<QGroupBox*>("audioOnlyGroupBox");
    ASSERT_NE(groupBox, nullptr);

    dialog.setAudioOnlyMode(true);
    EXPECT_TRUE(groupBox->isVisible()) << "Audio-only settings group box should be visible when audio-only mode is enabled";
}

// Test 76: Enabling audio-only mode disables video settings
TEST_F(SettingsDialogTest, EnablingAudioOnlyModeDisablesVideoSettings) {
    SettingsDialog dialog(nullptr);

    QComboBox* videoCodec = dialog.findChild<QComboBox*>("videoCodecCombo");
    QSpinBox* videoBitrate = dialog.findChild<QSpinBox*>("videoBitrateSpin");
    ASSERT_NE(videoCodec, nullptr);
    ASSERT_NE(videoBitrate, nullptr);

    dialog.setAudioOnlyMode(true);
    EXPECT_FALSE(videoCodec->isEnabled()) << "Video codec should be disabled in audio-only mode";
    EXPECT_FALSE(videoBitrate->isEnabled()) << "Video bitrate should be disabled in audio-only mode";
}

} // namespace
