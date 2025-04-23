# UARTサンプルコード
# M5Stack StampS3でUARTを使ってシリアル通信をするサンプル

# UART1を初期化 (TX: GPIO4, RX: GPIO5, 115200bps)
uart = UART.new(1, 4, 5, 115200)

# 起動メッセージを送信
uart.write("Hello from M5Stack StampS3!\r\n")

# データ受信ループ
10.times do |i|
  # プロンプトを送信
  uart.write("UART Test #{i+1}> ")
  
  # データを受信 (最大100バイト、タイムアウト500ms)
  data = uart.read(100, 500)
  
  if data
    # 受信データがあれば、エコーバックする
    uart.write("Echo: #{data}\r\n")
  else
    # タイムアウトしたら通知
    uart.write("No data received (timeout)\r\n")
  end
  
  # 少し待つ
  sleep(1)
end

# 終了メッセージ
uart.write("UART test completed.\r\n")

# UARTをクローズ
uart.close 