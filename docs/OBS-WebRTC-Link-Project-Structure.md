# OBS-WebRTC-Link Project Structure

標準的な OBS プラグイン（CMake ベース）の構成に、WebRTC の複雑なロジックと Docker 環境を整理するための構造を加えた構成例です。

```
obs-webrtc-link/
├── .github/                 # GitHub Actions (CI/CD設定)
│   └── workflows/
│       ├── build.yml        # Windows/Mac/Linux 自動ビルド
│       └── release.yml      # リリースタグ時のインストーラー作成
│
├── cmake/                   # CMakeのモジュール/ヘルパーファイル
│   └── FindLibDataChannel.cmake
│
├── data/                    # プラグインのリソースファイル
│   └── locale/              # 多言語対応ファイル
│       ├── en-US.ini
│       └── ja-JP.ini
│
├── deps/                    # 外部依存ライブラリ (Submodule推奨)
│   ├── libdatachannel/      # WebRTC実装
│   └── nlohmann-json/       # JSONパーサー
│
├── docker/                  # 開発・テスト用 Docker
│   └── livekit/
│       ├── docker-compose.yml
│       └── livekit.yaml
│
├── docs/                    # ドキュメント
│   ├── images/
│   └── INSTALL.md
│
├── src/
│   ├── core/                # WebRTC通信コアロジック
│   │   ├── peer-connection.cpp
│   │   ├── peer-connection.hpp
│   │   ├── signaling-client.cpp
│   │   └── whip-client.cpp
│   │
│   ├── output/              # OBS 出力機能
│   │   ├── webrtc-output.cpp
│   │   └── webrtc-output.hpp
│   │
│   ├── source/              # OBS ソース機能
│   │   ├── webrtc-source.cpp
│   │   └── webrtc-source.hpp
│   │
│   ├── ui/                  # 設定画面 (Qt)
│   │   ├── forms/
│   │   └── settings-dialog.cpp
│   │
│   └── plugin-main.cpp      # obs_module_load エントリポイント
│
├── .gitignore
├── .gitmodules
├── CMakeLists.txt
├── LICENSE
└── README.md
```

---

## 🔍 各ディレクトリのポイント

### 1. `src/core`（最重要）
「送信」「受信」両方に共通する WebRTC 接続のロジックをここに集約します。

含まれる処理：
- Offer/Answer 交換
- ICE candidate 管理
- Signaling（WHIP/WHEP の場合も統合）

OBS 依存ロジックから分離することで **保守・テストが容易** になります。

---

### 2. `src/output` & `src/source`
OBS の libobs API を扱う層。

#### output:
- エンコード済みフレームを WebRTC として送信
- 推奨: NVENC/AMF/QuickSync などの HW エンコード利用

#### source:
- 受信 WebRTC ストリームをデコード
- OBS のテクスチャに描画してソース表示

---

### 3. `data/locale`
OBS プラグインは `.ini` 形式での多言語対応が必須です。

例（en-US.ini）:

```
WebRTCLink="WebRTC Link"
Output.Mode="Connection Mode"
Output.Start="Start Streaming"
```

OSS として公開するなら最低限 `en-US.ini` は必須です。  
日本ユーザー向けに `ja-JP.ini` も配置します。

---

### 4. `docker/`（開発環境）
ローカルで LiveKit などの SFU を即時起動するための環境。

利点：
- 毎回 LiveKit コマンドを手動入力する必要なし
- ポート・APIキーなど構成が共通化できる
- チーム全体で同じテスト環境を共有できる

---

### 5. `deps/`（依存ライブラリ）
WebRTC ライブラリ（libdatachannel など）は OS パッケージに存在しないことが多いので：

- Git Submodule として同梱  
または  
- CMake FetchContent でビルド時に取得

が最適です。

---

## 🧩 まとめ

この構成は以下を最大限意識しています：

- **OBS依存ロジックとWebRTCロジックの分離**
- **ビルド環境の再現性確保（Docker / CMake）**
- **SFU / P2P の両対応**
- **OSS プロジェクトとしての標準構成**

必要であれば「最初からこの構成で動くテンプレプロジェクト」も生成できます。

