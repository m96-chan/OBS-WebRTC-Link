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

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent),
      connectionModeCombo_(nullptr),
      serverUrlEdit_(nullptr),
      tokenEdit_(nullptr),
      videoCodecCombo_(nullptr),
      videoBitrateSpin_(nullptr),
      audioCodecCombo_(nullptr),
      audioBitrateSpin_(nullptr),
      okButton_(nullptr),
      cancelButton_(nullptr),
      errorLabel_(nullptr),
      mainLayout_(nullptr),
      formLayout_(nullptr) {
    setupUi();
    setWindowTitle(tr("WebRTC Link Settings"));
    setMinimumWidth(400);
}

SettingsDialog::~SettingsDialog() = default;

void SettingsDialog::setupUi() {
    mainLayout_ = new QVBoxLayout(this);

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
}

QFormLayout* SettingsDialog::createFormLayout() {
    QFormLayout* layout = new QFormLayout();

    // Connection mode
    connectionModeCombo_ = new QComboBox(this);
    connectionModeCombo_->setObjectName("connectionModeCombo");
    connectionModeCombo_->addItem(tr("SFU (Selective Forwarding Unit)"), "SFU");
    connectionModeCombo_->addItem(tr("P2P (Peer-to-Peer)"), "P2P");
    layout->addRow(tr("Connection Mode:"), connectionModeCombo_);

    // Server URL
    serverUrlEdit_ = new QLineEdit(this);
    serverUrlEdit_->setObjectName("serverUrlEdit");
    serverUrlEdit_->setPlaceholderText(tr("https://example.com/webrtc"));
    layout->addRow(tr("Server URL:"), serverUrlEdit_);

    // Token
    tokenEdit_ = new QLineEdit(this);
    tokenEdit_->setObjectName("tokenEdit");
    tokenEdit_->setPlaceholderText(tr("Authentication token (optional)"));
    tokenEdit_->setEchoMode(QLineEdit::Password);
    layout->addRow(tr("Token:"), tokenEdit_);

    // Video codec
    videoCodecCombo_ = new QComboBox(this);
    videoCodecCombo_->setObjectName("videoCodecCombo");
    videoCodecCombo_->addItem(tr("H.264"), "h264");
    videoCodecCombo_->addItem(tr("VP8"), "vp8");
    videoCodecCombo_->addItem(tr("VP9"), "vp9");
    videoCodecCombo_->addItem(tr("AV1"), "av1");
    layout->addRow(tr("Video Codec:"), videoCodecCombo_);

    // Video bitrate
    videoBitrateSpin_ = new QSpinBox(this);
    videoBitrateSpin_->setObjectName("videoBitrateSpin");
    videoBitrateSpin_->setMinimum(100);
    videoBitrateSpin_->setMaximum(50000);
    videoBitrateSpin_->setSingleStep(100);
    videoBitrateSpin_->setValue(2500);
    videoBitrateSpin_->setSuffix(tr(" kbps"));
    layout->addRow(tr("Video Bitrate:"), videoBitrateSpin_);

    // Audio codec
    audioCodecCombo_ = new QComboBox(this);
    audioCodecCombo_->setObjectName("audioCodecCombo");
    audioCodecCombo_->addItem(tr("Opus"), "opus");
    audioCodecCombo_->addItem(tr("AAC"), "aac");
    layout->addRow(tr("Audio Codec:"), audioCodecCombo_);

    // Audio bitrate
    audioBitrateSpin_ = new QSpinBox(this);
    audioBitrateSpin_->setObjectName("audioBitrateSpin");
    audioBitrateSpin_->setMinimum(32);
    audioBitrateSpin_->setMaximum(512);
    audioBitrateSpin_->setSingleStep(8);
    audioBitrateSpin_->setValue(128);
    audioBitrateSpin_->setSuffix(tr(" kbps"));
    layout->addRow(tr("Audio Bitrate:"), audioBitrateSpin_);

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

// Validation
bool SettingsDialog::validateSettings() const {
    QString url = getServerUrl();

    // Check if URL is empty
    if (url.isEmpty()) {
        return false;
    }

    // Check URL format
    if (!isValidUrl(url)) {
        return false;
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
        QString url = getServerUrl();
        if (url.isEmpty()) {
            showValidationError(tr("Error: Server URL cannot be empty."));
        } else if (!isValidUrl(url)) {
            showValidationError(tr("Error: Invalid server URL format. "
                                   "Please use http:// or https:// protocol."));
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
    // Future enhancement: Show/hide fields based on connection mode
    // For P2P mode, server URL might be optional
}
