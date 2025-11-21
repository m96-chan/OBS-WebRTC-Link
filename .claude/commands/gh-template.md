---
description: GitHub テンプレート、ラベル、マイルストーンを作成
---

## 前提条件の確認

まず、以下の前提条件を確認してください。

1. GitHub CLIが認証済みであることを確認する（`gh auth status`）
   - 未認証の場合は `gh auth login` を実行してユーザーに指示を求める
2. 現在のディレクトリがGitリポジトリであることを確認する
   - リポジトリでない場合は、エラーメッセージを表示して処理を中断する
3. リモートリポジトリが設定されていることを確認する
   - 設定されていない場合は、エラーメッセージを表示して処理を中断する

## タスクの実行

.github/ ディレクトリ内にテンプレートファイル一式を作成し、README.mdから想定されるラベルとマイルストーンを作成してください。

### 1. README.mdの分析（ラベル・マイルストーン作成用）

1. **README.mdが存在する場合は読み取る**
   - プロジェクトの目的と概要を理解
   - 主要な機能リストを抽出
   - 技術スタックを確認
   - マイルストーンになりそうなフェーズ・バージョン情報を抽出

2. **ラベルの設計**
   以下のカテゴリでラベルを作成:

   **優先度ラベル（Priority）**:
   - `priority:p0` - Critical（赤: #d73a4a）
   - `priority:p1` - High（オレンジ: #e99695）
   - `priority:p2` - Medium（黄: #fbca04）
   - `priority:p3` - Low（緑: #0e8a16）

   **タイプラベル（Type）**:
   - `type:feature` - 新機能（青: #0075ca）
   - `type:bug` - バグ修正（赤: #d73a4a）
   - `type:docs` - ドキュメント（灰: #7057ff）
   - `type:test` - テスト（緑: #0e8a16）
   - `type:refactor` - リファクタリング（紫: #d876e3）
   - `type:setup` - セットアップ（茶: #8b4513）
   - `type:chore` - 雑務（灰: #fef2c0）

   **ステータスラベル（Status）**:
   - `status:todo` - 未着手（白: #ffffff）
   - `status:in-progress` - 進行中（黄: #fbca04）
   - `status:review` - レビュー中（青: #0075ca）
   - `status:blocked` - ブロック中（赤: #d73a4a）
   - `status:done` - 完了（緑: #0e8a16）

   **README.mdから追加で抽出**:
   - プロジェクト固有の技術スタックに関するラベル
   - 特定の機能領域に関するラベル（例: `area:frontend`, `area:backend`）

3. **マイルストーンの設計**
   README.mdから以下を抽出してマイルストーンを作成:
   - バージョン番号（例: `v1.0.0`, `v2.0.0`）
   - フェーズ名（例: `MVP`, `Beta Release`, `Production Ready`）
   - 機能グループ（例: `Core Features`, `Advanced Features`）

   デフォルトのマイルストーン:
   - `v0.1.0 - Initial Setup` - プロジェクトの初期セットアップ
   - `v1.0.0 - MVP` - 最小機能プロダクト
   - `v2.0.0 - Full Release` - 完全版リリース

### 2. ブランチの準備

1. mainにチェックアウトし、pullを行い、最新のリモートの状態を取得する
2. タスクの内容を元に、適切な命名でブランチを作成、チェックアウトする（例: feat/add-github-templates）

### 3. テンプレートファイルの作成

以下のファイルを.github/ディレクトリ内に作成する。【重要】TDDができるようなテンプレートにすること

**ファイル作成前に以下を確認**:
- 既存のテンプレートファイルが存在する場合は、ユーザーに上書き確認を行う
- ユーザーが拒否した場合は、既存ファイルを保持したまま処理を継続

**作成するファイル**:
- `.github/PULL_REQUEST_TEMPLATE.md`
- `.github/ISSUE_TEMPLATE/bug_report.md`
- `.github/ISSUE_TEMPLATE/feature_request.md`

### 4. ラベルの作成

`gh label create` コマンドを使用してラベルを作成:

```bash
# 優先度ラベル
gh label create "priority:p0" --description "Critical priority" --color "d73a4a"
gh label create "priority:p1" --description "High priority" --color "e99695"
gh label create "priority:p2" --description "Medium priority" --color "fbca04"
gh label create "priority:p3" --description "Low priority" --color "0e8a16"

# タイプラベル
gh label create "type:feature" --description "New feature" --color "0075ca"
gh label create "type:bug" --description "Bug fix" --color "d73a4a"
gh label create "type:docs" --description "Documentation" --color "7057ff"
gh label create "type:test" --description "Testing" --color "0e8a16"
gh label create "type:refactor" --description "Code refactoring" --color "d876e3"
gh label create "type:setup" --description "Setup and configuration" --color "8b4513"
gh label create "type:chore" --description "Maintenance tasks" --color "fef2c0"

# ステータスラベル
gh label create "status:todo" --description "Not started" --color "ffffff"
gh label create "status:in-progress" --description "Work in progress" --color "fbca04"
gh label create "status:review" --description "In review" --color "0075ca"
gh label create "status:blocked" --description "Blocked" --color "d73a4a"
gh label create "status:done" --description "Completed" --color "0e8a16"

# README.mdから抽出した追加ラベル
# ...
```

**注意**:
- 既存のラベルがある場合はスキップする（エラーを無視）
- ラベル作成結果を記録し、最後に報告する

### 5. マイルストーンの作成

`gh api` コマンドを使用してマイルストーンを作成:

```bash
# デフォルトマイルストーンの作成
gh api repos/{owner}/{repo}/milestones -f title="v0.1.0 - Initial Setup" -f description="プロジェクトの初期セットアップとインフラ構築" -f due_on="2024-12-31T23:59:59Z"
gh api repos/{owner}/{repo}/milestones -f title="v1.0.0 - MVP" -f description="最小機能プロダクトの完成" -f due_on="2025-03-31T23:59:59Z"
gh api repos/{owner}/{repo}/milestones -f title="v2.0.0 - Full Release" -f description="完全版のリリース" -f due_on="2025-06-30T23:59:59Z"

# README.mdから抽出した追加マイルストーン
# ...
```

**注意**:
- 期限（due_on）は現在の日付から適切な期間を計算して設定
- README.mdに明示的な期限がある場合はそれを使用
- 既存のマイルストーンがある場合はスキップ
- マイルストーン作成結果を記録し、最後に報告する

### 6. コミットとPR作成

1. 作成したファイルを適切な粒度でコミットする
2. テストとLintは不要
3. 以下のルールに従ってPRを作成する:
   - PRのdescriptionのテンプレートは @.github/PULL_REQUEST_TEMPLATE.md を参照し、それに従うこと
   - PRのdescriptionのテンプレート内でコメントアウトされている箇所は必ず削除すること
   - PRのdescriptionにはCloses #$ARGUMENTSと記載すること（Issue番号が指定されている場合）

### 7. 作成結果の報告

すべての作成が完了したら、以下の形式で報告:

```markdown
## GitHub テンプレート・ラベル・マイルストーン作成完了

### 作成されたテンプレートファイル
- ✓ .github/PULL_REQUEST_TEMPLATE.md
- ✓ .github/ISSUE_TEMPLATE/bug_report.md
- ✓ .github/ISSUE_TEMPLATE/feature_request.md

### 作成されたラベル（XX件）
**優先度**:
- priority:p0 (Critical)
- priority:p1 (High)
- priority:p2 (Medium)
- priority:p3 (Low)

**タイプ**:
- type:feature
- type:bug
- type:docs
- type:test
- type:refactor
- type:setup
- type:chore

**ステータス**:
- status:todo
- status:in-progress
- status:review
- status:blocked
- status:done

**プロジェクト固有**:
- [抽出されたラベル一覧]

### 作成されたマイルストーン（XX件）
- v0.1.0 - Initial Setup (期限: YYYY-MM-DD)
- v1.0.0 - MVP (期限: YYYY-MM-DD)
- v2.0.0 - Full Release (期限: YYYY-MM-DD)
- [README.mdから抽出されたマイルストーン]

### 次のステップ
プロジェクトの Issue を作成するには:
```
/gh-issues-from-readme
```

ラベルを確認するには:
```
gh label list
```

マイルストーンを確認するには:
```
gh api repos/{owner}/{repo}/milestones
```
```

## オプション引数

ユーザーが追加の引数を指定した場合：
- `--skip-labels`: ラベルの作成をスキップ
- `--skip-milestones`: マイルストーンの作成をスキップ
- `--templates-only`: テンプレートファイルのみ作成（ラベル・マイルストーンをスキップ）

## 注意事項

- ラベルやマイルストーンが既に存在する場合はエラーを無視して継続
- README.mdが存在しない場合はデフォルトのラベル・マイルストーンのみ作成
- マイルストーンの期限は現在の日付から適切に計算すること
- ラベルの色は16進数カラーコード（#なし）で指定すること