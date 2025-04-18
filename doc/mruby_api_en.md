# mruby API Specification

---

## Input Class

### pressed? Method

#### Arguments

None

#### Return Value (bool)

- true: Button is pressed
- false: Condition not met

#### Code Example

```ruby
Input.pressed?()
```

### released? Method

#### Arguments

None

#### Return Value (bool)

- true: Button is released
- false: Condition not met

#### Code Example

```ruby
Input.released?()
```

---

## LED Class

### set Method

#### Arguments

- `LED.set([r, g, b])` - Sets the RGB LED color. Each value should be between 0-255.

#### Return Value (bool)

- true: Success
- false: Failure

#### Code Example

```ruby
LED.set([255, 0, 0]) # Red
LED.set([0, 255, 0]) # Green
LED.set([0, 0, 255]) # Blue
```

---

## PWM Class

### setup Method

#### Arguments

- `PWM.setup(pin_num, initial_duty)` - Sets up a GPIO pin as PWM output.
  - pin_num: GPIO pin number
  - initial_duty: Initial duty cycle (0-100%)

#### Return Value (int)

- 0-7: Channel number on success
- -1: Failure

#### Code Example

```ruby
channel = PWM.setup(15, 50) # Set GPIO 15 with initial duty cycle 50%
```

### set_duty Method

#### Arguments

- `PWM.set_duty(channel, duty)` - Sets the duty cycle of the PWM output.
  - channel: Channel number from setup
  - duty: Duty cycle (0-100%)

#### Return Value (bool)

- true: Success
- false: Failure

#### Code Example

```ruby
PWM.set_duty(channel, 75) # Set duty cycle to 75%
```

### disable Method

#### Arguments

- `PWM.disable(channel)` - Disables the PWM output.
  - channel: Channel number from setup

#### Return Value (bool)

- true: Success
- false: Failure

#### Code Example

```ruby
PWM.disable(channel) # Disable PWM output
```

---

## Blink Class

### req_reload? Method

#### Arguments

None

#### Return Value (bool)

- true: Reload request exists
- false: No reload request

#### Code Example

```ruby
while true
  return if Blink.req_reload?
  # Main processing
end
```

### factory_reset! Method

- Upon successful processing, a restart (soft reset) will be executed.

#### Arguments

None

#### Return Value (bool)

- false: Fail

#### Code Example

```ruby
Blink.factory_reset!
```
