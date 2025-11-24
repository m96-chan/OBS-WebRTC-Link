# Windows Installer

This directory contains the NSIS script and build tools for creating a Windows installer for the OBS WebRTC Link plugin.

## Prerequisites

### For Building the Installer

1. **NSIS (Nullsoft Scriptable Install System)**
   - Download and install from: https://nsis.sourceforge.io/Download
   - Default installation path: `C:\Program Files (x86)\NSIS`

2. **Built Plugin**
   - The plugin must be built first using CMake
   - Plugin DLL should be in: `build/Release/obs-webrtc-link.dll`

## Building the Installer

### Using PowerShell Script (Recommended)

```powershell
cd installer/windows
.\build-installer.ps1
```

The script will:
1. Check if NSIS is installed
2. Verify the plugin DLL exists
3. Build the installer using NSIS
4. Output the installer to `build/installer/obs-webrtc-link-{version}-installer.exe`

### Custom NSIS Path

If NSIS is installed in a non-standard location:

```powershell
.\build-installer.ps1 -NSISPath "C:\Path\To\makensis.exe"
```

### Manual Build

```powershell
cd installer/windows
"C:\Program Files (x86)\NSIS\makensis.exe" obs-webrtc-link.nsi
```

## Installer Features

### Installation

- **OBS Studio Detection**: Automatically detects OBS Studio installation
- **64-bit Only**: Requires 64-bit Windows and OBS Studio
- **Plugin Installation**: Installs plugin DLL to OBS plugins directory
- **Documentation**: Optionally installs README, CHANGELOG, and LICENSE
- **Registry Integration**: Creates uninstaller entry in Windows Programs & Features

### Directory Structure

The installer places files in the following locations:

```
<OBS Studio>\obs-plugins\64bit\
├── obs-webrtc-link.dll           # Plugin DLL
├── obs-webrtc-link-uninstall.exe # Uninstaller
└── obs-webrtc-link-data\         # Data directory (optional)
    ├── README.txt
    ├── CHANGELOG.txt
    └── LICENSE.txt
```

### Uninstallation

The installer creates an uninstaller that can be accessed through:
- Windows Settings > Apps & Features
- Control Panel > Programs and Features
- Or directly running `obs-webrtc-link-uninstall.exe`

## Customization

### Changing Installation Location

Users can manually select the installation directory during installation if OBS Studio is not detected automatically.

### Adding Files

To add additional files to the installer, edit `obs-webrtc-link.nsi`:

```nsis
Section "Additional Files" SecAdditional
  SetOutPath "$INSTDIR\obs-webrtc-link-data"
  File "path\to\your\file.txt"
SectionEnd
```

### Language Support

The installer supports:
- English (default)
- Japanese (日本語)

To add more languages, add language strings in the NSIS script:

```nsis
!insertmacro MUI_LANGUAGE "German"
LangString DESC_SecMain ${LANG_GERMAN} "German description"
```

## Code Signing (For Official Releases)

For official releases, the installer should be code signed:

1. Obtain a code signing certificate
2. Use `signtool` to sign the installer:

```powershell
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com obs-webrtc-link-installer.exe
```

Or use SignPath.io for automated signing in CI.

## CI Integration

The installer build can be integrated into GitHub Actions workflow:

```yaml
- name: Build Installer
  run: |
    choco install nsis -y
    cd installer/windows
    .\build-installer.ps1
```

Note: Currently, the GitHub Actions CI does not build Windows binaries with OBS SDK, so installer generation is not included in the automated workflow.

## Troubleshooting

### NSIS Not Found

- Verify NSIS is installed
- Check the installation path
- Use `-NSISPath` parameter to specify custom location

### Plugin DLL Not Found

- Build the plugin first using CMake
- Verify the build output directory
- Check that `obs-webrtc-link.dll` exists in `build/Release`

### Installation Fails

- Run installer as Administrator
- Verify OBS Studio is installed
- Check that OBS Studio version is 64-bit
- Ensure sufficient disk space

## References

- [NSIS Documentation](https://nsis.sourceforge.io/Docs/)
- [Modern UI Reference](https://nsis.sourceforge.io/Docs/Modern%20UI%202/Readme.html)
- [NSIS Scripting Reference](https://nsis.sourceforge.io/Docs/Chapter4.html)
