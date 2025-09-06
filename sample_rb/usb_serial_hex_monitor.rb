# USB Serial HEX Monitor Sample
# USBシリアルから受信したデータをHEX形式でM5Stack画面に表示

# 画面初期化
Display.clear
Display.set_text_size(1)
Display.set_text_color(0xFFFF)  # 白色
Display.puts "USB Serial HEX Monitor"
Display.puts "Initializing..."

# USB Serial初期化
unless USBSerial.init
  Display.puts "USB Serial init FAILED"
  return
end

Display.puts "USB Serial OK"
Display.puts "Send data via USB..."
Display.puts "---"

# HEX変換用のヘルパー関数
def to_hex_string(data)
  return "" if data.nil? || data.empty?
  
  hex_str = ""
  data.each_byte do |byte|
    hex_str += sprintf("%02X ", byte)
  end
  hex_str.strip
end

# ASCII表示用のヘルパー関数（印刷可能文字のみ）
def to_ascii_string(data)
  return "" if data.nil? || data.empty?
  
  ascii_str = ""
  data.each_byte do |byte|
    if byte >= 32 && byte <= 126  # 印刷可能ASCII文字
      ascii_str += byte.chr
    else
      ascii_str += "."
    end
  end
  ascii_str
end

# メインループ
loop_count = 0
while true
  # Blinkリロード要求をチェック
  if Blink.req_reload?
    Display.clear
    Display.puts "RELOAD REQUESTED"
    break
  end

  # 接続状態を確認（定期的に）
  if loop_count % 50 == 0  # 5秒ごとに確認
    unless USBSerial.connected?
      Display.set_cursor(0, 120)
      Display.print "DISCONNECTED"
    end
  end

  # 受信データをチェック
  available = USBSerial.available
  if available > 0
    # データを読み込む（最大128バイト）
    read_size = available > 128 ? 128 : available
    data = USBSerial.read(read_size, 100)
    
    if data && !data.empty?
      # 画面をクリアして新しいデータを表示
      Display.clear
      Display.set_cursor(0, 0)
      Display.puts "RX: #{data.length} bytes"
      
      # HEX表示（16バイトずつ改行）
      hex_data = to_hex_string(data)
      Display.puts "HEX:"
      
      # 16バイトずつに分割して表示
      hex_bytes = hex_data.split(' ')
      line = ""
      hex_bytes.each_with_index do |hex_byte, i|
        line += hex_byte + " "
        if (i + 1) % 8 == 0
          line += " "  # 8バイトごとにスペース追加
        end
        if (i + 1) % 16 == 0 || i == hex_bytes.length - 1
          Display.puts line
          line = ""
        end
      end
      
      # ASCII表示
      ascii_data = to_ascii_string(data)
      Display.puts ""
      Display.puts "ASCII:"
      Display.puts "[#{ascii_data}]"
      
      # エコーバック（HEX形式で）
      echo_msg = "Echo(HEX): #{hex_data}\r\n"
      USBSerial.write(echo_msg)
    end
  end

  loop_count += 1
  sleep(0.1)  # 100ms待機
end

# 終了処理
USBSerial.deinit
Display.clear
Display.puts "USB Serial Monitor"
Display.puts "TERMINATED"
