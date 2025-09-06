# USB Serial/JTAG 実装詳細

## 概要

ESP32-S3 などの対応チップで利用可能な USB Serial/JTAG 機能を mruby/c から使用するための実装です。

## 実装構造

### ドライバ層 (src/drv/)

- `usb_serial.h` - ヘッダファイル
- `usb_serial.c` - ESP-IDF の USB Serial/JTAG API を使用したドライバ実装

### API 層 (src/api/)

- `usb_serial.h` - mruby/c 用ヘッダファイル
- `usb_serial.c` - mruby/c の USBSerial クラス実装

## 主要機能

### 初期化・終了処理

- `drv_usb_serial_init()` - USB Serial/JTAG ドライバの初期化
- `drv_usb_serial_deinit()` - USB Serial/JTAG ドライバの終了処理

### データ通信

- `drv_usb_serial_write()` - データ送信
- `drv_usb_serial_read()` - データ受信

### ステータス確認

- `drv_usb_serial_get_available()` - 受信バッファの利用可能バイト数取得
- `drv_usb_serial_is_connected()` - 接続状態確認

## Ruby API

### クラス: USBSerial

#### メソッド一覧

- `USBSerial.init()` - 初期化
- `USBSerial.write(data)` - データ送信
- `USBSerial.read(length, [timeout_ms])` - データ受信
- `USBSerial.available()` - 受信バッファサイズ取得
- `USBSerial.connected?()` - 接続状態確認
- `USBSerial.deinit()` - 終了処理

## 制限事項

1. **ハードウェア制限**

   - ESP32-S3 など USB Serial/JTAG 対応チップでのみ利用可能
   - USB ケーブルでの物理接続が必要

2. **実装上の制限**
   - `available()` メソッドは簡易実装（完全な利用可能バイト数は取得できない）
   - `connected?()` メソッドは初期化状態のみを確認

## 使用例

```ruby
# 初期化
USBSerial.init

# データ送信
USBSerial.write("Hello World\n")

# データ受信
data = USBSerial.read(64, 1000)
puts data if data

# 終了
USBSerial.deinit
```

## 参考資料

- [ESP-IDF USB Serial/JTAG API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/usb_serial_jtag.html)
- 既存の UART 実装 (`src/api/uart.c`, `src/drv/uart.c`)
