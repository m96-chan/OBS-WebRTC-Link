@echo off
REM Quick test runner for Windows
REM Usage: scripts\test.bat [clean]

setlocal enabledelayedexpansion

if "%1"=="clean" (
    echo Running clean build and tests...
    powershell -ExecutionPolicy Bypass -File "%~dp0setup-windows-dev.ps1" -Clean -RunTests
) else if "%1"=="help" (
    powershell -ExecutionPolicy Bypass -File "%~dp0setup-windows-dev.ps1" -Help
) else (
    echo Running tests...
    powershell -ExecutionPolicy Bypass -File "%~dp0setup-windows-dev.ps1" -RunTests
)

exit /b %ERRORLEVEL%
