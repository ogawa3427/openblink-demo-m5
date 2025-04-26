channel1 = PWM.setup(5, 0)
if channel1 < 0
  puts "PWM setup failed for GPIO 5"
end

channel2 = PWM.setup(42, 0)
if channel2 < 0
  puts "PWM setup failed for GPIO 42"
end

channel3 = PWM.setup(10, 0)
if channel3 < 0
  puts "PWM setup failed for GPIO 10"
end

channel4 = PWM.setup(41, 0)
if channel4 < 0
  puts "PWM setup failed for GPIO 41"
end

pwmChannel = [channel1, channel2, channel3, channel4]
i = 0

while true
  LED.set([0x10, 0x00, 0x20])
  sleep(0.1)
  if channel1 >= 0 && channel2 >= 0 && channel3 >= 0 && channel4 >= 0
        LED.set([0x0, 0x0, 0xFF])
        # PWM出力のデューティ比を変更
        PWM.set_duty(pwmChannel[2], 3)  # 75%に設定
        
        # 1秒待
        sleep(0.1)
        
        PWM.set_duty(pwmChannel[2], 0)
        
        LED.set([0, 0, 0])
        sleep(0.1)
        
        i = (i + 1) % 3  # iを0から2の範囲に制限
  else
    LED.set([0xFF, 0x00, 0x00])
    sleep(0.2)
    puts "PWM setup failed"
  end
  LED.set([0, 0, 0])
  sleep(0.1)
  break if Blink.req_reload?
end

channel1 = PWM.disable(channel1)
channel2 = PWM.disable(channel2)
channel3 = PWM.disable(channel3)
channel4 = PWM.disable(channel4)
LED.set([0x00, 0xFF, 0x00])
sleep(0.4)
LED.set([0, 0, 0])
