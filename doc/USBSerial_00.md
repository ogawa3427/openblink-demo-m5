# USB Serial 使用例

## 基本的な使用法

以下は USBSerial クラスを使った基本的な通信の例です。

```ruby
# USB Serial/JTAGを初期化
if USBSerial.init
  puts "USB Serial初期化成功"
else
  puts "USB Serial初期化失敗"
  return
end

# データ送信
bytes = USBSerial.write("Hello USB Serial\r\n")
puts "送信バイト数: #{bytes}"

# 受信待機（1秒間）
sleep(1)

# データ受信（最大64バイト、タイムアウト500ms）
data = USBSerial.read(64, 500)
if data && !data.empty?
  puts "受信データ: #{data}"
else
  puts "データ受信なし"
end

# 使用終了時
USBSerial.deinit
```

## シリアルモニタリング例

受信バッファのデータをモニタリングし続ける例です。

```ruby
# USB Serial/JTAGを初期化
USBSerial.init

# 無限ループでシリアルモニタリング
while true
  # Blinkリロード要求をチェック
  if Blink.req_reload?
    USBSerial.deinit
    break
  end

  # 接続状態を確認
  unless USBSerial.connected?
    puts "USB Serial未接続"
    sleep(1)
    next
  end

  # 受信バッファにデータがあるか確認
  available = USBSerial.available
  if available > 0
    # データを読み込む
    data = USBSerial.read(available)
    if data && !data.empty?
      # 受信データを処理
      puts "受信: #{data}"

      # エコーバック
      USBSerial.write("Echo: #{data}")
    end
  end

  # 少し待機
  sleep(0.1)
end
```

## バイト配列送信の例

バイナリデータを送信する例です。

```ruby
# USB Serial/JTAGを初期化
USBSerial.init

# バイト配列でデータ送信
byte_data = [0x01, 0x02, 0x03, 0xFF, 0x00]
bytes_written = USBSerial.write(byte_data)
puts "送信バイト数: #{bytes_written}"

# 文字列とバイト配列の混合送信
USBSerial.write("Start: ")
USBSerial.write([0x41, 0x42, 0x43])  # "ABC"のASCIIコード
USBSerial.write(" :End\r\n")

USBSerial.deinit
```

## API リファレンス

### USBSerial.init()

USB Serial/JTAG を初期化します。

#### 引数

なし

#### 戻り値

- `true`: 初期化成功
- `false`: 初期化失敗

### USBSerial.write(data)

USB Serial/JTAG にデータを送信します。

#### 引数

- `data`: 送信するデータ（文字列またはバイト配列）

#### 戻り値

- 送信したバイト数（成功時）
- `-1`: エラー時

### USBSerial.read(length, [timeout_ms])

USB Serial/JTAG からデータを受信します。

#### 引数

- `length`: 読み込むバイト数の最大値
- `timeout_ms`: タイムアウト（ミリ秒、省略時は 100ms）

#### 戻り値

- 受信したデータ（文字列）
- 空文字列: データなし
- `nil`: エラー時

### USBSerial.available()

受信バッファの未読データ量を取得します。

#### 引数

なし

#### 戻り値

- 読み出し可能なバイト数

### USBSerial.connected?()

USB Serial/JTAG が接続されているかを確認します。

#### 引数

なし

#### 戻り値

- `true`: 接続中
- `false`: 未接続

### USBSerial.deinit()

USB Serial/JTAG を終了します。

#### 引数

なし

#### 戻り値

- `true`: 終了成功
- `false`: 終了失敗

## 注意事項

- USB Serial/JTAG は ESP32-S3 などの対応チップでのみ利用可能です
- 通常の UART とは異なり、ピンの指定は不要です
- USB ケーブルで PC と接続されている必要があります
- 初期化は一度だけ実行してください
- 使用後は必ず `deinit` を呼び出してリソースを解放してください
