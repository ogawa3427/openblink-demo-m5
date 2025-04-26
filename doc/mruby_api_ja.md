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

## UART クラス

UART クラスは ESP32 の UART 通信を制御します。UART1 と UART2 のみ利用可能です（UART0 はすでにコンソール出力に使用されています）。

### init メソッド

#### 引数

- `UART.init(port_num, tx_pin, rx_pin, [baud_rate])` - UART を初期化します。
  - port_num: UART ポート番号 (1 または 2 のみ)
  - tx_pin: 送信ピン番号
  - rx_pin: 受信ピン番号
  - baud_rate: 通信速度（省略時は 115200）

#### 戻り値 (bool)

- true: 成功
- false: 失敗

#### コード例

```ruby
UART.init(1, 17, 16)            # UART1をTX=17, RX=16, ボーレート115200で初期化
UART.init(2, 21, 22, 9600)      # UART2をTX=21, RX=22, ボーレート9600で初期化
```

### write メソッド

#### 引数

- `UART.write(port_num, data)` - 指定した UART ポートからデータを送信します。
  - port_num: UART ポート番号 (1 または 2)
  - data: 送信するデータ（文字列またはバイト配列）

#### 戻り値 (int)

- 送信したバイト数、エラー時は-1

#### コード例

```ruby
UART.write(1, "Hello World")     # 文字列送信
UART.write(1, [0x01, 0x02, 0x03]) # バイト配列送信
```

### read メソッド

#### 引数

- `UART.read(port_num, length, [timeout_ms])` - 指定した UART ポートからデータを受信します。
  - port_num: UART ポート番号 (1 または 2)
  - length: 読み込むバイト数の最大値
  - timeout_ms: タイムアウト（ミリ秒、省略時は 100ms）

#### 戻り値 (string)

- 受信したデータ（文字列）、データがない場合やエラー時は nil

#### コード例

```ruby
data = UART.read(1, 64)         # 最大64バイト読み込み（タイムアウト100ms）
data = UART.read(1, 128, 1000)  # 最大128バイト読み込み（タイムアウト1秒）
```

### available メソッド

#### 引数

- `UART.available(port_num)` - 受信バッファの未読データ量を取得します。
  - port_num: UART ポート番号 (1 または 2)

#### 戻り値 (int)

- 読み出し可能なバイト数、エラー時は-1

#### コード例

```ruby
bytes = UART.available(1)   # UART1の受信バッファにあるバイト数を取得
if bytes > 0
  data = UART.read(1, bytes)
end
```

### deinit メソッド

#### 引数

- `UART.deinit(port_num)` - UART を終了します。
  - port_num: UART ポート番号 (1 または 2)

#### 戻り値 (bool)

- true: 成功
- false: 失敗

#### コード例

```ruby
UART.deinit(1)   # UART1を終了
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
