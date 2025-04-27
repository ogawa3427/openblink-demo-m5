# UART初期化（ピン設定を修正）
uart_port = 2  # UART1を使用
tx_pin = 6
rx_pin = 5
baud_rate = 115200

# ディスプレイの初期化と設定
if Display.available?
    Display.clear()  # ディスプレイをクリア
    Display.set_text_size(1)  # テキストサイズを設定
    Display.set_text_color(Display.color565(255, 255, 255))  # 白色テキスト
    Display.println("UART Monitor")
    Display.println("Baud: 115200")
    Display.println("TX: #{tx_pin}, RX: #{rx_pin}")
    Display.set_text_size(1)  # 小さめのテキストサイズに変更
    line_count = 3
  else
    line_count = 0
    puts "ディスプレイは利用できません"
end


puts "UARTを初期化します: ポート=#{uart_port}, TX=#{tx_pin}, RX=#{rx_pin}, ボーレート=#{baud_rate}"

# 初期化前にLEDを黄色に設定
LED.set([0xFF, 0xFF, 0x00])
sleep(0.3)

if UART.init(uart_port, tx_pin, rx_pin, baud_rate)
  LED.set([0x00, 0xFF, 0x00])  # 緑色でUART初期化成功を表示
  Display.println("UART init success")
  puts "UART初期化成功"
else
  LED.set([0xFF, 0x00, 0x00])  # 赤色でUART初期化失敗を表示
  puts "UART初期化失敗"
  sleep(1)
  # 失敗した場合は別のピン設定を試す
  puts "別のピン設定を試みます..."
  tx_pin = 15  # 別のTXピン
  rx_pin = 16  # 別のRXピン
  if UART.init(uart_port, tx_pin, rx_pin, baud_rate)
    LED.set([0x00, 0xFF, 0x00])
    puts "UART初期化成功 (TX=#{tx_pin}, RX=#{rx_pin})"
  else
    puts "UART初期化失敗 (TX=#{tx_pin}, RX=#{rx_pin})"
    LED.set([0xFF, 0x00, 0x00])
    # エラーを表示して一時停止
    sleep(2)
  end
  Display.println("UART init failed")
end


max_lines = 15  # 画面に表示する最大行数

# UART読み取りのタイムアウト設定（ミリ秒）
timeout_ms = 500

# # テスト送信
# test_message = "OpenBlink UART Test\r\n"
# bytes_sent = UART.write(uart_port, test_message)
# puts "テストメッセージ送信: #{bytes_sent}バイト"

# メインループ
while true
  # UARTからデータを受信
  available = UART.available(uart_port)
  puts "available: #{available}"
  Display.println("available: #{available}")
  if available > 0
    puts "受信データあり: #{available}バイト"
    
    # データを読み込む（タイムアウト付き）
    data = UART.read(uart_port, available, timeout_ms)
    if data && data.length > 0
      # データをHEX形式に変換
      hex_data = ""
      data.each_byte do |b|
        hex_data += sprintf("%02X ", b)
      end
      
      puts "受信: #{hex_data}"  # コンソールに出力
      puts "生データ: #{data.inspect}"  # デバッグ用
      
      # ディスプレイに表示
      if Display.available?
        # 行数が最大値を超えたらクリア
        if line_count >= max_lines
          Display.clear()
          Display.set_cursor(0, 0)
          Display.set_text_size(2)
          Display.println("UART Monitor")
          Display.println("Baud: 115200")
          Display.println("TX: #{tx_pin}, RX: #{rx_pin}")
          Display.set_text_size(1)
          line_count = 3
        end
        
        Display.println(hex_data)  # 受信データをHEX形式で表示
        puts "受信データ: #{hex_data}"
        line_count += 1
        
        # 受信時にLEDを点滅
        LED.set([0x00, 0x00, 0xFF])  # 青色で受信表示
        sleep(0.05)
        LED.set([0, 0, 0])
      end
    else
      puts "データ読み取り失敗またはデータなし"
    end
  end
  
  # 少し待機
  sleep(1)
  
  # Blinkリロード要求をチェック
  if Blink.req_reload?
    puts "リロード要求を検出"
    UART.deinit(uart_port)
    break
  end
end

# 終了処理
puts "UARTを終了します"
UART.deinit(uart_port)
LED.set([0x00, 0xFF, 0x00])
sleep(0.4)
LED.set([0, 0, 0])
