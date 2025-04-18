/**
 * @file pwm.h
 * @brief PWM driver implementation
 *
 * Implements functions for controlling PWM outputs using the ESP32 LEDC
 * peripheral.
 */

#ifndef PWM_H
#define PWM_H

#include <stdint.h>

#include "../lib/fn.h"
#include "driver/gpio.h"

/**
 * @brief PWM機能の初期化
 *
 * LEDC周波数とタイマーを設定します
 *
 * @return kSuccess 初期化成功時
 */
fn_t drv_pwm_init(void);

/**
 * @brief 指定したGPIOピンをPWM出力として設定
 *
 * @param gpio_pin GPIOピン番号
 * @param initial_duty 初期デューティ値 (0-100%)
 * @return 成功時にチャンネル番号、失敗時に-1
 */
int drv_pwm_setup_pin(gpio_num_t gpio_pin, uint8_t initial_duty);

/**
 * @brief PWMのデューティ比を設定
 *
 * @param channel チャンネル番号
 * @param duty デューティ比 (0-100%)
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_pwm_set_duty(int channel, uint8_t duty);

/**
 * @brief PWMチャンネルを無効化
 *
 * @param channel チャンネル番号
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_pwm_disable(int channel);

#endif /* PWM_H */