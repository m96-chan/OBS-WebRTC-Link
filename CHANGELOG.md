# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.3] - 2025-11-25

### Added
- **WebRTC Link Source UI Enhancements**:
  - Connection mode selection (WHEP/P2P)
  - Stream ID field for WHEP mode (optional)
  - Bearer token authentication field (password-protected, optional)
  - Session ID field for P2P mode
  - Audio-only mode with quality presets (Low/Medium/High: 32/48/64 kbps)
  - Dynamic UI that shows/hides relevant fields based on connection mode and audio-only settings

### Changed
- WebRTC Link Source now supports both WHEP and P2P connection modes
- Video codec selection is hidden when audio-only mode is enabled

### Note
- Backend integration for new UI fields is pending (authentication, stream ID, P2P signaling, audio-only configuration)

## [0.1.2] - 2025-11-25

### Changed
- Temporarily disabled macOS build due to simde headers compilation issue
- This release includes Windows and Linux binaries only

### Note
- macOS support will be restored in a future release once the OBS 32.0.2 simde dependency issue is resolved

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

[Unreleased]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v0.1.3...HEAD
[0.1.3]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/m96-chan/OBS-WebRTC-Link/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/m96-chan/OBS-WebRTC-Link/releases/tag/v0.1.0
