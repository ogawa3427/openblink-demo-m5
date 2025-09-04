# ESP-NOW 送信側サンプル
# Usage: M5Stack Atom/StampS3でボタンを押すとカウンターを送信

puts "ESP-NOW Sender starting..."

# ESP-NOW初期化（送信モード）
unless ESPNow.init(:send)
  puts "ESP-NOW initialization failed!"
  exit
end

# 受信側のMACアドレスを設定（実際の環境に合わせて変更してください）
# ブロードキャストの場合
unless ESPNow.add_peer("FF:FF:FF:FF:FF:FF")
  puts "Failed to add peer!"
  exit
end

puts "ESP-NOW Sender initialized successfully"
puts "Press button to send data..."

counter = 0

loop do
  # ボタンが押されたら送信
  if Input.pressed?
    if ESPNow.send(counter)
      puts "Sent counter: #{counter}"
      LED.set([0, 255, 0])  # 緑色で送信成功を表示
      counter += 1
    else
      puts "Send failed!"
      LED.set([255, 0, 0])  # 赤色でエラーを表示
    end
    
    # ボタンの連続押しを防ぐ
    sleep 0.3
  else
    # LEDを消す
    LED.set([0, 0, 0])
  end
  
  # ステータス確認
  status = ESPNow.status
  if status == :error
    puts "ESP-NOW error detected!"
    LED.set([255, 255, 0])  # 黄色でエラー状態を表示
  end
  
  sleep 0.1
end 