# OpenBlink プロジェクト概要

このリポジトリはM5Stack向けのmruby/cベースのプログラミング環境「OpenBlink」のソースコードを含んでいます。

## 主要コンポーネント

- **app.rb**: メインアプリケーションファイル
- **M5-SAM2695/**: M5関連のライブラリ
- **M5StampFly/**: M5関連のライブラリ
- **openblink-demo-m5/**: OpenBlinkのデモプロジェクト

## ドキュメント

このディレクトリには、OpenBlinkプロジェクトに関する各種ドキュメントが含まれています。

- [実装詳細](./implementation_details.md) - 詳細な実装情報とAPI仕様の詳細
- [APIリファレンス](./api_reference.md) - 利用可能なAPIの一覧と使用方法
- [インストール手順](./installation.md) - OpenBlinkのインストール方法
- [チュートリアル](./tutorial.md) - 初心者向けの使い方チュートリアル

## 使い方

```ruby
# サンプルコード
# 基本的な使用例
def setup
  M5.begin
  puts "Hello, OpenBlink!"
end

def loop
  if Input.a_pressed?
    puts "Button A pressed!"
  end
  M5.update
  sleep 0.1
end
``` 