# UART 使用例

## 基本的な使用法

以下は UART クラスを使った基本的な通信の例です。

```ruby
# ピン設定はデバイスボードによって異なります
# UART1をTX=17, RX=16で初期化（ATOM Matrix/Liteの場合）
if UART.init(1, 17, 16, 9600)
  puts "UART1初期化成功"
else
  puts "UART1初期化失敗"
  return
end

# データ送信
bytes = UART.write(1, "Hello UART\r\n")
puts "送信バイト数: #{bytes}"

# 受信待機（1秒間）
sleep(1)

# データ受信（最大64バイト、タイムアウト500ms）
data = UART.read(1, 64, 500)
if data
  puts "受信データ: #{data}"
else
  puts "データ受信なし"
end

# 使用終了時
UART.deinit(1)
```

## シリアルモニタリング例

受信バッファのデータをモニタリングし続ける例です。

```ruby
# UART2をTX=21, RX=22で初期化（ATOM Matrix/Liteの場合）
UART.init(2, 21, 22, 115200)

# 無限ループでシリアルモニタリング
while true
  # Blinkリロード要求をチェック
  if Blink.req_reload?
    UART.deinit(2)
    break
  end

  # 受信バッファにデータがあるか確認
  available = UART.available(2)
  if available > 0
    # データを読み込む
    data = UART.read(2, available)
    if data
      # 受信データを処理
      puts "受信: #{data}"

      # エコーバック
      UART.write(2, "Echo: #{data}")
    end
  end

  # 少し待機
  sleep(0.1)
end
```

## 2 つの UART 通信の例

UART1 と 2 を同時に使用する例です。

```ruby
# UART1（センサー接続用）
UART.init(1, 17, 16, 9600)

# UART2（PC通信用）
UART.init(2, 21, 22, 115200)

# センサーにコマンド送信
UART.write(1, [0xA1, 0x02, 0x03])

while true
  if Blink.req_reload?
    UART.deinit(1)
    UART.deinit(2)
    break
  end

  # センサーからのデータ確認
  if UART.available(1) > 0
    sensor_data = UART.read(1, 64)
    if sensor_data
      # データを処理

      # PCに送信
      UART.write(2, "Sensor data: #{sensor_data.unpack('H*')}")
    end
  end

  # PCからのコマンド確認
  if UART.available(2) > 0
    cmd = UART.read(2, 64)
    if cmd
      case cmd.strip
      when "GET_TEMP"
        # センサーに温度取得コマンド送信
        UART.write(1, [0xA1, 0x05])
      when "GET_HUMID"
        # センサーに湿度取得コマンド送信
        UART.write(1, [0xA1, 0x06])
      end
    end
  end

  sleep(0.05)
end
```

## 注意点

1. UART0 はコンソール出力に使用されているため、UART1 と UART2 のみが利用可能です。
2. ピン番号はボードによって異なります。M5Stack ATOM シリーズでは一般的に以下のピンが利用可能です：
   - UART1: TX=17, RX=16
   - UART2: TX=21, RX=22
3. 必要なくなった場合は必ず`deinit`メソッドで UART を終了してください。
4. バッファサイズには制限があります。大量のデータを受信する場合は、複数回に分けて読み込むことをお勧めします。
