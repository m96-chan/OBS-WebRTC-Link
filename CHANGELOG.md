# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.1] - 2025-11-25

### Added
- **Windows Installer**: Inno Setup-based installer with auto-detection of OBS installation
- **Windows Portable Build**: Manual installation ZIP with instructions
- **macOS Unsigned Binary**: Plugin bundle with manual installation instructions
- Full Windows plugin binary build in CI using OBS SDK 30.2.2
- Full macOS plugin binary build in CI using Homebrew OBS

### Changed
- Updated README installation instructions to reflect current release artifacts
- Windows builds now include pre-compiled DLL (no longer source-only)
- macOS builds now include pre-compiled plugin bundle (unsigned)

### Fixed
- Windows release pipeline now builds actual plugin binary instead of tests-only
- macOS release pipeline now builds actual plugin binary instead of tests-only

### Known Issues
- macOS plugin is unsigned and requires manual Gatekeeper override
- Windows may require "Run as Administrator" for installer depending on OBS install location

## [0.1.0] - 2025-11-25

### Added
- Initial plugin structure
- WebRTC Output support (WHIP)
- WebRTC Source support (WHEP)
- P2P connection support
- LiveKit SFU integration
- Docker development environment
- Comprehensive unit tests
- Performance benchmarks
- Detailed build instructions
- LiveKit setup guide

### Core Features
- **WebRTC Output**: Stream from OBS via WHIP protocol
- **WebRTC Source**: Receive WebRTC streams via WHEP protocol
- **P2P Mode**: Direct peer-to-peer connections without server
- **SFU Support**: LiveKit integration for scalable streaming
- **Reconnection**: Automatic reconnection with exponential backoff
- **Cross-Platform**: Windows, macOS, and Linux support

### Testing
- 9 unit test suites
- Performance benchmarks for connection establishment
- CI/CD integration with GitHub Actions

### Documentation
- README with usage instructions
- Build instructions for all platforms
- LiveKit setup guide
- Docker development environment setup

[Unreleased]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v0.1.1...HEAD
[0.1.1]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/m96-chan/OBS-WebRTC-Link/releases/tag/v0.1.0
