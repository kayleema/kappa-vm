# KappaVM

スタックベースの高性能でスケーラブルな仮想マシン。

## 概要

KappaVMは、スタックベースのバイトコードを実行するための堅牢で効率的な仮想マシンとして設計されています。スクリプト言語から複雑な計算タスクまで、さまざまなアプリケーションに対して高いパフォーマンスとスケーラビリティを提供することを目指しています。

## 特徴

- **スタックベースのアーキテクチャ**: シンプルさと効率性を追求したスタックベースの実行モデルを採用。
- **高性能**: 命令実行時のオーバーヘッドを最小限に抑えることに重点を置いた高速化。
- **スケーラビリティ**: 小規模なスクリプトから大規模なアプリケーションまで、幅広いワークロードに対応する設計。
- **カスタムアセンブラ**: 人間が読めるアセンブリコードをKappaVMバイトコードに変換するアセンブラを搭載。
- **拡張性のある設計**: 新しいオペコードや機能を簡単に追加可能。

## インストール

### 前提条件

- CMake（バージョン3.10以上）
- Cコンパイラ（例：GCCまたはClang）

### ソースからのビルド

1. リポジトリをクローンする：
   ```bash
   git clone https://github.com/kayleema/kappa-vm.git
   cd kappavm
   ```

2. ビルドディレクトリを作成し、そこに移動する：
   ```bash
   mkdir build
   cd build
   ```

3. CMakeを実行してビルドを構成する：
   ```bash
   cmake ..
   ```

4. プロジェクトをビルドする：
   ```bash
   make
   ```

5. コンパイルされたバイナリ `kappavm` が `build` ディレクトリに生成されます。

## 使用方法

### KappaVMバイトコードファイルの実行

バイトコードファイル（例：`test_bytecode.kbc`）を実行するには、以下のコマンドを使用します：

```bash
./build/kappavm test_bytecode.kbc
```

### Kappaアセンブリコードのアセンブル

Kappaアセンブリファイル（例：`test_assembly.kappa`）をバイトコードにアセンブルするには：

```bash
# プロジェクトにアセンブラユーティリティが含まれていると仮定
./build/kappavm --assemble test_assembly.kappa test_bytecode.kbc
```

## Kappaバイトコードの逆アセンブル

KappaVMは、デバッグや分析のためにバイトコードを人間が読めるアセンブリコードに逆アセンブルすることができます。これは、アセンブラによって生成されたバイトコードを理解したり、実行フローの問題をトラブルシューティングしたりするのに役立ちます。

バイトコードファイル（例：`test_bytecode.kbc`）をアセンブリコードに逆アセンブルするには：

```bash
./build/kappavm --dis test_bytecode.kbc > test_disassembled.kappa
```

このコマンドは、逆アセンブルされたコードを `test_disassembled.kappa` に出力し、必要に応じて確認や修正が可能です。

## プロジェクト構造

- **`assembler.c`, `assembler.h`**: Kappaアセンブリ言語をバイトコードにアセンブルするためのコード。
- **`chunk.c`, `chunk.h`**: 命令のシーケンスであるバイトコードチャンクを管理。
- **`vm.c`, `vm.h`**: バイトコードを実行するためのコア仮想マシン実装。
- **`opcode.h`**: KappaVMの命令セットを定義。
- **`value.h`**: VM内で使用されるデータ型と値を処理。
- **`tests/`**: KappaVMのさまざまなコンポーネントのテストファイルを含むディレクトリ。
- **`main.c`**: KappaVM実行ファイルのエントリーポイント。

## テスト

KappaVMには、コンポーネントの信頼性を確保するための一連のテストが含まれています。テストをビルドして実行するには：

1. `build` ディレクトリにいることを確認してください。
2. 以下のコマンドを実行します：
   ```bash
   make test
   ```

これにより、アセンブラ、VM実行、その他の重要なコンポーネントのテストがコンパイルされ、実行されます。

## 貢献

KappaVMへの貢献を歓迎します！貢献したい場合は、以下の手順に従ってください：

1. リポジトリをフォークする。
2. 機能追加やバグ修正のための新しいブランチを作成する。
3. 変更を行い、説明的なメッセージとともにコミットする。
4. ブランチをフォークにプッシュする。
5. メインリポジトリにプルリクエストを提出する。

コードがプロジェクトのコーディング標準に準拠し、適切なテストが含まれていることを確認してください。

## 連絡先

質問やサポートについては、GitHubリポジトリでissueを開くか、メンテナーに連絡してください。
