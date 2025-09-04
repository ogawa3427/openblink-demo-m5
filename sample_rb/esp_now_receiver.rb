# ESP-NOW 受信側サンプル
# Usage: M5Stack Atom/StampS3で受信データを表示

puts "ESP-NOW Receiver starting..."

# ESP-NOW初期化（受信モード）
unless ESPNow.init(:recv)
  puts "ESP-NOW initialization failed!"
  exit
end

puts "ESP-NOW Receiver initialized successfully"
puts "Waiting for data..."

last_received = -1

loop do
  # 受信カウンターをチェック
  current_received = ESPNow.received_counter
  
  if current_received != last_received
    puts "Received: #{current_received}"
    LED.set([0, 0, 255])  # 青色で受信を表示
    last_received = current_received
    
    # 受信データに応じてLEDの色を変化
    case current_received % 3
    when 0
      LED.set([255, 0, 0])  # 赤
    when 1
      LED.set([0, 255, 0])  # 緑
    when 2
      LED.set([0, 0, 255])  # 青
    end
    
    sleep 0.5  # LEDを少し点灯させる
  else
    # 受信していない時は薄暗い青
    LED.set([0, 0, 32])
  end
  
  # ステータス確認
  status = ESPNow.status
  if status == :error
    puts "ESP-NOW error detected!"
    LED.set([255, 255, 0])  # 黄色でエラー状態を表示
  end
  
  sleep 0.1
end 