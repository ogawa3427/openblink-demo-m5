# OpenBlink Demo M5

English text is followed by Chinese and Japanese translations. / 英文后面是中文和日文翻译。/ 英語の文章の後に中国語と日本語訳が続きます。

## What is OpenBlink

**_OpenBlink_** is an open source project forked from **_ViXion Blink_**.

- Ruby, a highly productive lightweight language, can be used to develop embedded devices.
- Program rewriting and debugging console are completely wireless. (BluetoothLE)
- Rewriting time is less than 0.1 second and does not involve a microprocessor restart. (We call it "Blink".)

## How to Get OpenBlink

To clone the repository and initialize the submodules, run the following commands:

```console
$ git clone https://github.com/OpenBlink/openblink-demo-m5.git
$ pio run
$ pio run -t erase && pio run -t upload
```

## Verified Hardware

The following hardware platforms have been tested with OpenBlink:

- M5 StampS3 (Espressif ESP32-S3FN8)

## mruby/c LED Control API

OpenBlink provides a simple API for controlling the onboard RGB LED through mruby/c.

### Available Classes and Methods

#### LED Class

- `LED.set([r, g, b])` - Sets the RGB LED color. Each value should be between 0-255.

#### Blink Class

- `Blink.req_reload?` - Checks if a code reload is requested.

### Example: LED Blinking Code

Here's a simple example that makes the LED blink in different colors:

```ruby
# RGB LED Blinking Example
while true do
  # Red
  LED.set([255, 0, 0])
  sleep 1

  # Green
  LED.set([0, 255, 0])
  sleep 1

  # Blue
  LED.set([0, 0, 255])
  sleep 1

  # Check if reload is requested
  break if Blink.req_reload?
end
```

This example demonstrates:

- Setting RGB LED colors using the `LED.set` method
- Using arrays to specify RGB values
- Implementing a clean exit when code reload is requested
- The `break if Blink.req_reload?` statement is crucial in OpenBlink applications. It allows the program to gracefully exit the current execution loop when a code reload is requested through the Bluetooth interface. Without this check, the program would continue running and ignore reload requests, making development and debugging difficult. This mechanism is what enables the "Blink" feature - the ability to update code wirelessly in less than 0.1 seconds without restarting the microprocessor.

# OpenBlink Demo M5 (中文)

## 什么是 OpenBlink

**_OpenBlink_** 是一个从 **_ViXion Blink_** 分支出来的开源项目。

- 可以使用 Ruby 这种高效轻量级语言来开发嵌入式设备。
- 程序重写和调试控制台完全无线。（蓝牙低功耗）
- 重写时间少于 0.1 秒，且不涉及微处理器重启。（我们称之为"Blink"）

## 如何获取 OpenBlink

要克隆仓库并初始化子模块，请运行以下命令：

```console
$ git clone https://github.com/OpenBlink/openblink-demo-m5.git
$ pio run
$ pio run -t erase && pio run -t upload
```

## 已验证的硬件

以下硬件平台已经过 OpenBlink 测试：

- M5 StampS3 (Espressif ESP32-S3FN8)

## mruby/c LED 控制 API

OpenBlink 提供了一个简单的 API，通过 mruby/c 控制板载 RGB LED。

### 可用的类和方法

#### LED 类

- `LED.set([r, g, b])` - 设置 RGB LED 颜色。每个值应在 0-255 之间。

#### Blink 类

- `Blink.req_reload?` - 检查是否请求代码重载。

### 示例：LED 闪烁代码

这是一个使 LED 以不同颜色闪烁的简单示例：

```ruby
# RGB LED 闪烁示例
while true do
  # 红色
  LED.set([255, 0, 0])
  sleep 1

  # 绿色
  LED.set([0, 255, 0])
  sleep 1

  # 蓝色
  LED.set([0, 0, 255])
  sleep 1

  # 检查是否请求重载
  break if Blink.req_reload?
end
```

此示例演示了：

- 使用`LED.set`方法设置 RGB LED 颜色
- 使用数组指定 RGB 值
- 在请求代码重载时实现干净退出
- `break if Blink.req_reload?` 语句在 OpenBlink 应用程序中至关重要。它允许程序在通过蓝牙接口请求代码重载时优雅地退出当前执行循环。如果没有这个检查，程序将继续运行并忽略重载请求，使开发和调试变得困难。这种机制正是实现"Blink"功能的关键 - 能够在不重启微处理器的情况下，以不到 0.1 秒的时间无线更新代码。

# OpenBlink Demo M5 (日本語)

## OpenBlink とは

**_OpenBlink_** は **_ViXion Blink_** からフォークされたオープンソースプロジェクトです。

- 高い生産性を持つ軽量言語 Ruby を使用して組み込みデバイスを開発できます。
- プログラムの書き換えとデバッグコンソールは完全にワイヤレスです。（BluetoothLE）
- 書き換え時間は 0.1 秒未満で、マイクロプロセッサの再起動を必要としません。（これを「Blink」と呼んでいます）

## OpenBlink の入手方法

リポジトリをクローンしてサブモジュールを初期化するには、次のコマンドを実行します：

```console
$ git clone https://github.com/OpenBlink/openblink-demo-m5.git
$ pio run
$ pio run -t erase && pio run -t upload
```

## 検証済みハードウェア

以下のハードウェアプラットフォームは OpenBlink でテスト済みです：

- M5 StampS3 (Espressif ESP32-S3FN8)

## mruby/c LED 制御 API

OpenBlink は mruby/c を通じて基板上の RGB LED を制御するためのシンプルな API を提供しています。

### 利用可能なクラスとメソッド

#### LED クラス

- `LED.set([r, g, b])` - RGB LED の色を設定します。各値は 0〜255 の間である必要があります。

#### Blink クラス

- `Blink.req_reload?` - コードのリロードが要求されているかどうかを確認します。

### 例：LED 点滅コード

異なる色で LED を点滅させる簡単な例です：

```ruby
# RGB LED 点滅例
while true do
  # 赤
  LED.set([255, 0, 0])
  sleep 1

  # 緑
  LED.set([0, 255, 0])
  sleep 1

  # 青
  LED.set([0, 0, 255])
  sleep 1

  # リロードが要求されているか確認
  break if Blink.req_reload?
end
```

この例は以下を示しています：

- `LED.set`メソッドを使用した RGB LED 色の設定
- RGB 値を指定するための配列の使用
- コードリロードが要求された場合のクリーンな終了の実装
- `break if Blink.req_reload?`文は OpenBlink アプリケーションにおいて非常に重要です。これにより、Bluetooth インターフェースを通じてコードのリロードが要求された際に、現在の実行ループを適切に終了させることができます。この確認がなければ、プログラムは実行を継続してリロード要求を無視するため、開発やデバッグが困難になります。このメカニズムが「Blink」機能を可能にしています - マイクロプロセッサを再起動せずに 0.1 秒未満でワイヤレスにコードを更新する能力です。
