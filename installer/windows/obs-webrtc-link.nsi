; OBS WebRTC Link Plugin Installer
; NSIS Script for Windows
;
; This script creates an installer for the OBS WebRTC Link plugin.
; It automatically detects OBS Studio installation and installs the plugin.

;--------------------------------
; Includes

!include "MUI2.nsh"
!include "x64.nsh"
!include "FileFunc.nsh"

;--------------------------------
; General

; Read version from VERSION file at build time
!define PRODUCT_NAME "OBS WebRTC Link"
!define PRODUCT_PUBLISHER "OBS-WebRTC-Link Contributors"
!define PRODUCT_WEB_SITE "https://github.com/m96-chan/OBS-WebRTC-Link"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Plugin files
!define PLUGIN_DLL "obs-webrtc-link.dll"
!define PLUGIN_DATA_DIR "obs-webrtc-link-data"

; Name and file
Name "${PRODUCT_NAME}"
OutFile "obs-webrtc-link-installer.exe"

; Default installation folder
InstallDir ""

; Request application privileges
RequestExecutionLevel admin

; Version information
VIProductVersion "0.1.0.0"
VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "FileDescription" "${PRODUCT_NAME} Installer"
VIAddVersionKey "FileVersion" "0.1.0"

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp"

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Japanese"

;--------------------------------
; Functions

; Detect OBS Studio installation path
Function .onInit
  ; Check if we're running on 64-bit Windows
  ${If} ${RunningX64}
    SetRegView 64
  ${Else}
    MessageBox MB_OK|MB_ICONEXCLAMATION "This plugin requires 64-bit Windows."
    Abort
  ${EndIf}

  ; Try to detect OBS Studio installation
  ReadRegStr $0 HKLM "SOFTWARE\OBS Studio" ""
  ${If} $0 != ""
    StrCpy $INSTDIR "$0\obs-plugins\64bit"
  ${Else}
    ; Try alternative registry location
    ReadRegStr $0 HKCU "SOFTWARE\OBS Studio" ""
    ${If} $0 != ""
      StrCpy $INSTDIR "$0\obs-plugins\64bit"
    ${Else}
      ; Default to Program Files
      StrCpy $INSTDIR "$PROGRAMFILES64\obs-studio\obs-plugins\64bit"
    ${EndIf}
  ${EndIf}

  ; Verify OBS installation
  ${If} ${FileExists} "$INSTDIR\obs.dll"
    ; OBS found
  ${Else}
    MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "OBS Studio installation not detected. Please select your OBS Studio installation directory manually." IDOK cont
    Abort
    cont:
  ${EndIf}
FunctionEnd

;--------------------------------
; Installer Sections

Section "Main Plugin" SecMain
  SectionIn RO  ; Read-only, always installed

  ; Set output path to the plugin directory
  SetOutPath "$INSTDIR"

  ; Install plugin DLL
  File /oname=${PLUGIN_DLL} "..\..\build\Release\${PLUGIN_DLL}"

  ; Install data directory if exists
  ${If} ${FileExists} "..\..\build\Release\${PLUGIN_DATA_DIR}\*.*"
    SetOutPath "$INSTDIR\${PLUGIN_DATA_DIR}"
    File /r "..\..\build\Release\${PLUGIN_DATA_DIR}\*.*"
  ${EndIf}

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\${PRODUCT_NAME}-uninstall.exe"

  ; Write registry keys for uninstaller
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\${PRODUCT_NAME}-uninstall.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\${PLUGIN_DLL}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1
SectionEnd

Section "Documentation" SecDocs
  SetOutPath "$INSTDIR\${PLUGIN_DATA_DIR}"

  ; Install documentation
  File /oname=README.txt "..\..\README.md"
  File /oname=CHANGELOG.txt "..\..\CHANGELOG.md"
  File /oname=LICENSE.txt "..\..\LICENSE"
SectionEnd

;--------------------------------
; Section Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "Core plugin files (required)"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDocs} "Documentation files (README, CHANGELOG, LICENSE)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"
  ; Remove plugin files
  Delete "$INSTDIR\${PLUGIN_DLL}"
  Delete "$INSTDIR\${PRODUCT_NAME}-uninstall.exe"

  ; Remove data directory
  RMDir /r "$INSTDIR\${PLUGIN_DATA_DIR}"

  ; Remove registry keys
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"

  ; Display completion message
  MessageBox MB_OK "$(^Name) has been uninstalled from your computer."
SectionEnd

;--------------------------------
; Language Strings

LangString DESC_SecMain ${LANG_ENGLISH} "Install the OBS WebRTC Link plugin."
LangString DESC_SecMain ${LANG_JAPANESE} "OBS WebRTC Link プラグインをインストールします。"

LangString DESC_SecDocs ${LANG_ENGLISH} "Install documentation files."
LangString DESC_SecDocs ${LANG_JAPANESE} "ドキュメントファイルをインストールします。"
