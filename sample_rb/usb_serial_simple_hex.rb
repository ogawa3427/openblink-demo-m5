# Simple USB Serial HEX Display
# シンプルなUSBシリアルHEX表示（M5Stack画面）

# 画面初期化
Display.clear
Display.set_text_size(1)
Display.puts "Simple USB HEX"
Display.puts "Initializing..."

# 初期化
USBSerial.init
Display.puts "Ready!"

while true
  # リロード要求チェック
  break if Blink.req_reload?
  
  # データ受信（短いタイムアウトで直接read）
  data = USBSerial.read(64, 10)  # 10msタイムアウト
    
  if data && !data.empty?
    # 画面クリア
    Display.clear
    Display.set_cursor(0, 0)
    
    # データ長表示
    Display.puts "RX: #{data.length} bytes"
    
    # HEX表示（8バイトずつ改行）
    hex_line = ""
    i = 0
    data.each_byte do |b|
      hex_line += sprintf("%02X ", b)
      if (i + 1) % 8 == 0
        Display.puts hex_line
        hex_line = ""
      end
      i += 1
    end
    # 残りがあれば表示
    Display.puts hex_line unless hex_line.empty?
    
    # 簡易エコー
    USBSerial.write("OK\r\n")
  end
  
  sleep(0.01)  # より短いスリープ
end

USBSerial.deinit
Display.clear
Display.puts "TERMINATED"
