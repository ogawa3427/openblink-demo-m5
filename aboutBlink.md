新たな API を追加するには、既存の機能と同様のパターンに従う必要があります。まず関連するファイルを調べてみましょう。

Ran tool
既存のクラスの実装パターンを確認するために、例えば c_canvas.h と c_canvas.cpp を見てみましょう。

Read file: src/m5u/c_canvas.h

Read file: src/m5u/c_canvas.cpp
my_mrubydef.h の内容も確認しましょう:

Read file: src/m5u/my_mrubydef.h
新しい API を追加する方法が分かりました。以下の手順で実装できます：

1. 新しい機能のためのヘッダーと CPP ファイルを作成
2. c_m5u.cpp の`init_c_m5u()`関数に新しいクラスの初期化関数を追加

以下は、新しい機能（例：センサー）を追加する方法の例です：

1. まず、`c_sensor.h`を作成：

```cpp
extern "C" {
    void class_sensor_init();
}
```

2. 次に、`c_sensor.cpp`を作成：

```cpp
#include <M5Unified.h>

#include "my_mrubydef.h"

#ifdef USE_SENSOR
#include "c_sensor.h"

static mrbc_class *sensor_class;

// センサークラスの実装
static void c_sensor_new(mrb_vm *vm, mrb_value *v, int argc) {
  v[0] = mrbc_instance_new(vm, v[0].cls, sizeof(void*));
  mrbc_instance_call_initialize(vm, v, argc);
}

static void c_sensor_initialize(mrb_vm *vm, mrb_value *v, int argc) {
  // センサー初期化のコード
}

// センサー関連の関数実装
static void c_sensor_read_temperature(mrb_vm *vm, mrb_value *v, int argc) {
  // 温度読み取りの実装
  float temp = 25.5; // 仮の値
  SET_FLOAT_RETURN(temp);
}

// クラス初期化関数
void class_sensor_init() {
  // センサークラスの定義
  sensor_class = mrbc_define_class(0, "Sensor", mrbc_class_object);
  mrbc_define_method(0, sensor_class, "new", c_sensor_new);
  mrbc_define_method(0, sensor_class, "initialize", c_sensor_initialize);
  mrbc_define_method(0, sensor_class, "temperature", c_sensor_read_temperature);
  // 他のメソッドを追加
}

#endif // USE_SENSOR
```

3. my_mrubydef.h に新しい機能のための定義を追加：

```cpp
#define USE_SENSOR           // to support Sensor functions
```

4. c_m5u.h に extern 宣言を追加：

```cpp
extern "C" {
    void init_c_m5u();
}
```

5. c_m5u.cpp を修正して新しいクラスを追加：

```cpp
#include "c_m5u.h"

#include <M5Unified.h>

#include "c_canvas.h"
#include "c_display_button.h"
#include "c_font.h"
#include "c_m5.h"
#include "c_sensor.h"  // 新しいヘッダーを追加
#include "c_speaker.h"
#include "c_touch.h"
#include "c_utils.h"
#include "my_mrubydef.h"

void init_c_m5u() {
  class_m5_init();
  class_display_button_init();
  class_utils_init();
#ifdef USE_TOUCH
  class_touch_init();
#endif
#ifdef USE_CANVAS
  class_canvas_init();
#endif
#ifdef USE_SPEAKER
  class_speaker_init();
#endif
#ifdef USE_FONT
  class_font_init();
//  c_font_add("JapanGothic_8", &lgfxJapanGothic_8);
#endif
#ifdef USE_SENSOR
  class_sensor_init();  // 新しいクラスの初期化を追加
#endif
}
```

これで、mruby スクリプトから新しい機能（この例ではセンサー）を利用できます：

```ruby
# mrubyでの使用例
sensor = Sensor.new
temp = sensor.temperature
puts "温度: #{temp}℃"
```

新しい機能を追加するときは、既存のクラスパターンに従い、適切な条件分岐（#ifdef）で囲むことで、機能を選択的に有効化できます。

このファームウェアは、M5Stack 用の mruby ベースのフレームワークのようです。構造を理解するために、まずコードベースを探索しましょう。

Ran tool
メイン構造を確認するために、main.c ファイルを読んでみましょう：

Read file: src/main.c
M5Stack の実装部分を見てみましょう：

Read file: src/m5u/c_m5.h
主要なファームウェアの構造を理解するためにさらに探索しましょう：

Read file: src/m5u/c_m5.cpp
mruby のコードも確認してみましょう：

Ran tool

Read file: src/rb/slot1.rb

Read file: src/rb/slot2.rb
システムの構造を理解するためにもう少し探索してみましょう：

Ran tool

Read file: src/api/led.h
基本的なシステム構造が把握できました。以下にこのファームウェアの構造と仕組みを説明します：

このファームウェアは、M5Stack 向けの mruby/c ベースのプログラミング環境「OpenBlink」です。主な特徴と構造は以下の通りです：

1. **基本構造**：

   - mruby/c VM を使用して、Ruby のコードを実行する環境を提供しています
   - M5Unified ライブラリを使って M5Stack ハードウェアを制御しています
   - C/C++で書かれたネイティブ機能を Ruby から利用できるように橋渡しする設計です

2. **主要コンポーネント**：

   - `main.c`: アプリケーションのエントリーポイント。mruby/c VM の初期化と実行を担当
   - `src/m5u/`: M5Stack の機能（ディスプレイ、ボタン、タッチ、スピーカーなど）を Ruby から利用するためのバインディング
   - `src/api/`: LED、入力、Blink などの基本 API の定義
   - `src/rb/`: スロット 1、スロット 2 などの Ruby コード
   - `src/app/`: アプリケーション初期化などの機能
   - `src/drv/`: デバイスドライバ層（BLE など）

3. **mruby/c の活用**：

   - コード実行にはスロットシステムを採用（slot1.rb、slot2.rb）
   - Ruby コードがバイトコードにコンパイルされて実行されます
   - エラー時には特別なエラースロット（slot_err）が読み込まれます

4. **API の設計**：

   - Ruby クラスとして各機能を公開（例：M5、LED、Input、Blink など）
   - C/C++の機能を Ruby メソッドとして呼び出せるようにブリッジしています

5. **機能の条件付きコンパイル**：

   - タッチ、キャンバス、スピーカー、フォントなどの機能は条件付きコンパイル（`#ifdef USE_XXX`）で制御されています

6. **サンプルアプリケーション**：
   - LED を制御する基本的なサンプルコードが含まれています（slot2.rb）

このフレームワークの主な利点は、M5Stack のハードウェア機能を Ruby の簡潔な構文で利用できることです。これにより、IoT やマイコンプログラミングの敷居を下げ、より多くの開発者が簡単に M5Stack を活用できるようになっています。

モーター制御機能を OpenBlink に組み込むための手順を説明します。

Ran tool

Read file: src/flight_control.hpp

Ran tool

Ran tool
モーター制御機能を OpenBlink に組み込む手順を説明します。以下のステップに従ってください：

1. まず、モーター制御用のヘッダーファイルを作成します：

```cpp:src/api/motor.h
#ifndef MOTOR_H
#define MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

void motor_init(void);
void motor_set_front_left(float duty);
void motor_set_front_right(float duty);
void motor_set_rear_left(float duty);
void motor_set_rear_right(float duty);
void motor_stop_all(void);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_H
```

2. モーター制御実装ファイルを作成します：

```cpp:src/api/motor.cpp
#include "motor.h"
#include <Arduino.h>

// PWMピン定義
const int pwmFrontLeft = 5;
const int pwmFrontRight = 42;
const int pwmRearLeft = 10;
const int pwmRearRight = 41;

// PWMチャンネル
const int FrontLeft_motor = 0;
const int FrontRight_motor = 1;
const int RearLeft_motor = 2;
const int RearRight_motor = 3;

// PWM設定
const int freq = 5000;
const int resolution = 8;

void motor_init(void) {
    // PWMチャンネルの設定
    ledcSetup(FrontLeft_motor, freq, resolution);
    ledcSetup(FrontRight_motor, freq, resolution);
    ledcSetup(RearLeft_motor, freq, resolution);
    ledcSetup(RearRight_motor, freq, resolution);

    // ピンのアタッチ
    ledcAttachPin(pwmFrontLeft, FrontLeft_motor);
    ledcAttachPin(pwmFrontRight, FrontRight_motor);
    ledcAttachPin(pwmRearLeft, RearLeft_motor);
    ledcAttachPin(pwmRearRight, RearRight_motor);

    // 初期化時はモーターを停止状態に
    motor_stop_all();
}

void motor_set_front_left(float duty) {
    // 値を0〜1.0の範囲に制限
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    ledcWrite(FrontLeft_motor, (uint32_t)(255 * duty));
}

void motor_set_front_right(float duty) {
    // 値を0〜1.0の範囲に制限
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    ledcWrite(FrontRight_motor, (uint32_t)(255 * duty));
}

void motor_set_rear_left(float duty) {
    // 値を0〜1.0の範囲に制限
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    ledcWrite(RearLeft_motor, (uint32_t)(255 * duty));
}

void motor_set_rear_right(float duty) {
    // 値を0〜1.0の範囲に制限
    if (duty < 0.0f) duty = 0.0f;
    if (duty > 1.0f) duty = 1.0f;

    ledcWrite(RearRight_motor, (uint32_t)(255 * duty));
}

void motor_stop_all(void) {
    ledcWrite(FrontLeft_motor, 0);
    ledcWrite(FrontRight_motor, 0);
    ledcWrite(RearLeft_motor, 0);
    ledcWrite(RearRight_motor, 0);
}
```

3. mruby からのバインディングを作成します：

```cpp:src/m5u/c_motor.h
extern "C" {
    void class_motor_init();
}
```

4. モータークラスの実装を作成します：

```cpp:src/m5u/c_motor.cpp
#include <M5Unified.h>
#include "my_mrubydef.h"
#include "c_motor.h"
#include "../api/motor.h"

#ifdef USE_MOTOR

static mrbc_class *motor_class;

// モータークラスの実装
static void c_motor_init_method(mrb_vm *vm, mrb_value *v, int argc) {
    motor_init();
}

static void c_motor_front_left(mrb_vm *vm, mrb_value *v, int argc) {
    float duty = 0;
    if (argc > 0) {
        if (v[1].tt == MRBC_TT_FLOAT) {
            duty = v[1].d;
        } else if (v[1].tt == MRBC_TT_INTEGER) {
            duty = (float)v[1].i / 100.0f;
        }
    }
    motor_set_front_left(duty);
}

static void c_motor_front_right(mrb_vm *vm, mrb_value *v, int argc) {
    float duty = 0;
    if (argc > 0) {
        if (v[1].tt == MRBC_TT_FLOAT) {
            duty = v[1].d;
        } else if (v[1].tt == MRBC_TT_INTEGER) {
            duty = (float)v[1].i / 100.0f;
        }
    }
    motor_set_front_right(duty);
}

static void c_motor_rear_left(mrb_vm *vm, mrb_value *v, int argc) {
    float duty = 0;
    if (argc > 0) {
        if (v[1].tt == MRBC_TT_FLOAT) {
            duty = v[1].d;
        } else if (v[1].tt == MRBC_TT_INTEGER) {
            duty = (float)v[1].i / 100.0f;
        }
    }
    motor_set_rear_left(duty);
}

static void c_motor_rear_right(mrb_vm *vm, mrb_value *v, int argc) {
    float duty = 0;
    if (argc > 0) {
        if (v[1].tt == MRBC_TT_FLOAT) {
            duty = v[1].d;
        } else if (v[1].tt == MRBC_TT_INTEGER) {
            duty = (float)v[1].i / 100.0f;
        }
    }
    motor_set_rear_right(duty);
}

static void c_motor_stop_all(mrb_vm *vm, mrb_value *v, int argc) {
    motor_stop_all();
}

// クラス初期化関数
void class_motor_init() {
    // モータークラスの定義a
    motor_class = mrbc_define_class(0, "Motor", mrbc_class_object);

    // クラスメソッドの定義
    mrbc_define_method(0, motor_class, "init", c_motor_init_method);
    mrbc_define_method(0, motor_class, "front_left", c_motor_front_left);
    mrbc_define_method(0, motor_class, "front_right", c_motor_front_right);
    mrbc_define_method(0, motor_class, "rear_left", c_motor_rear_left);
    mrbc_define_method(0, motor_class, "rear_right", c_motor_rear_right);
    mrbc_define_method(0, motor_class, "stop_all", c_motor_stop_all);
}

#endif // USE_MOTOR
```

5. `my_mrubydef.h`に新しい機能のための定義を追加します：

Read file: src/m5u/my_mrubydef.h 6. `my_mrubydef.h`に追加する行:

```cpp
#define USE_MOTOR             // to support Motor functions
```

7. `c_m5u.cpp`ファイルを修正してモータークラスを初期化に追加します：

Read file: src/m5u/c_m5u.cpp 8. 最終的な`c_m5u.cpp`の修正版:

```cpp:src/m5u/c_m5u.cpp
#include "c_m5u.h"

#include <M5Unified.h>

#include "c_canvas.h"
#include "c_display_button.h"
#include "c_font.h"
#include "c_m5.h"
#include "c_motor.h"  // 追加
#include "c_speaker.h"
#include "c_touch.h"
#include "c_utils.h"
#include "my_mrubydef.h"

void init_c_m5u() {
  class_m5_init();
  class_display_button_init();
  class_utils_init();
#ifdef USE_TOUCH
  class_touch_init();
#endif
#ifdef USE_CANVAS
  class_canvas_init();
#endif
#ifdef USE_SPEAKER
  class_speaker_init();
#endif
#ifdef USE_FONT
  class_font_init();
//  c_font_add("JapanGothic_8", &lgfxJapanGothic_8);
#endif
#ifdef USE_MOTOR
  class_motor_init();  // 追加
#endif
}
```

9. Ruby からモーターを制御するサンプルコード:

```ruby
# モーター制御の例
Motor.init  # モーターの初期化

# 各モーターを個別に制御（出力0～1.0）
Motor.front_left(0.3)   # 左前モーターを30%の出力で回転
Motor.front_right(0.3)  # 右前モーターを30%の出力で回転
Motor.rear_left(0.3)    # 左後モーターを30%の出力で回転
Motor.rear_right(0.3)   # 右後モーターを30%の出力で回転

sleep(2)  # 2秒間回転

# 全モーター停止
Motor.stop_all
```

10. モーター制御のサンプルスクリプトを Blink のスロットに保存する例:

```ruby:src/rb/motor_sample.rb
# モーター制御サンプル
M5.begin

M5.Lcd.print("モーター制御テスト")
Motor.init

# 前進するためのモーター制御
def forward(speed)
  Motor.front_left(speed)
  Motor.front_right(speed)
  Motor.rear_left(speed)
  Motor.rear_right(speed)
end

# 後退するためのモーター制御
def backward(speed)
  Motor.front_left(-speed)
  Motor.front_right(-speed)
  Motor.rear_left(-speed)
  Motor.rear_right(-speed)
end

# 右旋回するためのモーター制御
def turn_right(speed)
  Motor.front_left(speed)
  Motor.front_right(-speed)
  Motor.rear_left(speed)
  Motor.rear_right(-speed)
end

# 左旋回するためのモーター制御
def turn_left(speed)
  Motor.front_left(-speed)
  Motor.front_right(speed)
  Motor.rear_left(-speed)
  Motor.rear_right(speed)
end

# メインループ
while true
  if M5.BtnA.wasPressed  # ボタンAが押されたら前進
    M5.Lcd.setCursor(0, 20)
    M5.Lcd.print("前進中...")
    forward(0.3)
    sleep(2)
    Motor.stop_all
    M5.Lcd.setCursor(0, 20)
    M5.Lcd.print("停止中    ")
  end

  if M5.BtnB.wasPressed  # ボタンBが押されたら後退
    M5.Lcd.setCursor(0, 20)
    M5.Lcd.print("後退中...")
    backward(0.3)
    sleep(2)
    Motor.stop_all
    M5.Lcd.setCursor(0, 20)
    M5.Lcd.print("停止中    ")
  end

  if M5.BtnC.wasPressed  # ボタンCが押されたら右旋回
    M5.Lcd.setCursor(0, 20)
    M5.Lcd.print("旋回中...")
    turn_right(0.3)
    sleep(2)
    Motor.stop_all
    M5.Lcd.setCursor(0, 20)
    M5.Lcd.print("停止中    ")
  end

  M5.update
  sleep(0.1)
end
```

以上の手順で M5StampFly のモーター制御機能を OpenBlink に組み込むことができます。これにより、Ruby コードから簡単にモーターを制御できるようになります。
