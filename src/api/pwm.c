/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file pwm.c
 * @brief Implementation of PWM API for mruby/c
 *
 * Implements the PWM class and its methods for the mruby/c VM,
 * providing functionality to control PWM outputs on GPIO pins.
 */
#include "pwm.h"

#include <stdbool.h>
#include <stdint.h>

#include "../drv/led.h"
#include "../lib/fn.h"
#include "mrubyc.h"

// PWMチャンネルを保持する配列（最大8チャンネル）
static int pwm_channels[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

/**
 * @brief mruby/c用のメソッド実装の前方宣言
 */
static void c_pwm_setup(mrb_vm *vm, mrb_value *v, int argc);
static void c_pwm_set_duty(mrb_vm *vm, mrb_value *v, int argc);
static void c_pwm_disable(mrb_vm *vm, mrb_value *v, int argc);

/**
 * @brief mruby/c用のPWMクラスとメソッドを定義
 *
 * PWMクラスを作成し、setup、set_duty、disableメソッドを登録
 *
 * @return kSuccess 常に成功
 */
fn_t api_pwm_define(void) {
  mrb_class *class_pwm;
  class_pwm = mrbc_define_class(0, "PWM", mrbc_class_object);
  mrbc_define_method(0, class_pwm, "setup", c_pwm_setup);
  mrbc_define_method(0, class_pwm, "set_duty", c_pwm_set_duty);
  mrbc_define_method(0, class_pwm, "disable", c_pwm_disable);
  return kSuccess;
}

/**
 * @brief PWMクラスのsetupメソッドの実装
 *
 * GPIOピンをPWM出力として設定します。
 * 引数: pin_num - GPIOピン番号, initial_duty - 初期デューティ比(0-100%)
 * 戻り値: チャンネル番号（0-7）または -1（失敗時）
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_pwm_setup(mrb_vm *vm, mrb_value *v, int argc) {
  SET_INT_RETURN(-1); // デフォルトは失敗

  // 引数チェック
  if (argc < 2 || v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER) {
    return;
  }

  // ピン番号と初期デューティ比を取得
  int pin_num = v[1].i;
  int initial_duty = v[2].i;

  // 値の範囲チェック
  if (pin_num < 0 || initial_duty < 0 || initial_duty > 100) {
    return;
  }

  // PWMセットアップ
  int channel = drv_pwm_setup_pin(pin_num, initial_duty);
  if (channel >= 0 && channel < 8) {
    // チャンネル番号を保存
    pwm_channels[channel] = pin_num;
    SET_INT_RETURN(channel);
  }
}

/**
 * @brief PWMクラスのset_dutyメソッドの実装
 *
 * PWM出力のデューティ比を設定します。
 * 引数: channel - チャンネル番号, duty - デューティ比(0-100%)
 * 戻り値: 成功時true、失敗時false
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_pwm_set_duty(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN(); // デフォルトは失敗

  // 引数チェック
  if (argc < 2 || v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER) {
    return;
  }

  // チャンネル番号とデューティ比を取得
  int channel = v[1].i;
  int duty = v[2].i;

  // 値の範囲チェック
  if (channel < 0 || channel >= 8 || duty < 0 || duty > 100 || pwm_channels[channel] == -1) {
    return;
  }

  // デューティ比を設定
  if (kSuccess == drv_pwm_set_duty(channel, duty)) {
    SET_TRUE_RETURN();
  }
}

/**
 * @brief PWMクラスのdisableメソッドの実装
 *
 * PWM出力を無効化します。
 * 引数: channel - チャンネル番号
 * 戻り値: 成功時true、失敗時false
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_pwm_disable(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN(); // デフォルトは失敗

  // 引数チェック
  if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
    return;
  }

  // チャンネル番号を取得
  int channel = v[1].i;

  // 値の範囲チェック
  if (channel < 0 || channel >= 8 || pwm_channels[channel] == -1) {
    return;
  }

  // PWM出力を無効化
  if (kSuccess == drv_pwm_disable(channel)) {
    pwm_channels[channel] = -1;
    SET_TRUE_RETURN();
  }
} 