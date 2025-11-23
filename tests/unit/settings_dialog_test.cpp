// Copyright (c) 2025 OBS-WebRTC-Link Project
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
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

} // namespace
