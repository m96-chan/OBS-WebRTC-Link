#!/bin/bash
# Installation script for OBS WebRTC Link plugin (macOS)

OBS_PLUGINS_DIR="$HOME/Library/Application Support/obs-studio/plugins"

mkdir -p "$OBS_PLUGINS_DIR/obs-webrtc-link/bin"
mkdir -p "$OBS_PLUGINS_DIR/obs-webrtc-link/data"

cp obs-plugins/obs-webrtc-link.so "$OBS_PLUGINS_DIR/obs-webrtc-link/bin/"

echo "OBS WebRTC Link plugin installed successfully!"
echo "Please restart OBS Studio to load the plugin."
