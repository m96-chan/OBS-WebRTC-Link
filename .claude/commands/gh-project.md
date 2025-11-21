---
description: GitHub Projectsを作成し、Issueと連携する
---

## 前提条件の確認

まず、以下の前提条件を確認してください。

1. GitHub CLIが認証済みであることを確認する（`gh auth status`）
   - 未認証の場合は `gh auth login` を実行してユーザーに指示を求める
2. 現在のディレクトリがGitリポジトリであることを確認する
   - リポジトリでない場合は、エラーメッセージを表示して処理を中断する
3. リモートリポジトリが設定されていることを確認する
   - 設定されていない場合は、エラーメッセージを表示して処理を中断する
4. GitHub Projects (Beta) の機能が利用可能であることを確認
   - 必要に応じてGitHub CLIのバージョン確認（`gh --version`）

## GitHub Projects作成の実行

README.mdからプロジェクトの内容を理解し、適切な GitHub Project を作成してください。

### 1. README.mdの読み取りと分析

1. **README.mdが存在する場合は読み取る**
   - プロジェクトの目的と概要を理解
   - 主要な機能リストを抽出
   - 開発フェーズやマイルストーンを確認
   - ワークフローの特徴を理解

2. **プロジェクトボードのレイアウトを決定**
   以下のいずれかのレイアウトを選択:

   **推奨: カンバンボード（Kanban Board）**
   - ステータスベースの管理に適している
   - 列: `Backlog`, `Todo`, `In Progress`, `In Review`, `Done`

   **代替: ロードマップビュー（Roadmap View）**
   - 時系列での管理に適している
   - マイルストーンとの連携が強い

   **代替: テーブルビュー（Table View）**
   - 詳細な情報管理に適している
   - カスタムフィールドを多用する場合

### 2. プロジェクト名と説明の決定

README.mdの内容から以下を決定:

**プロジェクト名**:
- `[プロジェクト名] Development` - 開発全体の管理
- `[プロジェクト名] v1.0.0` - 特定バージョンの管理
- `[プロジェクト名] Roadmap` - ロードマップ管理

**プロジェクト説明**:
- README.mdの概要を元に簡潔な説明を作成
- 主要な目標やマイルストーンを含める

### 3. カスタムフィールドの設計

プロジェクトに以下のカスタムフィールドを追加:

**優先度（Priority）** - 単一選択:
- P0 - Critical 🔴
- P1 - High 🟠
- P2 - Medium 🟡
- P3 - Low 🟢

**見積もり（Estimate）** - 数値:
- ストーリーポイントまたは時間（単位: 日）

**担当領域（Area）** - 単一選択:
- Frontend
- Backend
- Database
- Infrastructure
- Documentation
- Testing
- (README.mdから抽出した追加領域)

**ステータス（Status）** - 単一選択（自動生成される列）:
- Backlog
- Todo
- In Progress
- In Review
- Done

**反復（Iteration）** - イテレーション:
- 2週間スプリントを設定（オプション）

### 4. GitHub Projectの作成

`gh project create` コマンドを使用してプロジェクトを作成:

```bash
# リポジトリレベルのプロジェクトを作成
gh project create --owner [owner] --title "[プロジェクト名] Development" --body "[説明]"
```

または、組織レベルのプロジェクトを作成:

```bash
# 組織レベルのプロジェクト
gh project create --org [organization] --title "[プロジェクト名] Development" --body "[説明]"
```

### 5. プロジェクトビューの設定

作成したプロジェクトに対して、複数のビューを作成:

**1. カンバンボードビュー（デフォルト）**
```bash
gh project field-create [project-number] --name "Status" --data-type "SINGLE_SELECT" --single-select-options "Backlog,Todo,In Progress,In Review,Done"
```

列の設定:
- Backlog: 優先順位付け待ちのタスク
- Todo: 着手可能な状態のタスク
- In Progress: 現在作業中のタスク
- In Review: レビュー待ちのタスク
- Done: 完了したタスク

**2. ロードマップビュー（オプション）**
- マイルストーンと連携
- 時系列での進捗確認

**3. テーブルビュー**
- すべてのフィールドを表示
- フィルタリングやソートが可能

### 6. カスタムフィールドの追加

```bash
# 優先度フィールド
gh project field-create [project-number] --name "Priority" --data-type "SINGLE_SELECT" --single-select-options "P0 - Critical,P1 - High,P2 - Medium,P3 - Low"

# 見積もりフィールド
gh project field-create [project-number] --name "Estimate" --data-type "NUMBER"

# 担当領域フィールド
gh project field-create [project-number] --name "Area" --data-type "SINGLE_SELECT" --single-select-options "Frontend,Backend,Database,Infrastructure,Documentation,Testing"
```

### 7. 既存Issueとの連携（オプション）

既存のIssueがある場合、プロジェクトに追加:

```bash
# リポジトリ内のすべてのオープンなIssueを取得
gh issue list --json number,title,labels --limit 100 | jq -r '.[] | .number' | while read issue_number; do
  gh project item-add [project-number] --owner [owner] --url "https://github.com/[owner]/[repo]/issues/$issue_number"
done
```

**注意**:
- 大量のIssueがある場合は、ユーザーに確認を求める
- ラベルに基づいて自動的にステータスを設定

### 8. 自動化ワークフローの設定

GitHub Projects の自動化機能を設定:

**推奨される自動化**:
1. **Issue作成時**: 自動的に `Todo` 列に追加
2. **PR作成時**: 自動的に `In Progress` 列に移動
3. **PR マージ時**: 自動的に `Done` 列に移動
4. **Issue クローズ時**: 自動的に `Done` 列に移動

```bash
# GitHub Projects Beta では、WebUIでの設定が推奨されます
# または、GitHub Actions を使用した自動化
```

### 9. プロジェクト作成結果の報告

すべての設定が完了したら、以下の形式で報告:

```markdown
## GitHub Project 作成完了

### プロジェクト情報
- **名前**: [プロジェクト名] Development
- **URL**: https://github.com/users/[owner]/projects/[number]
- **説明**: [プロジェクトの説明]

### 作成されたビュー
- ✓ カンバンボード（デフォルト）
  - 列: Backlog → Todo → In Progress → In Review → Done
- ✓ ロードマップビュー（オプション）
- ✓ テーブルビュー

### カスタムフィールド
- ✓ Priority (P0/P1/P2/P3)
- ✓ Estimate (数値)
- ✓ Area (Frontend/Backend/Database/etc.)
- ✓ Status (Backlog/Todo/In Progress/In Review/Done)

### 連携されたIssue
- 合計 XX 件のIssueをプロジェクトに追加しました

### 推奨される自動化設定
以下の自動化をGitHub Projects UIで設定することを推奨します:
1. Issue作成時 → Todoに自動追加
2. PR作成時 → In Progressに自動移動
3. PRマージ時 → Doneに自動移動
4. Issueクローズ時 → Doneに自動移動

### 次のステップ

プロジェクトボードを表示:
```
gh project view [project-number]
```

Issueをプロジェクトに追加:
```
gh project item-add [project-number] --owner [owner] --url [issue-url]
```

WebUIでプロジェクトを開く:
```
https://github.com/users/[owner]/projects/[number]
```

Issueを一括作成してプロジェクトに追加:
```
/gh-issues-from-readme --milestone="v1.0.0"
```
```

## オプション引数

ユーザーが追加の引数を指定した場合：
- `--name=[name]`: プロジェクト名を明示的に指定
- `--org=[organization]`: 組織レベルのプロジェクトを作成
- `--template=[kanban|roadmap|table]`: レイアウトテンプレートを指定
- `--link-issues`: 既存のIssueをすべてプロジェクトに追加
- `--with-iterations`: 2週間スプリント（イテレーション）を設定

## 使用例

```bash
# デフォルトのプロジェクト作成
/gh-project

# 組織レベルのプロジェクト作成
/gh-project --org=my-organization

# 既存Issueと連携
/gh-project --link-issues

# イテレーション（スプリント）を含む
/gh-project --with-iterations

# カスタム名を指定
/gh-project --name="Q1 2024 Roadmap"
```

## 注意事項

- GitHub Projects (Beta) は GitHub CLI で完全にサポートされていない機能があるため、一部の設定は Web UI で行う必要があります
- プロジェクトの作成には適切な権限が必要です（リポジトリの書き込み権限または組織の管理者権限）
- 既存のプロジェクトと名前が重複する場合は、ユーザーに確認を求めます
- README.mdが存在しない場合は、デフォルトのプロジェクト設定を使用します
- 大量のIssue（50件以上）を連携する場合は、ユーザーに確認を求めます

## GitHub Projects Beta について

GitHub Projects (Beta) は従来の Projects とは異なる新しい機能です:
- より柔軟なカスタムフィールド
- 複数のビュー（カンバン、テーブル、ロードマップ）
- 強力な自動化機能
- イテレーション管理

詳細: https://docs.github.com/en/issues/planning-and-tracking-with-projects
