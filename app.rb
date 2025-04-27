# UARTポート1の設定を追加
uart_port1 = 1 # UART1を使用
tx_pin1 = 1
rx_pin1 = 2
baud_rate1 = 31250 # MIDI標準ボーレート

# ディスプレイの初期化と設定
if Display.available?
    Display.clear()  # ディスプレイをクリア
    Display.set_text_size(1)  # テキストサイズを設定
    Display.set_text_color(Display.color565(255, 255, 255))  # 白色テキスト
    Display.println("UART Monitor & MIDI Demo") # タイトル変更
    Display.println("Port1(MIDI): Baud: #{baud_rate1}") # ポート1の情報を更新
    Display.println("TX: #{tx_pin1}, RX: #{rx_pin1}")
    Display.set_text_size(1)
    line_count = 5
  else
    line_count = 0
    puts "ディスプレイは利用できません"
end



# UARTポート1の初期化を追加
puts "UART1(MIDI)を初期化します: ポート=#{uart_port1}, TX=#{tx_pin1}, RX=#{rx_pin1}, ボーレート=#{baud_rate1}, 8N1"
sleep(0.1)
# MIDI標準の 8bit, Non-parity, 1 stop bit で初期化 (Arduinoコード参考)
if UART.init(uart_port1, tx_pin1, rx_pin1, baud_rate1, 128, 128, 1)
  LED.set([0x00, 0xFF, 0x00])
  Display.println("UART1(MIDI) init success (8N1)") # 設定情報を追加
  puts "UART1(MIDI)初期化成功 (8N1)" # 設定情報を追加
  sleep(0.3)
  LED.set([0,0,0])
else
  LED.set([0xFF, 0x00, 0x00])
  
  puts "UART1(MIDI)初期化失敗" # 表示を更新
  Display.println("UART1(MIDI) init failed") # 表示を更新
  sleep(1)
  LED.set([0,0,0])
end


max_lines = 15

# UART読み取りのタイムアウト設定（ミリ秒）
timeout_ms = 500

# MIDI送信関連
midi_channel = 0 # チャンネル1 (0-15)
midi_note = 60   # C4 (中央ド)
midi_velocity = 100 # ベロシティ (0-127)
# last_midi_send_time = Time.now # Timeクラスは使えないので削除
# midi_interval = 2 # 固定sleepにするので不要

# メインループ
while true
  # UARTからデータを受信

  
  # --- UARTポート1 MIDI送信処理 (sleepでタイミング調整) ---
  # current_time = Time.now # Timeクラスは使えないので削除
  # if current_time - last_midi_send_time >= midi_interval # Timeクラスは使えないので削除

  # 2秒待機 (元のmidi_intervalの値)
  sleep(2)

  # Note On: ステータスバイト 0x9n, ノート番号, ベロシティ
  # n = チャンネル番号 (0-15)
  note_on = [0x90 | midi_channel, midi_note, midi_velocity]
  # UART.write(uart_port1, note_on.pack('C*')) # packは使えない
  # UART.write(uart_port1, note_on.map(&:chr).join) # バイト文字列を自作
  UART.write(uart_port1, note_on.map { |b| b.chr }.join) # ブロック構文で書き換え
  puts "Port1(MIDI) 送信: Note On  (Ch=#{midi_channel+1}, Note=#{midi_note}, Vel=#{midi_velocity})"
  Display.println("P1 TX: NoteOn #{midi_note}")
  line_count += 1
  LED.set([0xFF, 0x00, 0xFF]) # Magenta
  sleep(0.05)
  LED.set([0,0,0])

  sleep(1) # ノートオンとノートオフの間隔

  # Note Off: ステータスバイト 0x8n, ノート番号, ベロシティ(0)
  # または Note On でベロシティ 0 でも可
  note_off = [0x80 | midi_channel, midi_note, 0]
  # UART.write(uart_port1, note_off.pack('C*')) # packは使えない
  # UART.write(uart_port1, note_off.map(&:chr).join) # バイト文字列を自作
  UART.write(uart_port1, note_off.map { |b| b.chr }.join) # ブロック構文で書き換え
  puts "Port1(MIDI) 送信: Note Off (Ch=#{midi_channel+1}, Note=#{midi_note})"
  # Display.println("P1 TX: NoteOff #{midi_note}") # オフは表示しない

  # last_midi_send_time = current_time # Timeクラスは使えないので削除

  # 次のノートへ (C4 と E4 を交互に)
  midi_note = (midi_note == 60) ? 64 : 60
  # end # if文削除に伴いendも削除

  # 少し待機
  sleep(0.05) # メインループの負荷軽減
  
  # Blinkリロード要求をチェック
  if Blink.req_reload?
    puts "リロード要求を検出"
    # UART.deinit(uart_port) # 未定義変数なので削除
    UART.deinit(uart_port1) # ポート1のdeinitを追加
    break
  end
end

# 終了処理
puts "UARTを終了します"
# UART.deinit(uart_port) # 未定義変数なので削除
UART.deinit(uart_port1) # ポート1のdeinitを追加
LED.set([0x00, 0xFF, 0x00])
sleep(0.4)
LED.set([0, 0, 0])
