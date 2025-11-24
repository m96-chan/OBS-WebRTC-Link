# Build Windows Installer for OBS WebRTC Link
# This script builds the NSIS installer for Windows

param(
    [string]$BuildDir = "..\..\build\Release",
    [string]$OutputDir = "..\..\build\installer",
    [string]$NSISPath = "C:\Program Files (x86)\NSIS\makensis.exe"
)

# Check if NSIS is installed
if (-not (Test-Path $NSISPath)) {
    Write-Error "NSIS not found at: $NSISPath"
    Write-Host "Please install NSIS from: https://nsis.sourceforge.io/Download"
    Write-Host "Or specify the path using -NSISPath parameter"
    exit 1
}

# Check if build directory exists
if (-not (Test-Path $BuildDir)) {
    Write-Error "Build directory not found: $BuildDir"
    Write-Host "Please build the plugin first using CMake"
    exit 1
}

# Check if plugin DLL exists
$pluginDll = Join-Path $BuildDir "obs-webrtc-link.dll"
if (-not (Test-Path $pluginDll)) {
    Write-Error "Plugin DLL not found: $pluginDll"
    Write-Host "Please build the plugin first"
    exit 1
}

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

# Get version from VERSION file
$version = (Get-Content "..\..\VERSION").Trim()
Write-Host "Building installer for version: $version"

# Build installer
Write-Host "Building NSIS installer..."
$nsisScript = "obs-webrtc-link.nsi"

& $NSISPath "/DPRODUCT_VERSION=$version" $nsisScript

if ($LASTEXITCODE -eq 0) {
    # Move installer to output directory
    $installerName = "obs-webrtc-link-$version-installer.exe"
    Move-Item "obs-webrtc-link-installer.exe" (Join-Path $OutputDir $installerName) -Force

    Write-Host ""
    Write-Host "Installer built successfully!" -ForegroundColor Green
    Write-Host "Output: $(Join-Path $OutputDir $installerName)"
    Write-Host ""
} else {
    Write-Error "Failed to build installer"
    exit 1
}
