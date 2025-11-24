@echo off
REM Installation script for OBS WebRTC Link plugin (Windows)

echo Installing OBS WebRTC Link Plugin...

set OBS_DIR=C:\Program Files\obs-studio
if not exist "%OBS_DIR%" (
    echo Error: OBS Studio not found at %OBS_DIR%
    echo Please install OBS Studio first or manually copy files
    pause
    exit /b 1
)

xcopy /Y /I obs-plugins\64bit\*.dll "%OBS_DIR%\obs-plugins\64bit\"
if errorlevel 1 (
    echo Error: Failed to copy plugin files
    echo Please run as Administrator
    pause
    exit /b 1
)

echo.
echo Installation complete!
echo Please restart OBS Studio to load the plugin.
pause
