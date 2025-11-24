# Integration test runner script for Windows
#
# This script:
# 1. Checks for Docker availability
# 2. Starts LiveKit container via Docker Compose
# 3. Runs integration tests
# 4. Stops LiveKit container
# 5. Reports results

param(
    [string]$Configuration = "Debug",
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ScriptDir "..\..\..\build"
$DockerDir = Join-Path $ScriptDir "..\docker"

Write-Host "===================="
Write-Host "Integration Test Runner"
Write-Host "===================="
Write-Host

# Check Docker
Write-Host "Checking Docker availability..."
try {
    $dockerVersion = docker --version 2>$null
    if ($LASTEXITCODE -ne 0) {
        throw "Docker command failed"
    }
    Write-Host "Docker is installed: $dockerVersion"
} catch {
    Write-Host "ERROR: Docker is not installed or not in PATH" -ForegroundColor Red
    Write-Host "Integration tests require Docker to run LiveKit server"
    exit 1
}

try {
    docker info 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "Docker is not running"
    }
    Write-Host "Docker is running"
} catch {
    Write-Host "ERROR: Docker is not running" -ForegroundColor Red
    Write-Host "Please start Docker Desktop and try again"
    exit 1
}

Write-Host

# Check if build directory exists
if (!(Test-Path $BuildDir)) {
    Write-Host "ERROR: Build directory not found: $BuildDir" -ForegroundColor Red
    Write-Host "Please build the project first"
    exit 1
}

# Navigate to build directory
Set-Location $BuildDir

Write-Host "Running integration tests (Configuration: $Configuration)..."
Write-Host

# Run integration tests with CTest
$ctestArgs = @(
    "-C", $Configuration,
    "-R", "Integration"
)

if ($Verbose) {
    $ctestArgs += "-V"
} else {
    $ctestArgs += "--output-on-failure"
}

& ctest @ctestArgs

$TestResult = $LASTEXITCODE

Write-Host
Write-Host "===================="
if ($TestResult -eq 0) {
    Write-Host "All integration tests PASSED" -ForegroundColor Green
} else {
    Write-Host "Some integration tests FAILED" -ForegroundColor Red
}
Write-Host "===================="

exit $TestResult
