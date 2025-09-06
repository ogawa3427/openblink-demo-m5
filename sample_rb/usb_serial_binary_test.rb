# USB Serial Binary Data Test
# バイナリデータの送受信テスト（M5Stack画面表示）

# 画面初期化
Display.clear
Display.set_text_size(1)
Display.puts "USB Binary Test"
Display.puts "Initializing..."

# 初期化
unless USBSerial.init
  Display.puts "Init FAILED"
  return
end

Display.puts "Init OK"

# テストデータ送信
test_data = [
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05,  # 連続データ
  0xFF, 0xFE, 0xFD, 0xFC,              # 大きな値
  0x0A, 0x0D,                          # 改行文字
  0x20, 0x21, 0x7E, 0x7F               # ASCII境界
]

Display.puts "Sending test data..."
USBSerial.write("=== Binary Test Start ===\r\n")
USBSerial.write(test_data)
USBSerial.write("\r\n=== Binary Test End ===\r\n")

# 受信ループ
Display.puts "Waiting for RX (10s)..."
start_time = Time.now.to_i

while (Time.now.to_i - start_time) < 10
  break if Blink.req_reload?
  
  if USBSerial.available > 0
    data = USBSerial.read(32, 200)
    
    if data && !data.empty?
      # 画面に詳細表示
      Display.clear
      Display.set_cursor(0, 0)
      Display.puts "RX: #{data.length} bytes"
      
      # HEX表示（8バイトずつ）
      hex_line = ""
      data.each_byte.with_index do |byte, i|
        hex_line += sprintf("%02X ", byte)
        if (i + 1) % 8 == 0
          Display.puts hex_line
          hex_line = ""
        end
      end
      Display.puts hex_line unless hex_line.empty?
      
      # ASCII表示
      ascii_line = ""
      data.each_byte do |byte|
        if byte >= 32 && byte <= 126
          ascii_line += byte.chr
        else
          ascii_line += "."
        end
      end
      Display.puts "ASCII: #{ascii_line}"
      
      sleep(2)  # 2秒表示
    end
  end
  
  sleep(0.1)
end

# 定期送信テスト
Display.clear
Display.puts "Periodic TX Test (5x)"
5.times do |i|
  break if Blink.req_reload?
  
  # カウンター付きデータ送信
  counter_data = [0xAA, 0xBB, i, 0xCC, 0xDD]
  USBSerial.write(counter_data)
  USBSerial.write("\r\n")
  
  Display.set_cursor(0, 20 + i * 10)
  hex_str = counter_data.map{|b| sprintf('%02X', b)}.join(' ')
  Display.puts "TX##{i + 1}: #{hex_str}"
  sleep(1)
end

USBSerial.deinit
Display.clear
Display.puts "Test Complete"
