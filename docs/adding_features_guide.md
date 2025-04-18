# mruby/c マイコンプロジェクトへの機能追加ガイド

## 1. 概要

mruby/c は軽量なマイコン向け Ruby 実行環境です。OpenBlink は、M5Stack などのマイコンデバイス上で mruby/c を使って開発するためのフレームワークです。新しい機能を追加するには、以下の手順に従います。

## 2. 全体的なアーキテクチャ

```
┌────────────────┐    ┌────────────────┐    ┌────────────────┐
│  Ruby スクリプト  │─→│  mruby/c VM     │─→│   C/C++ API    │
└────────────────┘    └────────────────┘    └────────┬───────┘
                                                     │
                                                     ↓
                                            ┌────────────────┐
                                            │ ドライバレイヤー  │
                                            └────────────────┘
```

## 3. 新機能追加の基本的な流れ

1. **ドライバレイヤーの作成**：ハードウェアを直接制御する低レベル関数
2. **API レイヤーの作成**：mruby/c VM からドライバを利用するためのブリッジ
3. **機能の有効化**：設定ファイルでの機能の有効化
4. **クラス初期化の追加**：メインの初期化関数への登録

## 4. 実装手順

### 4.1 ドライバレイヤーの作成

新しいハードウェア機能（例：温度センサー）を制御するためのドライバを作成します。

1. `src/drv/` ディレクトリに新しいファイルを作成
   - 例：`temperature.h` と `temperature.c`

```c
// temperature.h
#ifndef DRV_TEMPERATURE_H
#define DRV_TEMPERATURE_H

#include "../lib/fn.h"

fn_t drv_temperature_init(void);
float drv_temperature_read(void);

#endif
```

```c
// temperature.c
#include "temperature.h"
#include <driver/gpio.h>
// 必要なヘッダーファイル

fn_t drv_temperature_init(void) {
    // センサーの初期化コード
    return kSuccess;
}

float drv_temperature_read(void) {
    // センサーからの読み取りコード
    return 25.5f; // 仮の値
}
```

### 4.2 API レイヤーの作成

mruby/c VM からドライバを利用するための API を作成します。

1. `src/api/` ディレクトリに新しいファイルを作成
   - 例：`temperature.h` と `temperature.c`

```c
// temperature.h
#ifndef API_TEMPERATURE_H
#define API_TEMPERATURE_H

#include "../lib/fn.h"

fn_t api_temperature_define(void);

#endif
```

```c
// temperature.c
#include "temperature.h"
#include "../drv/temperature.h"
#include "mrubyc.h"

static void c_read_temperature(mrb_vm *vm, mrb_value *v, int argc) {
    float temp = drv_temperature_read();
    SET_FLOAT_RETURN(temp);
}

fn_t api_temperature_define(void) {
    mrb_class *class_temperature;
    class_temperature = mrbc_define_class(0, "Temperature", mrbc_class_object);
    mrbc_define_method(0, class_temperature, "read", c_read_temperature);
    return kSuccess;
}
```

### 4.3 M5U 拡張機能の場合

より複雑な機能の場合は、`src/m5u/` ディレクトリに実装します。

1. ヘッダーファイルを作成
   - 例：`c_temperature.h`

```c
extern "C" {
    void class_temperature_init();
}
```

2. 実装ファイルを作成
   - 例：`c_temperature.cpp`

```cpp
#include <M5Unified.h>
#include "my_mrubydef.h"
#include "c_temperature.h"
#include "../drv/temperature.h"

#ifdef USE_TEMPERATURE

static mrbc_class *temperature_class;

static void c_temperature_read(mrb_vm *vm, mrb_value *v, int argc) {
    float temp = drv_temperature_read();
    SET_FLOAT_RETURN(temp);
}

void class_temperature_init() {
    temperature_class = mrbc_define_class(0, "Temperature", mrbc_class_object);
    mrbc_define_method(0, temperature_class, "read", c_temperature_read);
}

#endif // USE_TEMPERATURE
```

### 4.4 設定ファイルの修正

1. `src/m5u/my_mrubydef.h` に機能フラグを追加

```c
#define USE_TEMPERATURE       // 温度センサー機能を有効化
```

### 4.5 初期化関数への追加

1. `src/m5u/c_m5u.cpp` を修正して新しいクラス初期化関数を呼び出す

```cpp
#include "c_temperature.h"  // 追加

void init_c_m5u() {
    // 既存の初期化コード

#ifdef USE_TEMPERATURE
    class_temperature_init();  // 追加
#endif
}
```

2. もしくは、メインアプリケーションに直接追加（`src/main.c`）

```c
#include "api/temperature.h"  // 追加

void app_main() {
    // 既存のコード

    // mruby/c クラスと機能の定義
    api_led_define();
    api_input_define();
    api_temperature_define();  // 追加

    // 残りのコード
}
```

## 5. PWM 機能の追加例

### 5.1 新しい関数の追加

```c
// motor.h
#ifndef DRV_MOTOR_H
#define DRV_MOTOR_H

#include "../lib/fn.h"
#include "driver/gpio.h"

fn_t drv_motor_init(void);
fn_t drv_motor_set_speed(uint8_t motor_num, uint8_t speed);
fn_t drv_motor_stop_all(void);

#endif
```

```c
// motor.c
#include "motor.h"
#include "driver/ledc.h"
#include "esp_log.h"

// PWM設定
#define MOTOR_PWM_FREQ        5000
#define MOTOR_PWM_RESOLUTION  LEDC_TIMER_13_BIT
#define MOTOR_MAX_CHANNELS    4

// モーターチャンネル情報
static struct {
    ledc_channel_t channel;
    gpio_num_t gpio;
    bool in_use;
} motor_channels[MOTOR_MAX_CHANNELS] = {0};

static bool motor_initialized = false;

fn_t drv_motor_init(void) {
    if (motor_initialized) {
        return kSuccess;
    }

    // LEDC設定
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = MOTOR_PWM_RESOLUTION,
        .freq_hz = MOTOR_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };

    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        return kFailure;
    }

    // チャンネル初期化
    for (int i = 0; i < MOTOR_MAX_CHANNELS; i++) {
        motor_channels[i].channel = i;
        motor_channels[i].in_use = false;
    }

    // モーターピン設定（例）
    motor_channels[0].gpio = GPIO_NUM_5;   // 前左モーター
    motor_channels[1].gpio = GPIO_NUM_16;  // 前右モーター
    motor_channels[2].gpio = GPIO_NUM_17;  // 後左モーター
    motor_channels[3].gpio = GPIO_NUM_18;  // 後右モーター

    // PWMチャンネル設定
    for (int i = 0; i < MOTOR_MAX_CHANNELS; i++) {
        gpio_reset_pin(motor_channels[i].gpio);

        ledc_channel_config_t ledc_channel = {
            .channel = motor_channels[i].channel,
            .duty = 0,
            .gpio_num = motor_channels[i].gpio,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .hpoint = 0,
            .timer_sel = LEDC_TIMER_0
        };

        err = ledc_channel_config(&ledc_channel);
        if (err != ESP_OK) {
            return kFailure;
        }

        motor_channels[i].in_use = true;
    }

    motor_initialized = true;
    return kSuccess;
}

fn_t drv_motor_set_speed(uint8_t motor_num, uint8_t speed) {
    if (motor_num >= MOTOR_MAX_CHANNELS || !motor_channels[motor_num].in_use) {
        return kFailure;
    }

    // 0-100%を0-8191の範囲に変換
    uint32_t duty = (speed * 8191) / 100;

    esp_err_t err = ledc_set_duty(LEDC_LOW_SPEED_MODE, motor_channels[motor_num].channel, duty);
    if (err != ESP_OK) {
        return kFailure;
    }

    err = ledc_update_duty(LEDC_LOW_SPEED_MODE, motor_channels[motor_num].channel);
    if (err != ESP_OK) {
        return kFailure;
    }

    return kSuccess;
}

fn_t drv_motor_stop_all(void) {
    for (int i = 0; i < MOTOR_MAX_CHANNELS; i++) {
        if (motor_channels[i].in_use) {
            ledc_set_duty(LEDC_LOW_SPEED_MODE, motor_channels[i].channel, 0);
            ledc_update_duty(LEDC_LOW_SPEED_MODE, motor_channels[i].channel);
        }
    }

    return kSuccess;
}
```

### 5.2 API レイヤーの作成

```c
// motor.h (API)
#ifndef API_MOTOR_H
#define API_MOTOR_H

#include "../lib/fn.h"

fn_t api_motor_define(void);

#endif
```

```c
// motor.c (API)
#include "motor.h"
#include "../drv/motor.h"
#include "mrubyc.h"

static void c_motor_init(mrb_vm *vm, mrb_value *v, int argc) {
    fn_t result = drv_motor_init();
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

static void c_motor_set(mrb_vm *vm, mrb_value *v, int argc) {
    uint8_t motor_num = 0;
    uint8_t speed = 0;

    if (argc >= 2 && v[1].tt == MRBC_TT_INTEGER && v[2].tt == MRBC_TT_INTEGER) {
        motor_num = (uint8_t)v[1].i;
        speed = (uint8_t)v[2].i;

        if (speed > 100) {
            speed = 100;
        }

        fn_t result = drv_motor_set_speed(motor_num, speed);
        if (result == kSuccess) {
            SET_TRUE_RETURN();
            return;
        }
    }

    SET_FALSE_RETURN();
}

static void c_motor_stop_all(mrb_vm *vm, mrb_value *v, int argc) {
    fn_t result = drv_motor_stop_all();
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

fn_t api_motor_define(void) {
    mrb_class *class_motor;
    class_motor = mrbc_define_class(0, "Motor", mrbc_class_object);
    mrbc_define_method(0, class_motor, "init", c_motor_init);
    mrbc_define_method(0, class_motor, "set", c_motor_set);
    mrbc_define_method(0, class_motor, "stop_all", c_motor_stop_all);
    return kSuccess;
}
```

### 5.3 高度な M5U 拡張機能の場合

より詳細な機能を提供したい場合は、M5U 拡張を作成します。

```cpp
// c_motor.h
extern "C" {
    void class_motor_init();
}
```

```cpp
// c_motor.cpp
#include <M5Unified.h>
#include "my_mrubydef.h"
#include "c_motor.h"
#include "../drv/motor.h"

#ifdef USE_MOTOR

static mrbc_class *motor_class;

static void c_motor_init_method(mrb_vm *vm, mrb_value *v, int argc) {
    fn_t result = drv_motor_init();
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

static void c_motor_front_left(mrb_vm *vm, mrb_value *v, int argc) {
    if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
        SET_FALSE_RETURN();
        return;
    }

    uint8_t speed = (uint8_t)v[1].i;
    if (speed > 100) speed = 100;

    fn_t result = drv_motor_set_speed(0, speed);
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

static void c_motor_front_right(mrb_vm *vm, mrb_value *v, int argc) {
    if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
        SET_FALSE_RETURN();
        return;
    }

    uint8_t speed = (uint8_t)v[1].i;
    if (speed > 100) speed = 100;

    fn_t result = drv_motor_set_speed(1, speed);
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

static void c_motor_rear_left(mrb_vm *vm, mrb_value *v, int argc) {
    if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
        SET_FALSE_RETURN();
        return;
    }

    uint8_t speed = (uint8_t)v[1].i;
    if (speed > 100) speed = 100;

    fn_t result = drv_motor_set_speed(2, speed);
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

static void c_motor_rear_right(mrb_vm *vm, mrb_value *v, int argc) {
    if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
        SET_FALSE_RETURN();
        return;
    }

    uint8_t speed = (uint8_t)v[1].i;
    if (speed > 100) speed = 100;

    fn_t result = drv_motor_set_speed(3, speed);
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

static void c_motor_stop_all(mrb_vm *vm, mrb_value *v, int argc) {
    fn_t result = drv_motor_stop_all();
    if (result == kSuccess) {
        SET_TRUE_RETURN();
    } else {
        SET_FALSE_RETURN();
    }
}

void class_motor_init() {
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

## 6. Ruby コードでの使用例

新しく追加した機能は、以下のような Ruby コードで使用できます。

```ruby
# モーター制御の例
Motor.init

# 前進
Motor.front_left(50)
Motor.front_right(50)
Motor.rear_left(50)
Motor.rear_right(50)

sleep 2  # 2秒間前進

# 左折
Motor.front_left(20)
Motor.front_right(50)
Motor.rear_left(20)
Motor.rear_right(50)

sleep 1  # 1秒間左折

# 停止
Motor.stop_all
```

## 7. まとめ

mruby/c マイコンプロジェクトに新機能を追加する手順は以下の通りです：

1. ドライバレイヤー（`src/drv/`）：ハードウェアを直接制御する低レベル関数
2. API レイヤー（`src/api/`）：mruby/c からアクセスするためのクラスとメソッド定義
3. 設定ファイル（`src/m5u/my_mrubydef.h`）：機能フラグの追加
4. 初期化関数への登録：`main.c`または`c_m5u.cpp`での初期化呼び出し

この構造に従うことで、既存のプロジェクトに新しい機能を簡単に追加できます。また、条件付きコンパイル（`#ifdef`）を利用することで、必要な機能だけを組み込むことができます。
