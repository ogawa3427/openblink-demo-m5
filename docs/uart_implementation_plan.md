# UART 実装計画

## 概要

OpenBlink プロジェクトに UART 機能を追加し、Ruby コードから簡単にシリアル通信を利用できるようにする。
この実装はドライバ層の C 実装と、mruby/c バインディングの API 層の 2 つの部分から構成される。

## ファイル構成

以下のファイルを新規作成する：

1. `src/drv/uart.h` - UART ドライバのヘッダーファイル
2. `src/drv/uart.c` - UART ドライバの実装
3. `src/api/uart.h` - mruby/c バインディングのヘッダーファイル
4. `src/api/uart.c` - mruby/c バインディングの実装

また、以下のファイルを修正する：

5. `src/main.c` - UART API を mruby/c VM に登録するため

## ドライバ層 (C 言語)

### 機能

- UART の初期化 (`drv_uart_init`)
- データ送信 (`drv_uart_write`)
- データ受信 (`drv_uart_read`)
- UART ドライバのクリーンアップ (`drv_uart_deinit`)

### インターフェース

```c
// drv/uart.h
fn_t drv_uart_init(uart_port_t port, gpio_num_t txd_pin, gpio_num_t rxd_pin, uint32_t baud_rate);
fn_t drv_uart_write(uart_port_t port, const uint8_t* data, size_t len, size_t* written);
fn_t drv_uart_read(uart_port_t port, uint8_t* buffer, size_t len, uint32_t timeout_ms, size_t* read);
fn_t drv_uart_deinit(uart_port_t port);
```

## API 層 (mruby/c バインディング)

### Ruby 側インターフェース

```ruby
# 使用例
uart = UART.new(1, 4, 5, 115200)  # ポート番号、TXピン、RXピン、ボーレート
uart.write("Hello World!")        # データ送信
data = uart.read(10, 100)         # 最大10バイト読み込み、タイムアウト100ms
uart.close                        # 終了時にクリーンアップ
```

### C 側実装メソッド

- `c_uart_initialize` - コンストラクタ (UART の初期化)
- `c_uart_write` - データ送信メソッド
- `c_uart_read` - データ受信メソッド
- `c_uart_close` - UART リソース解放メソッド

### データ構造

```c
typedef struct {
  uint8_t port;
  bool initialized;
} mrb_uart_t;
```

## 実装順序

1. ドライバ層の実装 (`src/drv/uart.h` および `src/drv/uart.c`)
2. API 層の実装 (`src/api/uart.h` および `src/api/uart.c`)
3. `src/main.c` の修正
4. テスト用 Ruby スクリプトの作成

## エラー処理

- 内部で ESP-IDF のエラーを処理し、シンプルな成功/失敗（`kSuccess`/`kFailure`）を API に返す
- Ruby 側では失敗時に `false` または `nil` を返し、成功時に適切な値（送信バイト数や受信データなど）を返す

## 留意点

- ESP32 の UART 機能は最大 3 つのポート（UART0, UART1, UART2）をサポート
- UART0 は通常コンソール出力に使用されるため、アプリケーションでは UART1 または UART2 を使用すべき
- ピン設定はハードウェアに依存するため、適切なピンを選択する必要がある
- M5Stack 向けに実装するが、他の ESP32 ボードでも動作するようにする
