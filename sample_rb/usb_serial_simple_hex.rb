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
sleep(1)

while true
  # リロード要求チェック
  break if Blink.req_reload?
  
  # データ受信
  if USBSerial.available > 0
    data = USBSerial.read(64, 500)
    
    if data && !data.empty?
      # 画面クリア
      Display.clear
      Display.set_cursor(0, 0)
      
      # データ長表示
      Display.puts "RX: #{data.length} bytes"
      
      # HEX表示（8バイトずつ改行）
      hex_line = ""
      data.each_byte.with_index do |b, i|
        hex_line += sprintf("%02X ", b)
        if (i + 1) % 8 == 0
          Display.puts hex_line
          hex_line = ""
        end
      end
      # 残りがあれば表示
      Display.puts hex_line unless hex_line.empty?
      
      # 簡易エコー
      USBSerial.write("OK\r\n")
    end
  end
  
  sleep(0.05)
end

USBSerial.deinit
Display.clear
Display.puts "TERMINATED"
