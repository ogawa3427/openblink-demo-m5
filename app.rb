# const int pwmFrontLeft  = 5;
# const int pwmFrontRight = 42;
# const int pwmRearLeft   = 10;
# const int pwmRearRight  = 41;

dut = 30

channel1 = PWM.setup(5, 0)
channel2 = PWM.setup(42, 0)
channel3 = PWM.setup(10, 0)
channel4 = PWM.setup(41, 0)

def set_duty(duty, channel1 = channel1, channel2 = channel2, channel3 = channel3, channel4 = channel4)
    PWM.set_duty(channel1, duty)
    PWM.set_duty(channel2, duty)
    PWM.set_duty(channel3, duty)
    PWM.set_duty(channel4, duty)
end

LED.set([0x0, 0xFF, 0x0])
set_duty(20, channel1, channel2, channel3, channel4)
sleep 0.2
set_duty(45, channel1, channel2, channel3, channel4)
sleep 0.5
set_duty(10, channel1, channel2, channel3, channel4)
sleep 0.2
set_duty(30, channel1, channel2, channel3, channel4)
sleep 0.5
set_duty(0, channel1, channel2, channel3, channel4)
sleep 0.5

while true
    # # 赤
    # LED.set([0x0, 0xFF, 0x0])
    # # PWM.set_duty(channel1, dut)
    # # PWM.set_duty(channel2, dut)
    # # PWM.set_duty(channel3, dut)
    # # PWM.set_duty(channel4, dut)
    # set_duty(4, channel1, channel2, channel3, channel4)
    # sleep 1
  
    # # 緑
    # LED.set([0xFF, 255, 0xFF])
    # set_duty(0, channel1, channel2, channel3, channel4)
    # sleep 1
  
    # # # 青
    # # LED.set([0, 0, 255])
    # # sleep 1
  
    # # リロードが要求されているか確認
    break if Blink.req_reload?
  end


  PWM.disable(channel1)
  PWM.disable(channel2)
  PWM.disable(channel3)
  PWM.disable(channel4)

