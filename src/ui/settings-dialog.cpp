// Copyright (c) 2025 OBS-WebRTC-Link Project
// SPDX-License-Identifier: MIT

#include "settings-dialog.hpp"
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QUrl>
#include <QUuid>
#include <QClipboard>
#include <QGuiApplication>
#include <QCheckBox>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent),
      connectionModeCombo_(nullptr),
      serverUrlEdit_(nullptr),
      tokenEdit_(nullptr),
      sessionIdEdit_(nullptr),
      videoCodecCombo_(nullptr),
      videoBitrateSpin_(nullptr),
      audioCodecCombo_(nullptr),
      audioBitrateSpin_(nullptr),
      okButton_(nullptr),
      cancelButton_(nullptr),
      errorLabel_(nullptr),
      generateSessionIdButton_(nullptr),
      copySessionIdButton_(nullptr),
      serverUrlLabel_(nullptr),
      tokenLabel_(nullptr),
      sessionIdLabel_(nullptr),
      videoCodecLabel_(nullptr),
      videoBitrateLabel_(nullptr),
      connectionStatusIndicator_(nullptr),
      connectionStatsLabel_(nullptr),
      connectionErrorLabel_(nullptr),
      currentConnectionStatus_("Disconnected"),
      audioOnlyModeCheckbox_(nullptr),
      audioQualityPresetCombo_(nullptr),
      echoCancellationCheckbox_(nullptr),
      noiseSuppressionCheckbox_(nullptr),
      automaticGainControlCheckbox_(nullptr),
      audioOnlyGroupBox_(nullptr),
      audioQualityPresetLabel_(nullptr),
      mainLayout_(nullptr),
      formLayout_(nullptr) {
    setupUi();
    setWindowTitle(tr("WebRTC Link Settings - v") + QString(PLUGIN_VERSION));
    setMinimumWidth(400);
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUi() {
    mainLayout_ = new QVBoxLayout(this);

    // Connection status indicator
    connectionStatusIndicator_ = new QLabel(this);
    connectionStatusIndicator_->setObjectName("connectionStatusIndicator");
    connectionStatusIndicator_->setText(tr("Status: Disconnected"));
    connectionStatusIndicator_->setStyleSheet("QLabel { color: red; font-weight: bold; padding: 5px; }");
    mainLayout_->addWidget(connectionStatusIndicator_);

    // Connection statistics label
    connectionStatsLabel_ = new QLabel(this);
    connectionStatsLabel_->setObjectName("connectionStatsLabel");
    connectionStatsLabel_->setText(tr("Bitrate: 0 kbps | Packet Loss: 0.00%"));
    connectionStatsLabel_->setStyleSheet("QLabel { padding: 5px; }");
    mainLayout_->addWidget(connectionStatsLabel_);

    // Connection error label (hidden by default)
    connectionErrorLabel_ = new QLabel(this);
    connectionErrorLabel_->setObjectName("connectionErrorLabel");
    connectionErrorLabel_->setStyleSheet("QLabel { color: red; padding: 5px; }");
    connectionErrorLabel_->setWordWrap(true);
    connectionErrorLabel_->hide();
    mainLayout_->addWidget(connectionErrorLabel_);

    // Create form layout
    formLayout_ = createFormLayout();
    mainLayout_->addLayout(formLayout_);

    // Error label (hidden by default)
    errorLabel_ = new QLabel(this);
    errorLabel_->setStyleSheet("QLabel { color: red; }");
    errorLabel_->setWordWrap(true);
    errorLabel_->hide();
    mainLayout_->addWidget(errorLabel_);

    // Button box
    QDialogButtonBox* buttonBox = createButtonBox();
    mainLayout_->addWidget(buttonBox);

    setLayout(mainLayout_);

    // Connect signals
    connect(connectionModeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onConnectionModeChanged);
    connect(generateSessionIdButton_, &QPushButton::clicked,
            this, &SettingsDialog::onGenerateSessionId);
    connect(copySessionIdButton_, &QPushButton::clicked,
            this, &SettingsDialog::onCopySessionId);
    connect(sessionIdEdit_, &QLineEdit::textChanged,
            this, &SettingsDialog::updateCopyButtonState);
    connect(audioOnlyModeCheckbox_, &QCheckBox::toggled,
            this, &SettingsDialog::onAudioOnlyModeChanged);
    connect(audioQualityPresetCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onAudioQualityPresetChanged);

    // Initialize copy button state
    updateCopyButtonState();
}

QFormLayout* SettingsDialog::createFormLayout() {
    QFormLayout* layout = new QFormLayout();

    // Connection mode
    connectionModeCombo_ = new QComboBox(this);
    connectionModeCombo_->setObjectName("connectionModeCombo");
    connectionModeCombo_->addItem(tr("SFU (Selective Forwarding Unit)"), "SFU");
    connectionModeCombo_->addItem(tr("P2P (Peer-to-Peer)"), "P2P");
    layout->addRow(tr("Connection Mode:"), connectionModeCombo_);

    // Server URL (SFU mode only)
    serverUrlLabel_ = new QLabel(tr("Server URL:"), this);
    serverUrlEdit_ = new QLineEdit(this);
    serverUrlEdit_->setObjectName("serverUrlEdit");
    serverUrlEdit_->setPlaceholderText(tr("https://example.com/webrtc"));
    layout->addRow(serverUrlLabel_, serverUrlEdit_);

    // Token (SFU mode only)
    tokenLabel_ = new QLabel(tr("Token:"), this);
    tokenEdit_ = new QLineEdit(this);
    tokenEdit_->setObjectName("tokenEdit");
    tokenEdit_->setPlaceholderText(tr("Authentication token (optional)"));
    tokenEdit_->setEchoMode(QLineEdit::Password);
    layout->addRow(tokenLabel_, tokenEdit_);

    // Session ID (P2P mode only)
    sessionIdLabel_ = new QLabel(tr("Session ID:"), this);
    sessionIdEdit_ = new QLineEdit(this);
    sessionIdEdit_->setObjectName("sessionIdEdit");
    sessionIdEdit_->setPlaceholderText(tr("Enter or generate session ID"));

    // Create horizontal layout for session ID with buttons
    QHBoxLayout* sessionIdLayout = new QHBoxLayout();
    sessionIdLayout->addWidget(sessionIdEdit_);

    // Generate button
    generateSessionIdButton_ = new QPushButton(tr("Generate"), this);
    generateSessionIdButton_->setObjectName("generateSessionIdButton");
    sessionIdLayout->addWidget(generateSessionIdButton_);

    // Copy button
    copySessionIdButton_ = new QPushButton(tr("Copy"), this);
    copySessionIdButton_->setObjectName("copySessionIdButton");
    sessionIdLayout->addWidget(copySessionIdButton_);

    layout->addRow(sessionIdLabel_, sessionIdLayout);

    // Initially hide session ID (default mode is SFU)
    sessionIdLabel_->setVisible(false);
    sessionIdEdit_->setVisible(false);
    sessionIdEdit_->setEnabled(false);
    generateSessionIdButton_->setVisible(false);
    copySessionIdButton_->setVisible(false);

    // Audio-only mode checkbox
    audioOnlyModeCheckbox_ = new QCheckBox(tr("Enable Audio-Only Mode"), this);
    audioOnlyModeCheckbox_->setObjectName("audioOnlyModeCheckbox");
    audioOnlyModeCheckbox_->setToolTip(tr("Disable video transmission and only send audio"));
    layout->addRow("", audioOnlyModeCheckbox_);

    // Video codec
    videoCodecLabel_ = new QLabel(tr("Video Codec:"), this);
    videoCodecCombo_ = new QComboBox(this);
    videoCodecCombo_->setObjectName("videoCodecCombo");
    videoCodecCombo_->addItem(tr("H.264"), "h264");
    videoCodecCombo_->addItem(tr("VP8"), "vp8");
    videoCodecCombo_->addItem(tr("VP9"), "vp9");
    videoCodecCombo_->addItem(tr("AV1"), "av1");
    layout->addRow(videoCodecLabel_, videoCodecCombo_);

    // Video bitrate
    videoBitrateLabel_ = new QLabel(tr("Video Bitrate:"), this);
    videoBitrateSpin_ = new QSpinBox(this);
    videoBitrateSpin_->setObjectName("videoBitrateSpin");
    videoBitrateSpin_->setMinimum(100);
    videoBitrateSpin_->setMaximum(50000);
    videoBitrateSpin_->setSingleStep(100);
    videoBitrateSpin_->setValue(2500);
    videoBitrateSpin_->setSuffix(tr(" kbps"));
    layout->addRow(videoBitrateLabel_, videoBitrateSpin_);

    // Audio codec
    audioCodecCombo_ = new QComboBox(this);
    audioCodecCombo_->setObjectName("audioCodecCombo");
    audioCodecCombo_->addItem(tr("Opus"), "opus");
    audioCodecCombo_->addItem(tr("AAC"), "aac");
    layout->addRow(tr("Audio Codec:"), audioCodecCombo_);

    // Audio bitrate
    audioBitrateSpin_ = new QSpinBox(this);
    audioBitrateSpin_->setObjectName("audioBitrateSpin");
    audioBitrateSpin_->setMinimum(16);
    audioBitrateSpin_->setMaximum(512);
    audioBitrateSpin_->setSingleStep(8);
    audioBitrateSpin_->setValue(128);
    audioBitrateSpin_->setSuffix(tr(" kbps"));
    layout->addRow(tr("Audio Bitrate:"), audioBitrateSpin_);

    // Audio-only mode group box
    audioOnlyGroupBox_ = new QGroupBox(tr("Audio-Only Mode Settings"), this);
    audioOnlyGroupBox_->setObjectName("audioOnlyGroupBox");
    QFormLayout* audioOnlyLayout = new QFormLayout(audioOnlyGroupBox_);

    // Audio quality preset
    audioQualityPresetLabel_ = new QLabel(tr("Audio Quality:"), this);
    audioQualityPresetCombo_ = new QComboBox(this);
    audioQualityPresetCombo_->setObjectName("audioQualityPresetCombo");
    audioQualityPresetCombo_->addItem(tr("Low (32 kbps)"), "low");
    audioQualityPresetCombo_->addItem(tr("Medium (48 kbps)"), "medium");
    audioQualityPresetCombo_->addItem(tr("High (64 kbps)"), "high");
    audioQualityPresetCombo_->setCurrentIndex(1); // Default: Medium
    audioOnlyLayout->addRow(audioQualityPresetLabel_, audioQualityPresetCombo_);

    // Audio processing options
    echoCancellationCheckbox_ = new QCheckBox(tr("Echo Cancellation"), this);
    echoCancellationCheckbox_->setObjectName("echoCancellationCheckbox");
    echoCancellationCheckbox_->setChecked(true); // Enabled by default
    echoCancellationCheckbox_->setToolTip(tr("Remove echo from audio input"));
    audioOnlyLayout->addRow("", echoCancellationCheckbox_);

    noiseSuppressionCheckbox_ = new QCheckBox(tr("Noise Suppression"), this);
    noiseSuppressionCheckbox_->setObjectName("noiseSuppressionCheckbox");
    noiseSuppressionCheckbox_->setChecked(true); // Enabled by default
    noiseSuppressionCheckbox_->setToolTip(tr("Reduce background noise"));
    audioOnlyLayout->addRow("", noiseSuppressionCheckbox_);

    automaticGainControlCheckbox_ = new QCheckBox(tr("Automatic Gain Control"), this);
    automaticGainControlCheckbox_->setObjectName("automaticGainControlCheckbox");
    automaticGainControlCheckbox_->setChecked(false); // Disabled by default
    automaticGainControlCheckbox_->setToolTip(tr("Automatically adjust audio levels"));
    audioOnlyLayout->addRow("", automaticGainControlCheckbox_);

    audioOnlyGroupBox_->setLayout(audioOnlyLayout);
    audioOnlyGroupBox_->setVisible(false); // Hidden by default
    layout->addRow("", audioOnlyGroupBox_);

    return layout;
}

QDialogButtonBox* SettingsDialog::createButtonBox() {
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    okButton_ = buttonBox->button(QDialogButtonBox::Ok);
    okButton_->setObjectName("okButton");
    cancelButton_ = buttonBox->button(QDialogButtonBox::Cancel);
    cancelButton_->setObjectName("cancelButton");

    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::onRejected);

    return buttonBox;
}

// Getters
QString SettingsDialog::getServerUrl() const {
    return serverUrlEdit_ ? serverUrlEdit_->text().trimmed() : QString();
}

QString SettingsDialog::getConnectionMode() const {
    return connectionModeCombo_ ? connectionModeCombo_->currentData().toString() : QString("SFU");
}

QString SettingsDialog::getVideoCodec() const {
    return videoCodecCombo_ ? videoCodecCombo_->currentData().toString() : QString("h264");
}

QString SettingsDialog::getAudioCodec() const {
    return audioCodecCombo_ ? audioCodecCombo_->currentData().toString() : QString("opus");
}

int SettingsDialog::getVideoBitrate() const {
    return videoBitrateSpin_ ? videoBitrateSpin_->value() : 2500;
}

int SettingsDialog::getAudioBitrate() const {
    return audioBitrateSpin_ ? audioBitrateSpin_->value() : 128;
}

QString SettingsDialog::getToken() const {
    return tokenEdit_ ? tokenEdit_->text().trimmed() : QString();
}

QString SettingsDialog::getSessionId() const {
    return sessionIdEdit_ ? sessionIdEdit_->text().trimmed() : QString();
}

QString SettingsDialog::getConnectionStatus() const {
    return currentConnectionStatus_;
}

// Setters
void SettingsDialog::setServerUrl(const QString& url) {
    if (serverUrlEdit_) {
        serverUrlEdit_->setText(url);
    }
}

void SettingsDialog::setConnectionMode(const QString& mode) {
    if (connectionModeCombo_) {
        int index = connectionModeCombo_->findData(mode);
        if (index >= 0) {
            connectionModeCombo_->setCurrentIndex(index);
        }
    }
}

void SettingsDialog::setVideoCodec(const QString& codec) {
    if (videoCodecCombo_) {
        int index = videoCodecCombo_->findData(codec);
        if (index >= 0) {
            videoCodecCombo_->setCurrentIndex(index);
        }
    }
}

void SettingsDialog::setAudioCodec(const QString& codec) {
    if (audioCodecCombo_) {
        int index = audioCodecCombo_->findData(codec);
        if (index >= 0) {
            audioCodecCombo_->setCurrentIndex(index);
        }
    }
}

void SettingsDialog::setVideoBitrate(int bitrate) {
    if (videoBitrateSpin_) {
        videoBitrateSpin_->setValue(bitrate);
    }
}

void SettingsDialog::setAudioBitrate(int bitrate) {
    if (audioBitrateSpin_) {
        audioBitrateSpin_->setValue(bitrate);
    }
}

void SettingsDialog::setToken(const QString& token) {
    if (tokenEdit_) {
        tokenEdit_->setText(token);
    }
}

void SettingsDialog::setSessionId(const QString& sessionId) {
    if (sessionIdEdit_) {
        sessionIdEdit_->setText(sessionId);
    }
}

void SettingsDialog::setConnectionStatus(const QString& status) {
    currentConnectionStatus_ = status;

    if (!connectionStatusIndicator_) {
        return;
    }

    // Update text and color based on status
    connectionStatusIndicator_->setText(tr("Status: %1").arg(status));

    if (status == "Disconnected") {
        connectionStatusIndicator_->setStyleSheet(
            "QLabel { color: red; font-weight: bold; padding: 5px; }");
    } else if (status == "Connecting") {
        connectionStatusIndicator_->setStyleSheet(
            "QLabel { color: #FFA500; font-weight: bold; padding: 5px; }"); // Orange/yellow
    } else if (status == "Connected") {
        connectionStatusIndicator_->setStyleSheet(
            "QLabel { color: green; font-weight: bold; padding: 5px; }");
    }
}

void SettingsDialog::setConnectionError(const QString& error) {
    if (connectionErrorLabel_) {
        connectionErrorLabel_->setText(error);
        connectionErrorLabel_->show();
    }
}

void SettingsDialog::clearConnectionError() {
    if (connectionErrorLabel_) {
        connectionErrorLabel_->clear();
        connectionErrorLabel_->hide();
    }
}

void SettingsDialog::updateConnectionStats(int bitrateKbps, double packetLossPercent) {
    if (connectionStatsLabel_) {
        connectionStatsLabel_->setText(
            tr("Bitrate: %1 kbps | Packet Loss: %2%")
                .arg(bitrateKbps)
                .arg(packetLossPercent, 0, 'f', 2));
    }
}

// Validation
bool SettingsDialog::validateSettings() const {
    QString mode = getConnectionMode();

    if (mode == "SFU") {
        // SFU mode requires server URL
        QString url = getServerUrl();

        // Check if URL is empty
        if (url.isEmpty()) {
            return false;
        }

        // Check URL format
        if (!isValidUrl(url)) {
            return false;
        }
    } else if (mode == "P2P") {
        // P2P mode requires session ID
        QString sessionId = getSessionId();
        if (sessionId.isEmpty()) {
            return false;
        }
        // Server URL is optional in P2P mode
    }

    return true;
}

bool SettingsDialog::isValidUrl(const QString& url) const {
    QUrl qurl(url);
    if (!qurl.isValid()) {
        return false;
    }

    QString scheme = qurl.scheme().toLower();
    if (scheme != "http" && scheme != "https") {
        return false;
    }

    if (qurl.host().isEmpty()) {
        return false;
    }

    return true;
}

void SettingsDialog::showValidationError(const QString& message) {
    if (errorLabel_) {
        errorLabel_->setText(message);
        errorLabel_->show();
    }
}

// Slots
void SettingsDialog::onAccepted() {
    errorLabel_->hide();

    if (!validateSettings()) {
        QString mode = getConnectionMode();
        QString url = getServerUrl();
        QString sessionId = getSessionId();

        if (mode == "SFU") {
            if (url.isEmpty()) {
                showValidationError(tr("Error: Server URL cannot be empty in SFU mode."));
            } else if (!isValidUrl(url)) {
                showValidationError(tr("Error: Invalid server URL format. "
                                       "Please use http:// or https:// protocol."));
            }
        } else if (mode == "P2P") {
            if (sessionId.isEmpty()) {
                showValidationError(tr("Error: Session ID cannot be empty in P2P mode."));
            }
        }
        return;
    }

    accept();
}

void SettingsDialog::onRejected() {
    reject();
}

void SettingsDialog::onConnectionModeChanged(int index) {
    Q_UNUSED(index);

    QString mode = getConnectionMode();

    if (mode == "SFU") {
        // SFU mode: Show server URL and token, hide session ID
        if (serverUrlLabel_) serverUrlLabel_->setVisible(true);
        if (serverUrlEdit_) {
            serverUrlEdit_->setVisible(true);
            serverUrlEdit_->setEnabled(true);
        }

        if (tokenLabel_) tokenLabel_->setVisible(true);
        if (tokenEdit_) {
            tokenEdit_->setVisible(true);
            tokenEdit_->setEnabled(true);
        }

        if (sessionIdLabel_) sessionIdLabel_->setVisible(false);
        if (sessionIdEdit_) {
            sessionIdEdit_->setVisible(false);
            sessionIdEdit_->setEnabled(false);
        }

        // Hide session ID buttons in SFU mode
        if (generateSessionIdButton_) generateSessionIdButton_->setVisible(false);
        if (copySessionIdButton_) copySessionIdButton_->setVisible(false);
    } else if (mode == "P2P") {
        // P2P mode: Hide server URL and token, show session ID
        if (serverUrlLabel_) serverUrlLabel_->setVisible(false);
        if (serverUrlEdit_) {
            serverUrlEdit_->setVisible(false);
            serverUrlEdit_->setEnabled(false);
        }

        if (tokenLabel_) tokenLabel_->setVisible(false);
        if (tokenEdit_) {
            tokenEdit_->setVisible(false);
            tokenEdit_->setEnabled(false);
        }

        if (sessionIdLabel_) sessionIdLabel_->setVisible(true);
        if (sessionIdEdit_) {
            sessionIdEdit_->setVisible(true);
            sessionIdEdit_->setEnabled(true);
        }

        // Show session ID buttons in P2P mode
        if (generateSessionIdButton_) {
            generateSessionIdButton_->setVisible(true);
            generateSessionIdButton_->setEnabled(true);
        }
        if (copySessionIdButton_) {
            copySessionIdButton_->setVisible(true);
        }

        // Auto-generate session ID if empty
        if (sessionIdEdit_ && sessionIdEdit_->text().isEmpty()) {
            setSessionId(generateSessionId());
        }

        // Update copy button state
        updateCopyButtonState();
    }
}

QString SettingsDialog::generateSessionId() {
    // Generate a UUID and take the first 8 characters for a shorter code
    QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // Remove hyphens and take first 12 characters for readability
    QString sessionId = uuid.remove('-').left(12).toUpper();
    return sessionId;
}

void SettingsDialog::onGenerateSessionId() {
    QString newSessionId = generateSessionId();
    setSessionId(newSessionId);
}

void SettingsDialog::onCopySessionId() {
    if (!sessionIdEdit_) {
        return;
    }

    QString sessionId = getSessionId();
    if (!sessionId.isEmpty()) {
        QClipboard* clipboard = QGuiApplication::clipboard();
        clipboard->setText(sessionId);
    }
}

void SettingsDialog::updateCopyButtonState() {
    if (!copySessionIdButton_ || !sessionIdEdit_) {
        return;
    }

    bool hasSessionId = !sessionIdEdit_->text().isEmpty();
    copySessionIdButton_->setEnabled(hasSessionId);
}

// Audio-only mode getters
bool SettingsDialog::isAudioOnlyMode() const {
    return audioOnlyModeCheckbox_ ? audioOnlyModeCheckbox_->isChecked() : false;
}

QString SettingsDialog::getAudioQualityPreset() const {
    if (!audioQualityPresetCombo_) {
        return "medium";
    }
    return audioQualityPresetCombo_->currentData().toString();
}

bool SettingsDialog::isEchoCancellationEnabled() const {
    return echoCancellationCheckbox_ ? echoCancellationCheckbox_->isChecked() : true;
}

bool SettingsDialog::isNoiseSuppressionEnabled() const {
    return noiseSuppressionCheckbox_ ? noiseSuppressionCheckbox_->isChecked() : true;
}

bool SettingsDialog::isAutomaticGainControlEnabled() const {
    return automaticGainControlCheckbox_ ? automaticGainControlCheckbox_->isChecked() : false;
}

// Audio-only mode setters
void SettingsDialog::setAudioOnlyMode(bool enabled) {
    if (audioOnlyModeCheckbox_) {
        audioOnlyModeCheckbox_->setChecked(enabled);
    }
}

void SettingsDialog::setAudioQualityPreset(const QString& preset) {
    if (!audioQualityPresetCombo_) {
        return;
    }

    int index = audioQualityPresetCombo_->findData(preset);
    if (index >= 0) {
        audioQualityPresetCombo_->setCurrentIndex(index);
    }
}

void SettingsDialog::setEchoCancellation(bool enabled) {
    if (echoCancellationCheckbox_) {
        echoCancellationCheckbox_->setChecked(enabled);
    }
}

void SettingsDialog::setNoiseSuppression(bool enabled) {
    if (noiseSuppressionCheckbox_) {
        noiseSuppressionCheckbox_->setChecked(enabled);
    }
}

void SettingsDialog::setAutomaticGainControl(bool enabled) {
    if (automaticGainControlCheckbox_) {
        automaticGainControlCheckbox_->setChecked(enabled);
    }
}

// Audio-only mode slots
void SettingsDialog::onAudioOnlyModeChanged(bool checked) {
    // Show/hide audio-only settings group box
    if (audioOnlyGroupBox_) {
        audioOnlyGroupBox_->setVisible(checked);
    }

    // Enable/disable video settings
    if (videoCodecLabel_) videoCodecLabel_->setEnabled(!checked);
    if (videoCodecCombo_) videoCodecCombo_->setEnabled(!checked);
    if (videoBitrateLabel_) videoBitrateLabel_->setEnabled(!checked);
    if (videoBitrateSpin_) videoBitrateSpin_->setEnabled(!checked);

    // Update audio bitrate based on audio-only mode
    if (checked && audioBitrateSpin_) {
        // When audio-only mode is enabled, suggest quality preset bitrate
        QString preset = getAudioQualityPreset();
        if (preset == "low") {
            audioBitrateSpin_->setValue(32);
        } else if (preset == "medium") {
            audioBitrateSpin_->setValue(48);
        } else if (preset == "high") {
            audioBitrateSpin_->setValue(64);
        }
    }
}

void SettingsDialog::onAudioQualityPresetChanged(int index) {
    Q_UNUSED(index);

    // Update audio bitrate based on selected preset
    if (isAudioOnlyMode() && audioBitrateSpin_) {
        QString preset = getAudioQualityPreset();
        if (preset == "low") {
            audioBitrateSpin_->setValue(32);
        } else if (preset == "medium") {
            audioBitrateSpin_->setValue(48);
        } else if (preset == "high") {
            audioBitrateSpin_->setValue(64);
        }
    }
}
