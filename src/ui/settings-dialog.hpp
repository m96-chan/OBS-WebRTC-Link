// Copyright (c) 2025 OBS-WebRTC-Link Project
// SPDX-License-Identifier: MIT

#pragma once

#include <QDialog>
#include <QString>
#include <memory>

class QComboBox;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QDialogButtonBox;
class QCheckBox;
class QGroupBox;

/**
 * @brief Settings dialog for WebRTC connection configuration
 *
 * This dialog allows users to configure WebRTC connection settings including:
 * - Connection mode (SFU/P2P)
 * - Server URL
 * - Authentication token
 * - Video codec and bitrate
 * - Audio codec and bitrate
 */
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    /**
     * @brief Construct a new Settings Dialog
     * @param parent Parent widget
     */
    explicit SettingsDialog(QWidget* parent = nullptr);

    /**
     * @brief Destroy the Settings Dialog
     */
    ~SettingsDialog() override;

    // Getters
    QString getServerUrl() const;
    QString getConnectionMode() const;
    QString getVideoCodec() const;
    QString getAudioCodec() const;
    int getVideoBitrate() const;
    int getAudioBitrate() const;
    QString getToken() const;
    QString getSessionId() const;
    QString getConnectionStatus() const;
    bool isAudioOnlyMode() const;
    QString getAudioQualityPreset() const;
    bool isEchoCancellationEnabled() const;
    bool isNoiseSuppressionEnabled() const;
    bool isAutomaticGainControlEnabled() const;

    // Setters
    void setServerUrl(const QString& url);
    void setConnectionMode(const QString& mode);
    void setVideoCodec(const QString& codec);
    void setAudioCodec(const QString& codec);
    void setVideoBitrate(int bitrate);
    void setAudioBitrate(int bitrate);
    void setToken(const QString& token);
    void setSessionId(const QString& sessionId);
    void setConnectionStatus(const QString& status);
    void setConnectionError(const QString& error);
    void clearConnectionError();
    void updateConnectionStats(int bitrateKbps, double packetLossPercent);
    void setAudioOnlyMode(bool enabled);
    void setAudioQualityPreset(const QString& preset);
    void setEchoCancellation(bool enabled);
    void setNoiseSuppression(bool enabled);
    void setAutomaticGainControl(bool enabled);

    /**
     * @brief Validate all settings
     * @return true if all settings are valid, false otherwise
     */
    bool validateSettings() const;

    /**
     * @brief Generate a new session ID
     * @return Newly generated session ID string
     */
    QString generateSessionId();

private slots:
    /**
     * @brief Handle OK button click
     */
    void onAccepted();

    /**
     * @brief Handle Cancel button click
     */
    void onRejected();

    /**
     * @brief Handle connection mode change
     * @param index Current index of connection mode combo box
     */
    void onConnectionModeChanged(int index);

    /**
     * @brief Handle generate session ID button click
     */
    void onGenerateSessionId();

    /**
     * @brief Handle copy session ID button click
     */
    void onCopySessionId();

    /**
     * @brief Update copy button enabled state based on session ID
     */
    void updateCopyButtonState();

    /**
     * @brief Handle audio-only mode checkbox change
     * @param checked true if audio-only mode is enabled
     */
    void onAudioOnlyModeChanged(bool checked);

    /**
     * @brief Handle audio quality preset change
     * @param index Current index of audio quality preset combo box
     */
    void onAudioQualityPresetChanged(int index);

private:
    /**
     * @brief Initialize the UI components
     */
    void setupUi();

    /**
     * @brief Create form layout with all settings fields
     * @return Form layout with all settings
     */
    QFormLayout* createFormLayout();

    /**
     * @brief Create button box with OK and Cancel buttons
     * @return Dialog button box
     */
    QDialogButtonBox* createButtonBox();

    /**
     * @brief Validate URL format
     * @param url URL to validate
     * @return true if URL is valid, false otherwise
     */
    bool isValidUrl(const QString& url) const;

    /**
     * @brief Show validation error message
     * @param message Error message to display
     */
    void showValidationError(const QString& message);

    // UI components
    QComboBox* connectionModeCombo_;
    QLineEdit* serverUrlEdit_;
    QLineEdit* tokenEdit_;
    QLineEdit* sessionIdEdit_;
    QComboBox* videoCodecCombo_;
    QSpinBox* videoBitrateSpin_;
    QComboBox* audioCodecCombo_;
    QSpinBox* audioBitrateSpin_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QLabel* errorLabel_;

    // Session ID management buttons
    QPushButton* generateSessionIdButton_;
    QPushButton* copySessionIdButton_;

    // Labels for dynamic show/hide
    QLabel* serverUrlLabel_;
    QLabel* tokenLabel_;
    QLabel* sessionIdLabel_;
    QLabel* videoCodecLabel_;
    QLabel* videoBitrateLabel_;

    // Connection status display components
    QLabel* connectionStatusIndicator_;
    QLabel* connectionStatsLabel_;
    QLabel* connectionErrorLabel_;
    QString currentConnectionStatus_;

    // Audio-only mode components
    QCheckBox* audioOnlyModeCheckbox_;
    QComboBox* audioQualityPresetCombo_;
    QCheckBox* echoCancellationCheckbox_;
    QCheckBox* noiseSuppressionCheckbox_;
    QCheckBox* automaticGainControlCheckbox_;
    QGroupBox* audioOnlyGroupBox_;
    QLabel* audioQualityPresetLabel_;

    // Layouts
    QVBoxLayout* mainLayout_;
    QFormLayout* formLayout_;
};
