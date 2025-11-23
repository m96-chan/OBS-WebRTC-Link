# Windows Development Environment Setup Script
# Sets up CMake, builds test-only configuration, and runs unit tests

param(
    [switch]$Clean,
    [switch]$RunTests,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Script configuration
$CMAKE_PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$BUILD_DIR = "build"
$BUILD_CONFIG = "Release"

function Show-Help {
    Write-Host @"
Windows Development Environment Setup for OBS-WebRTC-Link

Usage: .\scripts\setup-windows-dev.ps1 [options]

Options:
    -Clean      Clean build directory before building
    -RunTests   Run all unit tests after building
    -Help       Show this help message

Examples:
    # Initial setup and build
    .\scripts\setup-windows-dev.ps1

    # Clean build
    .\scripts\setup-windows-dev.ps1 -Clean

    # Build and run tests
    .\scripts\setup-windows-dev.ps1 -RunTests

    # Clean build and run tests
    .\scripts\setup-windows-dev.ps1 -Clean -RunTests

Requirements:
    - Visual Studio 2022 with C++ development tools
    - Git with submodules initialized
"@
}

function Test-Prerequisites {
    Write-Host "Checking prerequisites..." -ForegroundColor Cyan

    # Check CMake
    if (-not (Test-Path $CMAKE_PATH)) {
        Write-Host "ERROR: CMake not found at: $CMAKE_PATH" -ForegroundColor Red
        Write-Host "Please install Visual Studio 2022 with CMake support" -ForegroundColor Yellow
        exit 1
    }
    Write-Host "✓ CMake found: $CMAKE_PATH" -ForegroundColor Green

    # Check Git submodules
    $submodules = @("deps/libdatachannel", "deps/nlohmann-json", "deps/googletest")
    foreach ($submodule in $submodules) {
        if (-not (Test-Path $submodule)) {
            Write-Host "ERROR: Submodule not found: $submodule" -ForegroundColor Red
            Write-Host "Please initialize submodules: git submodule update --init --recursive" -ForegroundColor Yellow
            exit 1
        }
    }
    Write-Host "✓ All submodules initialized" -ForegroundColor Green
}

function Invoke-CleanBuild {
    if (Test-Path $BUILD_DIR) {
        Write-Host "Cleaning build directory..." -ForegroundColor Cyan
        Remove-Item -Path $BUILD_DIR -Recurse -Force
        Write-Host "✓ Build directory cleaned" -ForegroundColor Green
    }
}

function Invoke-CMakeConfigure {
    Write-Host ""
    Write-Host "Configuring CMake (Tests Only)..." -ForegroundColor Cyan
    Write-Host "This builds unit tests without requiring OBS Studio SDK" -ForegroundColor Yellow
    Write-Host ""

    & $CMAKE_PATH -B $BUILD_DIR `
        -DCMAKE_BUILD_TYPE=$BUILD_CONFIG `
        -DBUILD_LIBDATACHANNEL=ON `
        -DBUILD_TESTING=ON `
        -DBUILD_TESTS_ONLY=ON

    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: CMake configuration failed" -ForegroundColor Red
        exit 1
    }
    Write-Host "✓ CMake configuration successful" -ForegroundColor Green
}

function Invoke-Build {
    Write-Host ""
    Write-Host "Building dependencies and tests..." -ForegroundColor Cyan
    Write-Host ""

    # Build datachannel-static
    Write-Host "Building libdatachannel..." -ForegroundColor Yellow
    & $CMAKE_PATH --build $BUILD_DIR --config $BUILD_CONFIG --target datachannel-static
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: datachannel-static build failed" -ForegroundColor Red
        exit 1
    }

    # Build test framework
    Write-Host ""
    Write-Host "Building Google Test framework..." -ForegroundColor Yellow
    & $CMAKE_PATH --build $BUILD_DIR --config $BUILD_CONFIG --target gtest
    & $CMAKE_PATH --build $BUILD_DIR --config $BUILD_CONFIG --target gtest_main
    & $CMAKE_PATH --build $BUILD_DIR --config $BUILD_CONFIG --target gmock

    # Build all test targets
    Write-Host ""
    Write-Host "Building all test executables..." -ForegroundColor Yellow
    & $CMAKE_PATH --build $BUILD_DIR --config $BUILD_CONFIG

    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERROR: Build failed" -ForegroundColor Red
        exit 1
    }
    Write-Host ""
    Write-Host "✓ Build successful" -ForegroundColor Green
}

function Invoke-Tests {
    Write-Host ""
    Write-Host "Running unit tests..." -ForegroundColor Cyan
    Write-Host ""

    # Find test executables
    $testDir = "$BUILD_DIR\tests\unit\$BUILD_CONFIG"
    if (-not (Test-Path $testDir)) {
        Write-Host "ERROR: Test directory not found: $testDir" -ForegroundColor Red
        exit 1
    }

    $testExecutables = Get-ChildItem -Path $testDir -Filter "*_test.exe"
    if ($testExecutables.Count -eq 0) {
        Write-Host "ERROR: No test executables found in: $testDir" -ForegroundColor Red
        exit 1
    }

    Write-Host "Found $($testExecutables.Count) test executable(s)" -ForegroundColor Yellow
    Write-Host ""

    $failedTests = @()
    $passedTests = @()

    foreach ($test in $testExecutables) {
        Write-Host "======================================" -ForegroundColor Cyan
        Write-Host "Running: $($test.Name)" -ForegroundColor Cyan
        Write-Host "======================================" -ForegroundColor Cyan

        & $test.FullName --gtest_color=yes

        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ $($test.Name) PASSED" -ForegroundColor Green
            $passedTests += $test.Name
        } else {
            Write-Host "✗ $($test.Name) FAILED" -ForegroundColor Red
            $failedTests += $test.Name
        }
        Write-Host ""
    }

    # Summary
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "Test Summary" -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "Total: $($testExecutables.Count)" -ForegroundColor White
    Write-Host "Passed: $($passedTests.Count)" -ForegroundColor Green
    Write-Host "Failed: $($failedTests.Count)" -ForegroundColor $(if ($failedTests.Count -gt 0) { "Red" } else { "Green" })
    Write-Host ""

    if ($failedTests.Count -gt 0) {
        Write-Host "Failed tests:" -ForegroundColor Red
        foreach ($test in $failedTests) {
            Write-Host "  - $test" -ForegroundColor Red
        }
        exit 1
    } else {
        Write-Host "✓ All tests passed!" -ForegroundColor Green
    }
}

# Main script execution
try {
    if ($Help) {
        Show-Help
        exit 0
    }

    Write-Host ""
    Write-Host "===========================================================" -ForegroundColor Cyan
    Write-Host "  OBS-WebRTC-Link Windows Development Setup" -ForegroundColor Cyan
    Write-Host "===========================================================" -ForegroundColor Cyan
    Write-Host ""

    Test-Prerequisites

    if ($Clean) {
        Invoke-CleanBuild
    }

    Invoke-CMakeConfigure
    Invoke-Build

    if ($RunTests) {
        Invoke-Tests
    } else {
        Write-Host ""
        Write-Host "Build complete! To run tests, use:" -ForegroundColor Yellow
        Write-Host "  .\scripts\setup-windows-dev.ps1 -RunTests" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "Or run tests manually:" -ForegroundColor Yellow
        Write-Host "  cd $BUILD_DIR\tests\unit\$BUILD_CONFIG" -ForegroundColor Cyan
        Write-Host "  .\sample_test.exe" -ForegroundColor Cyan
    }

    Write-Host ""
    Write-Host "===========================================================" -ForegroundColor Cyan
    Write-Host "  Setup Complete!" -ForegroundColor Green
    Write-Host "===========================================================" -ForegroundColor Cyan
    Write-Host ""

} catch {
    Write-Host ""
    Write-Host "ERROR: An unexpected error occurred" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    exit 1
}
