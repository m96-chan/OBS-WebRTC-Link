# Installation Guide / インストールガイド

**English | [日本語](#日本語)

---

## English

This guide provides detailed instructions for installing the OBS-WebRTC-Link plugin on Windows, macOS, and Linux.

### System Requirements

- **OBS Studio**: Version 30.0 or later
- **Operating System**:
  - Windows 10/11 (64-bit)
  - macOS 11 (Big Sur) or later
  - Linux (Ubuntu 20.04 LTS or later, or equivalent distribution)
- **Hardware**:
  - CPU with hardware encoding support recommended (NVENC/AMF/QuickSync)
  - Minimum 4GB RAM
  - Stable internet connection for WebRTC streaming

---

## Windows Installation

### Method 1: Using the Installer (Recommended)

1. **Download the Installer**
   - Go to the [Releases page](https://github.com/m96-chan/OBS-WebRTC-Link/releases)
   - Download the latest `.exe` installer for Windows
   - File name format: `obs-webrtc-link-<version>-windows-installer.exe`

2. **Run the Installer**
   - Double-click the downloaded installer
   - If Windows SmartScreen appears, click "More info" → "Run anyway"
   - Click "Next" to proceed through the installation wizard
   - The installer will automatically detect your OBS Studio installation path
   - If OBS Studio is not found automatically, click "Browse" and navigate to your OBS installation folder (typically `C:\Program Files\obs-studio`)
   - Click "Install" to begin installation

3. **Complete Installation**
   - Wait for the installation to complete
   - Click "Finish"
   - **Important**: Restart OBS Studio if it's currently running

4. **Verify Installation**
   - Launch OBS Studio
   - Click on "Tools" menu → Look for "WebRTC Link Settings" or similar option
   - In the Sources panel, click the "+" button → You should see "WebRTC Link Source" in the list

### Method 2: Manual Installation from ZIP

1. **Download the ZIP Package**
   - Go to the [Releases page](https://github.com/m96-chan/OBS-WebRTC-Link/releases)
   - Download the latest `.zip` file for Windows
   - File name format: `obs-webrtc-link-<version>-windows.zip`

2. **Extract Files**
   - Right-click the downloaded ZIP file and select "Extract All..."
   - Choose a temporary extraction location

3. **Locate OBS Plugin Directory**
   - Open File Explorer and navigate to one of these locations:
     - **Program Files** (system-wide): `C:\Program Files\obs-studio\obs-plugins\64bit\`
     - **AppData** (user-specific): `%APPDATA%\obs-studio\obs-plugins\64bit\`

4. **Copy Plugin Files**
   - Copy all files from the extracted folder into the OBS plugin directory:
     - `obs-webrtc-link.dll`
     - `obs-webrtc-link.pdb` (optional, for debugging)
   - Copy the `data` folder to the parent directory (`obs-plugins/`)

5. **Restart OBS Studio**
   - Close OBS Studio completely (check system tray)
   - Relaunch OBS Studio
   - Verify the plugin appears in Tools menu and Sources list

### Windows Troubleshooting

**Plugin doesn't appear in OBS:**
- Ensure OBS Studio is completely closed before copying files
- Check that you copied files to the correct `64bit` subdirectory
- Verify OBS Studio version is 30.0 or later
- Check Windows Event Viewer for DLL loading errors

**Missing DLL errors:**
- Install Visual C++ Redistributable 2019 or later from [Microsoft](https://aka.ms/vs/17/release/vc_redist.x64.exe)
- Ensure all plugin files were copied correctly

**Permission errors during installation:**
- Right-click installer and select "Run as Administrator"
- Or install to user directory (`%APPDATA%`) instead

---

## macOS Installation

### Method 1: Using the PKG Installer (Recommended)

1. **Download the Installer**
   - Go to the [Releases page](https://github.com/m96-chan/OBS-WebRTC-Link/releases)
   - Download the latest `.pkg` installer for macOS
   - File name format: `obs-webrtc-link-<version>-macos.pkg`

2. **Run the Installer**
   - Double-click the downloaded PKG file
   - If macOS shows "unidentified developer" warning:
     - Click "OK"
     - Open "System Settings" → "Privacy & Security"
     - Scroll down and click "Open Anyway" next to the blocked app message
   - Follow the installation wizard
   - Enter your administrator password when prompted

3. **Complete Installation**
   - The plugin will be installed to `/Library/Application Support/obs-studio/plugins/`
   - Click "Close" when installation finishes
   - Restart OBS Studio if it's running

4. **Verify Installation**
   - Launch OBS Studio
   - Check "Tools" menu for WebRTC Link options
   - Verify "WebRTC Link Source" appears in Sources list

### Method 2: Manual Installation from ZIP

1. **Download the ZIP Package**
   - Go to the [Releases page](https://github.com/m96-chan/OBS-WebRTC-Link/releases)
   - Download the latest `.zip` file for macOS
   - File name format: `obs-webrtc-link-<version>-macos.zip`

2. **Extract Files**
   - Double-click the ZIP file to extract (or use `unzip` in Terminal)

3. **Locate OBS Plugin Directory**
   - System-wide: `/Library/Application Support/obs-studio/plugins/`
   - User-specific: `~/Library/Application Support/obs-studio/plugins/`
   - Create the directory if it doesn't exist:
     ```bash
     mkdir -p ~/Library/Application\ Support/obs-studio/plugins/
     ```

4. **Copy Plugin Files**
   - Open Terminal and run:
     ```bash
     cp -R obs-webrtc-link.plugin ~/Library/Application\ Support/obs-studio/plugins/
     ```
   - Or drag the `obs-webrtc-link.plugin` bundle to the plugins folder

5. **Remove Quarantine Attribute (Important)**
   - macOS may quarantine downloaded files. Remove the quarantine:
     ```bash
     xattr -cr ~/Library/Application\ Support/obs-studio/plugins/obs-webrtc-link.plugin
     ```

6. **Restart OBS Studio**
   - Quit OBS Studio completely (Cmd+Q)
   - Relaunch OBS Studio

### macOS Troubleshooting

**"Cannot be opened because the developer cannot be verified":**
- Run the quarantine removal command above
- Or: Right-click the plugin → "Open" → Confirm

**Plugin not loading:**
- Check Console.app for error messages from OBS
- Verify macOS version is 11.0 or later
- Ensure you copied the entire `.plugin` bundle, not just individual files

**Permission issues:**
- Try installing to user directory instead of system directory
- Or use `sudo` for system-wide installation (not recommended)

---

## Linux Installation

### Method 1: Using Distribution Package (Recommended)

#### Ubuntu/Debian (DEB package)

1. **Download the DEB Package**
   - Go to the [Releases page](https://github.com/m96-chan/OBS-WebRTC-Link/releases)
   - Download the latest `.deb` file
   - File name format: `obs-webrtc-link_<version>_amd64.deb`

2. **Install via Terminal**
   ```bash
   sudo apt install ./obs-webrtc-link_<version>_amd64.deb
   ```
   - Or double-click the DEB file to open Software Center and click "Install"

3. **Restart OBS Studio**
   ```bash
   killall obs
   obs &
   ```

#### Fedora/RHEL (RPM package)

1. **Download the RPM Package**
   - Go to the [Releases page](https://github.com/m96-chan/OBS-WebRTC-Link/releases)
   - Download the latest `.rpm` file
   - File name format: `obs-webrtc-link-<version>.x86_64.rpm`

2. **Install via Terminal**
   ```bash
   sudo dnf install ./obs-webrtc-link-<version>.x86_64.rpm
   ```
   - Or use `yum` on older systems

3. **Restart OBS Studio**

#### Arch Linux (AUR)

1. **Install from AUR**
   ```bash
   yay -S obs-webrtc-link
   # or using your preferred AUR helper
   ```

2. **Restart OBS Studio**

### Method 2: Manual Installation from TAR.GZ

1. **Download the Archive**
   - Go to the [Releases page](https://github.com/m96-chan/OBS-WebRTC-Link/releases)
   - Download the latest `.tar.gz` file for Linux
   - File name format: `obs-webrtc-link-<version>-linux-x86_64.tar.gz`

2. **Extract the Archive**
   ```bash
   tar -xzf obs-webrtc-link-<version>-linux-x86_64.tar.gz
   cd obs-webrtc-link-<version>
   ```

3. **Locate OBS Plugin Directory**
   - System-wide: `/usr/lib/obs-plugins/` or `/usr/lib/x86_64-linux-gnu/obs-plugins/`
   - User-specific: `~/.config/obs-studio/plugins/`
   - Create user directory if it doesn't exist:
     ```bash
     mkdir -p ~/.config/obs-studio/plugins/
     ```

4. **Copy Plugin Files**
   - For user installation:
     ```bash
     cp -R lib/obs-plugins/* ~/.config/obs-studio/plugins/
     cp -R share/obs/obs-plugins/* ~/.local/share/obs/obs-plugins/
     ```
   - For system-wide installation (requires sudo):
     ```bash
     sudo cp -R lib/obs-plugins/* /usr/lib/obs-plugins/
     sudo cp -R share/obs/obs-plugins/* /usr/share/obs/obs-plugins/
     ```

5. **Restart OBS Studio**
   ```bash
   killall obs
   obs &
   ```

### Building from Source (Advanced)

For developers or users who want the latest development version:

1. **Install Build Dependencies**
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential cmake git obs-studio-dev \
                    libssl-dev pkg-config

   # Fedora
   sudo dnf install gcc-c++ cmake git obs-studio-devel openssl-devel

   # Arch Linux
   sudo pacman -S base-devel cmake git obs-studio openssl
   ```

2. **Clone the Repository**
   ```bash
   git clone --recursive https://github.com/m96-chan/OBS-WebRTC-Link.git
   cd OBS-WebRTC-Link
   ```

3. **Build and Install**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_INSTALL_PREFIX=/usr
   make -j$(nproc)
   sudo make install
   ```

   For user installation (no sudo required):
   ```bash
   cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
   make -j$(nproc)
   make install
   ```

4. **Restart OBS Studio**

See [DEVELOPMENT.md](DEVELOPMENT.md) for more detailed build instructions.

### Linux Troubleshooting

**Plugin not loading:**
- Check OBS Studio logs: `~/.config/obs-studio/logs/`
- Verify library dependencies:
  ```bash
  ldd ~/.config/obs-studio/plugins/obs-webrtc-link.so
  ```
- Ensure all required libraries are installed

**Missing dependencies:**
```bash
# Ubuntu/Debian
sudo apt install libssl3 libstdc++6

# Fedora
sudo dnf install openssl-libs

# Arch Linux
sudo pacman -S openssl
```

**Permission errors:**
- Use user installation method instead of system-wide
- Or ensure proper file permissions:
  ```bash
  chmod 755 ~/.config/obs-studio/plugins/obs-webrtc-link.so
  ```

---

## Common Troubleshooting

### Plugin Not Appearing in OBS

1. **Verify OBS Studio Version**
   - Open OBS Studio → Help → About
   - Ensure version is 30.0 or later
   - If older, update OBS Studio first

2. **Check Plugin Files**
   - Ensure all plugin files were copied to the correct location
   - Windows: Check both `64bit` directory and `data` folder
   - macOS: Ensure entire `.plugin` bundle was copied
   - Linux: Verify both library and data files exist

3. **Review OBS Logs**
   - OBS Studio → Help → Log Files → View Current Log
   - Look for errors related to "webrtc" or "plugin loading"
   - On Linux: Check `~/.config/obs-studio/logs/`

### Connection Issues

**Cannot connect to SFU server:**
- Verify server URL is correct (should start with `http://` or `https://`)
- Check firewall settings allow WebRTC traffic (UDP ports)
- Ensure server supports WHIP/WHEP protocols
- Verify authentication token is valid

**P2P connection fails:**
- Ensure both peers are using the same Session ID
- Check NAT/firewall configuration
- Try using SFU mode if P2P continues to fail

### Performance Issues

**High CPU usage:**
- Enable hardware encoding (NVENC/AMF/QuickSync) in OBS settings
- Reduce video resolution or bitrate
- Close unnecessary programs

**Video stuttering or lag:**
- Check network bandwidth and stability
- Reduce bitrate in plugin settings
- Try using lower video resolution
- For P2P: Ensure both peers have good network conditions

---

## Uninstallation

### Windows

**Using Installer:**
1. Open "Settings" → "Apps" → "Apps & features"
2. Find "OBS-WebRTC-Link" in the list
3. Click "Uninstall" → Follow prompts

**Manual Removal:**
1. Navigate to OBS plugin directory
2. Delete `obs-webrtc-link.dll` and related files
3. Delete `data/obs-plugins/obs-webrtc-link/` folder
4. Restart OBS Studio

### macOS

**Using PKG Installer:**
- Run the uninstaller package (if provided)
- Or manually remove files as described below

**Manual Removal:**
```bash
rm -rf ~/Library/Application\ Support/obs-studio/plugins/obs-webrtc-link.plugin
# or for system-wide installation:
sudo rm -rf /Library/Application\ Support/obs-studio/plugins/obs-webrtc-link.plugin
```

### Linux

**Using Package Manager:**
```bash
# Ubuntu/Debian
sudo apt remove obs-webrtc-link

# Fedora/RHEL
sudo dnf remove obs-webrtc-link

# Arch Linux
sudo pacman -R obs-webrtc-link
```

**Manual Removal:**
```bash
rm -rf ~/.config/obs-studio/plugins/obs-webrtc-link.so
rm -rf ~/.local/share/obs/obs-plugins/obs-webrtc-link/
# or for system-wide:
sudo rm -rf /usr/lib/obs-plugins/obs-webrtc-link.so
sudo rm -rf /usr/share/obs/obs-plugins/obs-webrtc-link/
```

---

## Getting Help

If you encounter issues not covered in this guide:

1. **Check Existing Issues**: [GitHub Issues](https://github.com/m96-chan/OBS-WebRTC-Link/issues)
2. **Join Discussions**: [GitHub Discussions](https://github.com/m96-chan/OBS-WebRTC-Link/discussions)
3. **Report a Bug**: [Create New Issue](https://github.com/m96-chan/OBS-WebRTC-Link/issues/new)

When reporting issues, please include:
- Operating system and version
- OBS Studio version
- Plugin version
- Steps to reproduce the problem
- Relevant log files from OBS

---

## Next Steps

After installation, see the [README.md](../README.md) for usage instructions and configuration examples.

For developers interested in contributing, check out [CONTRIBUTING.md](CONTRIBUTING.md) and [DEVELOPMENT.md](DEVELOPMENT.md).

---
---

# 日本語

このガイドでは、Windows、macOS、Linux に OBS-WebRTC-Link プラグインをインストールする詳細な手順を説明します。

## システム要件

- **OBS Studio**: バージョン 30.0 以降
- **オペレーティングシステム**:
  - Windows 10/11 (64ビット)
  - macOS 11 (Big Sur) 以降
  - Linux (Ubuntu 20.04 LTS 以降、または同等のディストリビューション)
- **ハードウェア**:
  - ハードウェアエンコーディング対応 CPU 推奨 (NVENC/AMF/QuickSync)
  - 最小 4GB RAM
  - WebRTC ストリーミング用の安定したインターネット接続

---

## Windows インストール

### 方法1: インストーラーを使用 (推奨)

1. **インストーラーのダウンロード**
   - [リリースページ](https://github.com/m96-chan/OBS-WebRTC-Link/releases)にアクセス
   - Windows 用の最新 `.exe` インストーラーをダウンロード
   - ファイル名形式: `obs-webrtc-link-<version>-windows-installer.exe`

2. **インストーラーの実行**
   - ダウンロードしたインストーラーをダブルクリック
   - Windows SmartScreen が表示されたら、「詳細情報」→「実行」をクリック
   - インストールウィザードで「次へ」をクリック
   - インストーラーは OBS Studio のインストールパスを自動検出します
   - 自動検出されない場合は「参照」をクリックし、OBS インストールフォルダーに移動（通常は `C:\Program Files\obs-studio`）
   - 「インストール」をクリックしてインストールを開始

3. **インストールの完了**
   - インストールが完了するまで待機
   - 「完了」をクリック
   - **重要**: OBS Studio が実行中の場合は再起動してください

4. **インストールの確認**
   - OBS Studio を起動
   - 「ツール」メニュー → 「WebRTC Link 設定」などのオプションを確認
   - ソースパネルで「+」ボタンをクリック → リストに「WebRTC Link ソース」が表示されることを確認

### 方法2: ZIP からの手動インストール

1. **ZIP パッケージのダウンロード**
   - [リリースページ](https://github.com/m96-chan/OBS-WebRTC-Link/releases)にアクセス
   - Windows 用の最新 `.zip` ファイルをダウンロード
   - ファイル名形式: `obs-webrtc-link-<version>-windows.zip`

2. **ファイルの展開**
   - ダウンロードした ZIP ファイルを右クリックして「すべて展開」を選択
   - 一時的な展開場所を選択

3. **OBS プラグインディレクトリの場所**
   - エクスプローラーを開き、以下のいずれかの場所に移動:
     - **Program Files** (システム全体): `C:\Program Files\obs-studio\obs-plugins\64bit\`
     - **AppData** (ユーザー固有): `%APPDATA%\obs-studio\obs-plugins\64bit\`

4. **プラグインファイルのコピー**
   - 展開したフォルダーからすべてのファイルを OBS プラグインディレクトリにコピー:
     - `obs-webrtc-link.dll`
     - `obs-webrtc-link.pdb` (オプション、デバッグ用)
   - `data` フォルダーを親ディレクトリ (`obs-plugins/`) にコピー

5. **OBS Studio の再起動**
   - OBS Studio を完全に終了（システムトレイを確認）
   - OBS Studio を再起動
   - プラグインがツールメニューとソースリストに表示されることを確認

### Windows トラブルシューティング

**プラグインが OBS に表示されない:**
- ファイルをコピーする前に OBS Studio が完全に終了していることを確認
- 正しい `64bit` サブディレクトリにファイルをコピーしたことを確認
- OBS Studio のバージョンが 30.0 以降であることを確認
- Windows イベントビューアで DLL 読み込みエラーを確認

**DLL 不足エラー:**
- [Microsoft](https://aka.ms/vs/17/release/vc_redist.x64.exe) から Visual C++ 再頒布可能パッケージ 2019 以降をインストール
- すべてのプラグインファイルが正しくコピーされていることを確認

**インストール中の権限エラー:**
- インストーラーを右クリックして「管理者として実行」を選択
- またはユーザーディレクトリ (`%APPDATA%`) にインストール

---

## macOS インストール

### 方法1: PKG インストーラーを使用 (推奨)

1. **インストーラーのダウンロード**
   - [リリースページ](https://github.com/m96-chan/OBS-WebRTC-Link/releases)にアクセス
   - macOS 用の最新 `.pkg` インストーラーをダウンロード
   - ファイル名形式: `obs-webrtc-link-<version>-macos.pkg`

2. **インストーラーの実行**
   - ダウンロードした PKG ファイルをダブルクリック
   - macOS が「確認できない開発元」の警告を表示した場合:
     - 「OK」をクリック
     - 「システム設定」→「プライバシーとセキュリティ」を開く
     - 下にスクロールし、ブロックされたアプリメッセージの横にある「このまま開く」をクリック
   - インストールウィザードに従う
   - プロンプトが表示されたら管理者パスワードを入力

3. **インストールの完了**
   - プラグインは `/Library/Application Support/obs-studio/plugins/` にインストールされます
   - インストールが完了したら「閉じる」をクリック
   - OBS Studio が実行中の場合は再起動

4. **インストールの確認**
   - OBS Studio を起動
   - 「ツール」メニューで WebRTC Link オプションを確認
   - ソースリストに「WebRTC Link ソース」が表示されることを確認

### 方法2: ZIP からの手動インストール

1. **ZIP パッケージのダウンロード**
   - [リリースページ](https://github.com/m96-chan/OBS-WebRTC-Link/releases)にアクセス
   - macOS 用の最新 `.zip` ファイルをダウンロード
   - ファイル名形式: `obs-webrtc-link-<version>-macos.zip`

2. **ファイルの展開**
   - ZIP ファイルをダブルクリックして展開（またはターミナルで `unzip` を使用）

3. **OBS プラグインディレクトリの場所**
   - システム全体: `/Library/Application Support/obs-studio/plugins/`
   - ユーザー固有: `~/Library/Application Support/obs-studio/plugins/`
   - ディレクトリが存在しない場合は作成:
     ```bash
     mkdir -p ~/Library/Application\ Support/obs-studio/plugins/
     ```

4. **プラグインファイルのコピー**
   - ターミナルを開いて実行:
     ```bash
     cp -R obs-webrtc-link.plugin ~/Library/Application\ Support/obs-studio/plugins/
     ```
   - または `obs-webrtc-link.plugin` バンドルをプラグインフォルダーにドラッグ

5. **隔離属性の削除 (重要)**
   - macOS はダウンロードされたファイルを隔離する場合があります。隔離を削除:
     ```bash
     xattr -cr ~/Library/Application\ Support/obs-studio/plugins/obs-webrtc-link.plugin
     ```

6. **OBS Studio の再起動**
   - OBS Studio を完全に終了 (Cmd+Q)
   - OBS Studio を再起動

### macOS トラブルシューティング

**「開発元を確認できないため開けません」:**
- 上記の隔離削除コマンドを実行
- または: プラグインを右クリック → 「開く」→ 確認

**プラグインが読み込まれない:**
- Console.app で OBS からのエラーメッセージを確認
- macOS のバージョンが 11.0 以降であることを確認
- 個々のファイルだけでなく、`.plugin` バンドル全体をコピーしたことを確認

**権限の問題:**
- システムディレクトリではなくユーザーディレクトリへのインストールを試す
- またはシステム全体のインストールには `sudo` を使用（非推奨）

---

## Linux インストール

### 方法1: ディストリビューションパッケージを使用 (推奨)

#### Ubuntu/Debian (DEB パッケージ)

1. **DEB パッケージのダウンロード**
   - [リリースページ](https://github.com/m96-chan/OBS-WebRTC-Link/releases)にアクセス
   - 最新の `.deb` ファイルをダウンロード
   - ファイル名形式: `obs-webrtc-link_<version>_amd64.deb`

2. **ターミナル経由でインストール**
   ```bash
   sudo apt install ./obs-webrtc-link_<version>_amd64.deb
   ```
   - または DEB ファイルをダブルクリックしてソフトウェアセンターを開き、「インストール」をクリック

3. **OBS Studio の再起動**
   ```bash
   killall obs
   obs &
   ```

#### Fedora/RHEL (RPM パッケージ)

1. **RPM パッケージのダウンロード**
   - [リリースページ](https://github.com/m96-chan/OBS-WebRTC-Link/releases)にアクセス
   - 最新の `.rpm` ファイルをダウンロード
   - ファイル名形式: `obs-webrtc-link-<version>.x86_64.rpm`

2. **ターミナル経由でインストール**
   ```bash
   sudo dnf install ./obs-webrtc-link-<version>.x86_64.rpm
   ```
   - 古いシステムでは `yum` を使用

3. **OBS Studio の再起動**

#### Arch Linux (AUR)

1. **AUR からインストール**
   ```bash
   yay -S obs-webrtc-link
   # または好みの AUR ヘルパーを使用
   ```

2. **OBS Studio の再起動**

### 方法2: TAR.GZ からの手動インストール

1. **アーカイブのダウンロード**
   - [リリースページ](https://github.com/m96-chan/OBS-WebRTC-Link/releases)にアクセス
   - Linux 用の最新 `.tar.gz` ファイルをダウンロード
   - ファイル名形式: `obs-webrtc-link-<version>-linux-x86_64.tar.gz`

2. **アーカイブの展開**
   ```bash
   tar -xzf obs-webrtc-link-<version>-linux-x86_64.tar.gz
   cd obs-webrtc-link-<version>
   ```

3. **OBS プラグインディレクトリの場所**
   - システム全体: `/usr/lib/obs-plugins/` または `/usr/lib/x86_64-linux-gnu/obs-plugins/`
   - ユーザー固有: `~/.config/obs-studio/plugins/`
   - ユーザーディレクトリが存在しない場合は作成:
     ```bash
     mkdir -p ~/.config/obs-studio/plugins/
     ```

4. **プラグインファイルのコピー**
   - ユーザーインストールの場合:
     ```bash
     cp -R lib/obs-plugins/* ~/.config/obs-studio/plugins/
     cp -R share/obs/obs-plugins/* ~/.local/share/obs/obs-plugins/
     ```
   - システム全体のインストールの場合 (sudo が必要):
     ```bash
     sudo cp -R lib/obs-plugins/* /usr/lib/obs-plugins/
     sudo cp -R share/obs/obs-plugins/* /usr/share/obs/obs-plugins/
     ```

5. **OBS Studio の再起動**
   ```bash
   killall obs
   obs &
   ```

### ソースからのビルド (上級者向け)

開発者または最新の開発バージョンを必要とするユーザー向け:

1. **ビルド依存関係のインストール**
   ```bash
   # Ubuntu/Debian
   sudo apt install build-essential cmake git obs-studio-dev \
                    libssl-dev pkg-config

   # Fedora
   sudo dnf install gcc-c++ cmake git obs-studio-devel openssl-devel

   # Arch Linux
   sudo pacman -S base-devel cmake git obs-studio openssl
   ```

2. **リポジトリのクローン**
   ```bash
   git clone --recursive https://github.com/m96-chan/OBS-WebRTC-Link.git
   cd OBS-WebRTC-Link
   ```

3. **ビルドとインストール**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_INSTALL_PREFIX=/usr
   make -j$(nproc)
   sudo make install
   ```

   ユーザーインストールの場合 (sudo 不要):
   ```bash
   cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
   make -j$(nproc)
   make install
   ```

4. **OBS Studio の再起動**

詳細なビルド手順については [DEVELOPMENT.md](DEVELOPMENT.md) を参照してください。

### Linux トラブルシューティング

**プラグインが読み込まれない:**
- OBS Studio ログを確認: `~/.config/obs-studio/logs/`
- ライブラリ依存関係を確認:
  ```bash
  ldd ~/.config/obs-studio/plugins/obs-webrtc-link.so
  ```
- 必要なすべてのライブラリがインストールされていることを確認

**依存関係の不足:**
```bash
# Ubuntu/Debian
sudo apt install libssl3 libstdc++6

# Fedora
sudo dnf install openssl-libs

# Arch Linux
sudo pacman -S openssl
```

**権限エラー:**
- システム全体ではなくユーザーインストール方法を使用
- または適切なファイル権限を確保:
  ```bash
  chmod 755 ~/.config/obs-studio/plugins/obs-webrtc-link.so
  ```

---

## 共通のトラブルシューティング

### プラグインが OBS に表示されない

1. **OBS Studio バージョンの確認**
   - OBS Studio を開く → ヘルプ → バージョン情報
   - バージョンが 30.0 以降であることを確認
   - 古い場合は、まず OBS Studio を更新

2. **プラグインファイルの確認**
   - すべてのプラグインファイルが正しい場所にコピーされていることを確認
   - Windows: `64bit` ディレクトリと `data` フォルダーの両方を確認
   - macOS: `.plugin` バンドル全体がコピーされていることを確認
   - Linux: ライブラリとデータファイルの両方が存在することを確認

3. **OBS ログの確認**
   - OBS Studio → ヘルプ → ログファイル → 現在のログを表示
   - 「webrtc」または「plugin loading」に関連するエラーを探す
   - Linux の場合: `~/.config/obs-studio/logs/` を確認

### 接続の問題

**SFU サーバーに接続できない:**
- サーバー URL が正しいことを確認 (`http://` または `https://` で始まる)
- ファイアウォール設定で WebRTC トラフィック (UDP ポート) が許可されていることを確認
- サーバーが WHIP/WHEP プロトコルをサポートしていることを確認
- 認証トークンが有効であることを確認

**P2P 接続が失敗する:**
- 両方のピアが同じセッション ID を使用していることを確認
- NAT/ファイアウォール構成を確認
- P2P が引き続き失敗する場合は SFU モードを試す

### パフォーマンスの問題

**高い CPU 使用率:**
- OBS 設定でハードウェアエンコーディング (NVENC/AMF/QuickSync) を有効化
- ビデオ解像度またはビットレートを下げる
- 不要なプログラムを閉じる

**ビデオのカクつきや遅延:**
- ネットワーク帯域幅と安定性を確認
- プラグイン設定でビットレートを下げる
- より低いビデオ解像度を試す
- P2P の場合: 両方のピアが良好なネットワーク条件を持っていることを確認

---

## アンインストール

### Windows

**インストーラー使用:**
1. 「設定」→「アプリ」→「アプリと機能」を開く
2. リストから「OBS-WebRTC-Link」を見つける
3. 「アンインストール」をクリック → プロンプトに従う

**手動削除:**
1. OBS プラグインディレクトリに移動
2. `obs-webrtc-link.dll` と関連ファイルを削除
3. `data/obs-plugins/obs-webrtc-link/` フォルダーを削除
4. OBS Studio を再起動

### macOS

**PKG インストーラー使用:**
- アンインストーラーパッケージを実行（提供されている場合）
- または以下の手動削除方法に従う

**手動削除:**
```bash
rm -rf ~/Library/Application\ Support/obs-studio/plugins/obs-webrtc-link.plugin
# システム全体のインストールの場合:
sudo rm -rf /Library/Application\ Support/obs-studio/plugins/obs-webrtc-link.plugin
```

### Linux

**パッケージマネージャー使用:**
```bash
# Ubuntu/Debian
sudo apt remove obs-webrtc-link

# Fedora/RHEL
sudo dnf remove obs-webrtc-link

# Arch Linux
sudo pacman -R obs-webrtc-link
```

**手動削除:**
```bash
rm -rf ~/.config/obs-studio/plugins/obs-webrtc-link.so
rm -rf ~/.local/share/obs/obs-plugins/obs-webrtc-link/
# システム全体の場合:
sudo rm -rf /usr/lib/obs-plugins/obs-webrtc-link.so
sudo rm -rf /usr/share/obs/obs-plugins/obs-webrtc-link/
```

---

## サポート

このガイドでカバーされていない問題が発生した場合:

1. **既存の Issue を確認**: [GitHub Issues](https://github.com/m96-chan/OBS-WebRTC-Link/issues)
2. **ディスカッションに参加**: [GitHub Discussions](https://github.com/m96-chan/OBS-WebRTC-Link/discussions)
3. **バグを報告**: [新しい Issue を作成](https://github.com/m96-chan/OBS-WebRTC-Link/issues/new)

問題を報告する際は、以下を含めてください:
- オペレーティングシステムとバージョン
- OBS Studio のバージョン
- プラグインのバージョン
- 問題を再現する手順
- OBS からの関連ログファイル

---

## 次のステップ

インストール後、使用方法と設定例については [README.md](../README.md) を参照してください。

開発に貢献することに興味がある開発者は、[CONTRIBUTING.md](CONTRIBUTING.md) と [DEVELOPMENT.md](DEVELOPMENT.md) を確認してください。
