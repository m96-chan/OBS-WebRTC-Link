#!/bin/bash
# Installation script for OBS WebRTC Link plugin (Linux)

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root (use sudo)"
  exit 1
fi

cp lib/obs-plugins/obs-webrtc-link.so /usr/lib/obs-plugins/ || \
cp lib/obs-plugins/obs-webrtc-link.so /usr/lib/x86_64-linux-gnu/obs-plugins/

echo "OBS WebRTC Link plugin installed successfully!"
echo "Please restart OBS Studio to load the plugin."
