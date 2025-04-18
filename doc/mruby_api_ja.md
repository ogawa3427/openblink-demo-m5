# mruby API 仕様

---

## Input クラス

### pressed? メソッド

#### 引数

なし

#### 戻り値 (bool)

- true: ボタンが押されている
- false: 条件を満たしていない

#### コード例

```ruby
Input.pressed?()
```

### released? メソッド

#### 引数

なし

#### 戻り値 (bool)

- true: ボタンが離されている
- false: 条件を満たしていない

#### コード例

```ruby
Input.released?()
```

---

## LED クラス

### set メソッド

#### 引数

- `LED.set([r, g, b])` - RGB LED の色を設定します。各値は 0〜255 の間である必要があります。

#### 戻り値 (bool)

- true: 成功
- false: 失敗

#### コード例

```ruby
LED.set([255, 0, 0]) # 赤
LED.set([0, 255, 0]) # 緑
LED.set([0, 0, 255]) # 青
```

---

## PWM クラス

### setup メソッド

#### 引数

- `PWM.setup(pin_num, initial_duty)` - GPIO ピンを PWM 出力として設定します。
  - pin_num: GPIO ピン番号
  - initial_duty: 初期デューティ比（0-100%）

#### 戻り値 (int)

- 0-7: 成功時のチャンネル番号
- -1: 失敗

#### コード例

```ruby
channel = PWM.setup(15, 50) # GPIO 15を初期デューティ比50%で設定
```

### set_duty メソッド

#### 引数

- `PWM.set_duty(channel, duty)` - PWM 出力のデューティ比を設定します。
  - channel: setup で取得したチャンネル番号
  - duty: デューティ比（0-100%）

#### 戻り値 (bool)

- true: 成功
- false: 失敗

#### コード例

```ruby
PWM.set_duty(channel, 75) # デューティ比を75%に設定
```

### disable メソッド

#### 引数

- `PWM.disable(channel)` - PWM 出力を無効化します。
  - channel: setup で取得したチャンネル番号

#### 戻り値 (bool)

- true: 成功
- false: 失敗

#### コード例

```ruby
PWM.disable(channel) # PWM出力を無効化
```

---

## Blink クラス

### req_reload? メソッド

#### 引数

なし

#### 戻り値 (bool)

- true: リロード要求が存在する
- false: リロード要求なし

#### コード例

```ruby
while true
  return if Blink.req_reload?
  # メイン処理
end
```

### factory_reset! メソッド

- 処理が成功すると、再起動（ソフトリセット）を実行します。

#### 引数

なし

#### 戻り値 (bool)

- false: 失敗

#### コード例

```ruby
Blink.factory_reset!
```
