# TDD Watch Script for Windows
# Automatically rebuilds and runs tests when source files change

param(
    [string]$TestName = "*",
    [int]$Interval = 2,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

$CMAKE_PATH = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$BUILD_DIR = "build"
$BUILD_CONFIG = "Release"

function Show-Help {
    Write-Host @"
TDD Watch Mode - Automatically rebuild and test on file changes

Usage: .\scripts\tdd-watch.ps1 [options]

Options:
    -TestName <pattern>   Filter tests by name (default: "*" for all tests)
    -Interval <seconds>   Check interval in seconds (default: 2)
    -Help                 Show this help message

Examples:
    # Watch all tests
    .\scripts\tdd-watch.ps1

    # Watch specific test
    .\scripts\tdd-watch.ps1 -TestName "settings_dialog"

    # Watch with custom interval
    .\scripts\tdd-watch.ps1 -Interval 5

Controls:
    Ctrl+C    Stop watching
"@
}

function Get-SourceFiles {
    $patterns = @("*.cpp", "*.hpp", "*.h", "*.c")
    $directories = @("src", "tests")

    $files = @()
    foreach ($dir in $directories) {
        foreach ($pattern in $patterns) {
            $files += Get-ChildItem -Path $dir -Filter $pattern -Recurse -File -ErrorAction SilentlyContinue
        }
    }
    return $files
}

function Get-FileHashes {
    param([array]$Files)

    $hashes = @{}
    foreach ($file in $Files) {
        $hashes[$file.FullName] = (Get-FileHash -Path $file.FullName -Algorithm MD5).Hash
    }
    return $hashes
}

function Test-FilesChanged {
    param(
        [hashtable]$OldHashes,
        [array]$Files
    )

    foreach ($file in $Files) {
        $newHash = (Get-FileHash -Path $file.FullName -Algorithm MD5).Hash
        $oldHash = $OldHashes[$file.FullName]

        if ($oldHash -ne $newHash) {
            return $true
        }
    }
    return $false
}

function Invoke-BuildAndTest {
    param([string]$TestFilter)

    Write-Host ""
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "Building..." -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan

    # Rebuild
    & $CMAKE_PATH --build $BUILD_DIR --config $BUILD_CONFIG 2>&1 | Out-Host

    if ($LASTEXITCODE -ne 0) {
        Write-Host "✗ Build failed" -ForegroundColor Red
        return $false
    }

    Write-Host "✓ Build successful" -ForegroundColor Green

    # Run tests
    Write-Host ""
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "Running tests (filter: $TestFilter)..." -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan

    $testDir = "$BUILD_DIR\tests\unit\$BUILD_CONFIG"
    if (-not (Test-Path $testDir)) {
        Write-Host "✗ Test directory not found" -ForegroundColor Red
        return $false
    }

    $testExecutables = Get-ChildItem -Path $testDir -Filter "*$TestFilter*_test.exe"

    if ($testExecutables.Count -eq 0) {
        Write-Host "✗ No tests found matching: $TestFilter" -ForegroundColor Yellow
        return $false
    }

    $allPassed = $true
    foreach ($test in $testExecutables) {
        Write-Host ""
        Write-Host "Running: $($test.Name)" -ForegroundColor Yellow
        & $test.FullName --gtest_color=yes 2>&1 | Out-Host

        if ($LASTEXITCODE -ne 0) {
            $allPassed = $false
        }
    }

    return $allPassed
}

# Main execution
if ($Help) {
    Show-Help
    exit 0
}

Write-Host ""
Write-Host "===========================================================" -ForegroundColor Cyan
Write-Host "  TDD Watch Mode" -ForegroundColor Cyan
Write-Host "===========================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Watching for changes in src/ and tests/" -ForegroundColor Yellow
Write-Host "Test filter: $TestName" -ForegroundColor Yellow
Write-Host "Check interval: $Interval seconds" -ForegroundColor Yellow
Write-Host "Press Ctrl+C to stop" -ForegroundColor Yellow
Write-Host ""

# Check prerequisites
if (-not (Test-Path $CMAKE_PATH)) {
    Write-Host "ERROR: CMake not found" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $BUILD_DIR)) {
    Write-Host "ERROR: Build directory not found. Run setup-windows-dev.ps1 first" -ForegroundColor Red
    exit 1
}

# Initial build and test
Write-Host "Running initial build and test..." -ForegroundColor Cyan
$lastResult = Invoke-BuildAndTest -TestFilter $TestName

if ($lastResult) {
    Write-Host ""
    Write-Host "✓ All tests passed" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "✗ Some tests failed" -ForegroundColor Red
}

# Get initial file hashes
$sourceFiles = Get-SourceFiles
$fileHashes = Get-FileHashes -Files $sourceFiles

Write-Host ""
Write-Host "Watching for file changes..." -ForegroundColor Yellow

# Watch loop
try {
    while ($true) {
        Start-Sleep -Seconds $Interval

        # Refresh file list
        $sourceFiles = Get-SourceFiles

        # Check for changes
        if (Test-FilesChanged -OldHashes $fileHashes -Files $sourceFiles) {
            Write-Host ""
            Write-Host "========================================" -ForegroundColor Magenta
            Write-Host "  File change detected!" -ForegroundColor Magenta
            Write-Host "========================================" -ForegroundColor Magenta

            # Rebuild and test
            $lastResult = Invoke-BuildAndTest -TestFilter $TestName

            if ($lastResult) {
                Write-Host ""
                Write-Host "✓ All tests passed" -ForegroundColor Green
                # Optional: Play success sound
                [console]::beep(800, 200)
            } else {
                Write-Host ""
                Write-Host "✗ Some tests failed" -ForegroundColor Red
                # Optional: Play failure sound
                [console]::beep(400, 300)
            }

            # Update hashes
            $fileHashes = Get-FileHashes -Files $sourceFiles

            Write-Host ""
            Write-Host "Watching for file changes..." -ForegroundColor Yellow
        }
    }
} catch {
    Write-Host ""
    Write-Host "Watch mode stopped" -ForegroundColor Yellow
}
