# USB Serial/JTAG 実装の知識メモ

## 実装内容

ESP32-S3 の USB Serial/JTAG 機能を Ruby から使えるようにした実装。

### 作成したファイル

1. **ドライバ層**

   - `src/drv/usb_serial.h` - ヘッダファイル
   - `src/drv/usb_serial.c` - ESP-IDF の API を使った実装

2. **API 層**

   - `src/api/usb_serial.h` - mruby/c 用ヘッダ
   - `src/api/usb_serial.c` - Ruby クラス実装

3. **ドキュメント**
   - `doc/USBSerial_00.md` - 使用例と API 仕様
   - `docs/usb_serial_implementation.md` - 実装詳細

### 統合作業

- `src/main.c`に include と api_usb_serial_define()呼び出しを追加

## 技術的なポイント

### ESP-IDF の USB Serial/JTAG API

使用した API:

- `usb_serial_jtag_driver_install()` - 初期化
- `usb_serial_jtag_write_bytes()` - 送信
- `usb_serial_jtag_read_bytes()` - 受信
- `usb_serial_jtag_driver_uninstall()` - 終了処理

### 制限事項と対処法

1. **available()の制限**

   - ESP-IDF には直接的な「利用可能バイト数取得」API がない
   - 0 バイト読み込みを試してタイムアウトで判定する簡易実装

2. **connected?()の制限**
   - 接続状態を直接確認する API がない
   - 初期化済みかどうかで判定

### UART との違い

- ピン指定が不要（USB 固定）
- ポート番号指定が不要（USB Serial/JTAG は 1 つだけ）
- 物理的な USB ケーブル接続が必要

## Ruby API 仕様

```ruby
USBSerial.init()                    # 初期化
USBSerial.write("data")            # 文字列送信
USBSerial.write([0x01, 0x02])      # バイト配列送信
data = USBSerial.read(64, 1000)    # 受信
available = USBSerial.available()  # バッファ確認
connected = USBSerial.connected?() # 接続確認
USBSerial.deinit()                 # 終了処理
```

## 動作確認方法

1. ESP32-S3 搭載の M5Stack 製品で動作
2. USB ケーブルで PC と接続
3. シリアルターミナルソフトで通信確認

## 参考にした実装

- `src/api/uart.c` - UART 実装をベースに作成
- `src/drv/uart.c` - ドライバ層の構造を参考

## メモ

ESP32 の USB Serial/JTAG 機能は比較的新しい機能で、ESP32-S3 などの対応チップでのみ利用可能。通常の UART よりもシンプルで、デバッグやプログラミング用途に最適化されている。
