/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.c
 * @brief Implementation of UART API for mruby/c
 *
 * Implements the UART class and its methods for the mruby/c VM,
 * providing functionality to communicate using UART on ESP32.
 */
#include "uart.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// #include "../drv/uart.h" // drvレイヤーは使用しない
#include "../lib/fn.h"
#include "driver/gpio.h"        // GPIO設定用
#include "driver/uart.h"        // ESP-IDFのUARTドライバヘッダ
#include "esp_log.h"            // ログ出力用 (エラー表示などに便利)
#include "freertos/FreeRTOS.h"  // pdMS_TO_TICKS用
#include "freertos/task.h"      // pdMS_TO_TICKS用
#include "mrubyc.h"

// ESP-IDF UARTドライバ関数のプロトタイプ宣言 // 不要になったため削除

// ESP-IDF のエラーコード定義（存在しない場合のみ） //
// 通常はヘッダに含まれるため削除検討

// 初期化状態フラグ
// (UART_NUM_MAXを使用するのが望ましいが、ここでは3のままにしておく)
static bool uart_initialized[UART_NUM_MAX] = {
    false};                            // サイズをUART_NUM_MAXに変更し、初期化
static const char *TAG = "mrbc_uart";  // ログ用タグ

/**
 * @brief mruby/c用のメソッド実装の前方宣言
 */
static void c_uart_init(mrb_vm *vm, mrb_value *v, int argc);
static void c_uart_write(mrb_vm *vm, mrb_value *v, int argc);
static void c_uart_read(mrb_vm *vm, mrb_value *v, int argc);
static void c_uart_deinit(mrb_vm *vm, mrb_value *v, int argc);
static void c_uart_available(mrb_vm *vm, mrb_value *v, int argc);

/**
 * @brief mruby/c用のUARTクラスとメソッドを定義
 *
 * UARTクラスを作成し、init、write、read、deinitメソッドを登録
 *
 * @return kSuccess 常に成功
 */
fn_t api_uart_define(void) {
  mrb_class *class_uart;
  class_uart = mrbc_define_class(0, "UART", mrbc_class_object);

  // クラスメソッドを定義
  mrbc_define_method(0, class_uart, "init", c_uart_init);
  mrbc_define_method(0, class_uart, "write", c_uart_write);
  mrbc_define_method(0, class_uart, "read", c_uart_read);
  mrbc_define_method(0, class_uart, "deinit", c_uart_deinit);
  mrbc_define_method(0, class_uart, "available", c_uart_available);

  return kSuccess;
}

/**
 * @brief UARTクラスのinitメソッドの実装
 *
 * UARTポートを初期化します。
 * 引数: port_num - ポート番号(1または2), tx_pin - 送信ピン, rx_pin - 受信ピン,
 * [baud_rate] - ボーレート(デフォルト115200) 戻り値: 成功時true、失敗時false
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_init(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // デフォルトは失敗

  // 最低3個の引数が必要（ポート番号、TX、RX）
  // オプション引数：ボーレート、RXバッファサイズ、TXバッファサイズ
  if (argc < 3) {
    mrbc_raisef(vm, MRBC_CLASS(ArgumentError),
                "wrong number of arguments (given %d, expected 3..6)", argc);
    return;
  }

  // 引数の取得と型チェック
  if (v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER ||
      v[3].tt != MRBC_TT_INTEGER) {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "invalid argument type");
    return;
  }

  int port_num_int = v[1].i;
  int tx_pin = v[2].i;
  int rx_pin = v[3].i;
  int baud_rate = 115200;     // デフォルトボーレート
  int rx_buffer_size = 1024;  // デフォルトRXバッファ
  int tx_buffer_size = 0;     // デフォルトTXバッファ (0はバッファなし)

  // オプションの引数を処理
  if (argc >= 4 && v[4].tt == MRBC_TT_INTEGER) baud_rate = v[4].i;
  if (argc >= 5 && v[5].tt == MRBC_TT_INTEGER) rx_buffer_size = v[5].i;
  if (argc >= 6 && v[6].tt == MRBC_TT_INTEGER) tx_buffer_size = v[6].i;

  // ポート番号チェック (0, 1, 2)
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "invalid UART port number");
    return;
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // すでに初期化済みの場合、一度削除する
  if (uart_initialized[port_num]) {
    ESP_LOGW(TAG, "UART%d already initialized. Deinitializing first.",
             port_num);
    uart_driver_delete(port_num);
    uart_initialized[port_num] = false;
  }

  // UARTドライバのインストール
  esp_err_t err =
      uart_driver_install(port_num, rx_buffer_size, tx_buffer_size, 0, NULL, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install UART%d driver: %s", port_num,
             esp_err_to_name(err));
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "uart_driver_install failed");
    return;
  }

  // UARTパラメータの設定
  uart_config_t uart_config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,  // または UART_SCLK_APB
  };
  err = uart_param_config(port_num, &uart_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure UART%d parameters: %s", port_num,
             esp_err_to_name(err));
    uart_driver_delete(port_num);  // インストールしたドライバを削除
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "uart_param_config failed");
    return;
  }

  // ピンの割り当て
  // GPIO_NUM_NC を指定するとピンを割り当てない
  err = uart_set_pin(port_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE,
                     UART_PIN_NO_CHANGE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set UART%d pins (TX:%d, RX:%d): %s", port_num,
             tx_pin, rx_pin, esp_err_to_name(err));
    uart_driver_delete(port_num);  // インストールしたドライバを削除
    mrbc_raise(vm, MRBC_CLASS(RuntimeError), "uart_set_pin failed");
    return;
  }

  uart_initialized[port_num] = true;
  ESP_LOGI(TAG, "UART%d initialized (TX:%d, RX:%d, Baud:%d)", port_num, tx_pin,
           rx_pin, baud_rate);
  SET_TRUE_RETURN();
}

/**
 * @brief UARTクラスのwriteメソッドの実装
 *
 * UARTポートからデータを送信します。
 * 引数: port_num - ポート番号(1または2), data - 送信データ（文字列）
 * 戻り値: 送信したバイト数、エラー時は-1
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_write(mrb_vm *vm, mrb_value *v, int argc) {
  SET_INT_RETURN(-1);  // デフォルトはエラー

  // 引数チェック
  if (argc < 2 || v[1].tt != MRBC_TT_INTEGER) {
    return;
  }

  int port_num_int = v[1].i;

  // ポート番号チェック (0, 1, 2) と初期化チェック
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX ||
      !uart_initialized[port_num_int]) {
    mrbc_raise(vm, MRBC_CLASS(StandardError),
               "UART port not initialized or invalid");
    return;
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // 送信データの型チェック
  if (v[2].tt == MRBC_TT_STRING) {
    // 文字列データ
    const char *data = (const char *)v[2].string->data;
    size_t len = v[2].string->size;

    // データ送信
    int written = uart_write_bytes(port_num, data, len);
    if (written < 0) {
      ESP_LOGE(TAG, "UART%d write failed", port_num);
      // エラー発生時は-1を返す仕様のままにする
    }
    SET_INT_RETURN(written);

  } else if (v[2].tt == MRBC_TT_ARRAY) {
    // 配列データ（バイト配列）
    int len = v[2].array->n_stored;
    if (len <= 0) {
      SET_INT_RETURN(0);  // 空の配列なら0バイト書き込み
      return;
    }

    // 動的にバッファ確保
    uint8_t *buf = (uint8_t *)mrbc_alloc(vm, len);
    if (!buf) {
      mrbc_raise(vm, MRBC_CLASS(NoMemoryError),
                 "failed to allocate buffer for write");
      return;  // エラー時は-1が返る
    }

    // 配列からバイトデータを抽出
    bool type_error = false;
    for (int i = 0; i < len; i++) {
      mrb_value *item = &v[2].array->data[i];
      if (item->tt == MRBC_TT_INTEGER) {
        // 0-255の範囲にクリップする (mruby/cの整数は範囲が広い)
        int val = item->i;
        if (val < 0) val = 0;
        if (val > 255) val = 255;
        buf[i] = (uint8_t)val;
      } else {
        // 整数でない要素があればエラー
        type_error = true;
        break;
      }
    }

    int written = -1;
    if (type_error) {
      mrbc_raise(vm, MRBC_CLASS(TypeError),
                 "array contains non-integer elements");
    } else {
      // データ送信
      written = uart_write_bytes(port_num, (const char *)buf, len);
      if (written < 0) {
        ESP_LOGE(TAG, "UART%d write array failed", port_num);
      }
    }

    // バッファ解放
    mrbc_free(vm, buf);
    SET_INT_RETURN(written);

  } else {
    mrbc_raise(vm, MRBC_CLASS(TypeError), "data must be String or Array");
    return;  // エラー時は-1が返る
  }
}

/**
 * @brief UARTクラスのreadメソッドの実装
 *
 * UARTポートからデータを受信します。
 * 引数: port_num - ポート番号(1または2), length - 読み込むバイト数,
 * [timeout_ms] - タイムアウト（ミリ秒、デフォルト100ms） 戻り値:
 * 受信したデータ（文字列）、エラー時はnil
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_read(mrb_vm *vm, mrb_value *v, int argc) {
  SET_NIL_RETURN();  // デフォルトはnil

  // 引数チェック（最低2つ必要：ポート番号、読み込むバイト数）
  if (argc < 2 || v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER) {
    return;
  }

  int port_num_int = v[1].i;
  int length = v[2].i;
  uint32_t timeout_ms = 100;  // デフォルトタイムアウト

  // ポート番号チェック (0, 1, 2) と初期化チェック
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX ||
      !uart_initialized[port_num_int]) {
    mrbc_raise(vm, MRBC_CLASS(StandardError),
               "UART port not initialized or invalid");
    return;  // エラー時はnilが返る
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // 読み込むバイト数のチェック
  if (length <= 0) {
    // 0バイト読み込み要求は空文字列を返す
    mrb_value result = mrbc_string_new(vm, "", 0);
    SET_RETURN(result);
    return;
  }
  // ESP-IDFのバッファサイズに合わせるか、より大きなサイズを許可するか検討
  // ここでは上限を設けないが、メモリ使用量に注意
  // if (length > 1024) length = 1024;

  // タイムアウト指定がある場合
  if (argc >= 3 && v[3].tt == MRBC_TT_INTEGER) {
    timeout_ms = (uint32_t)v[3].i;
    if ((int32_t)timeout_ms < 0) timeout_ms = 0;  // 負の値は0として扱う
  }
  // ESP-IDFのタイムアウトはTickType_tで指定
  TickType_t timeout_ticks = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);
  // タイムアウト0msでも最低1Tick待つようにする (ESP-IDFの仕様に依存)
  // if (timeout_ms > 0 && timeout_ticks == 0) timeout_ticks = 1;

  // データ受信用バッファ
  uint8_t *buf = (uint8_t *)mrbc_alloc(vm, length);
  if (!buf) {
    mrbc_raise(vm, MRBC_CLASS(NoMemoryError),
               "failed to allocate buffer for read");
    return;  // メモリ確保失敗時はnil
  }

  // データ受信 (uart_read_bytes)
  int read_bytes = uart_read_bytes(port_num, buf, length, timeout_ticks);

  if (read_bytes > 0) {
    // 受信データを文字列として返す
    mrb_value result = mrbc_string_new(vm, (const char *)buf, read_bytes);
    SET_RETURN(result);
  } else if (read_bytes == 0) {
    // タイムアウトまたはデータなし
    mrb_value result = mrbc_string_new(vm, "", 0);  // 空文字列を返す
    SET_RETURN(result);
  } else {
    // エラー発生 (-1 が返る場合など)
    ESP_LOGE(TAG, "UART%d read failed", port_num);
    // エラー時はnilを返す仕様のまま
    SET_NIL_RETURN();
  }

  // バッファ解放
  mrbc_free(vm, buf);  // read_bytesの値に関わらず解放する
}

/**
 * @brief UARTクラスのdeinitメソッドの実装
 *
 * UARTポートを終了します。
 * 引数: port_num - ポート番号(1または2)
 * 戻り値: 成功時true、失敗時false
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_deinit(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // デフォルトは失敗

  // 引数チェック
  if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
    return;
  }

  int port_num_int = v[1].i;

  // ポート番号チェック (0, 1, 2)
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX) {
    mrbc_raise(vm, MRBC_CLASS(RangeError), "invalid UART port number");
    return;  // エラー時はfalseが返る
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // 初期化されていなければ何もしない（成功扱い）
  if (!uart_initialized[port_num]) {
    SET_TRUE_RETURN();
    return;
  }

  // UARTドライバの削除
  esp_err_t err = uart_driver_delete(port_num);
  if (err == ESP_OK) {
    uart_initialized[port_num] = false;
    ESP_LOGI(TAG, "UART%d deinitialized", port_num);
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "Failed to deinitialize UART%d: %s", port_num,
             esp_err_to_name(err));
    // 失敗した場合もfalseを返す
    SET_FALSE_RETURN();
  }
}

/**
 * @brief UARTクラスのavailableメソッドの実装
 *
 * UARTポートの受信バッファにデータがあるか確認します。
 * 引数: port_num - ポート番号(1または2)
 * 戻り値: 読み出し可能なバイト数、エラー時は-1
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_available(mrb_vm *vm, mrb_value *v, int argc) {
  SET_INT_RETURN(-1);  // デフォルトはエラー

  // 引数チェック
  if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
    return;
  }

  int port_num_int = v[1].i;

  // ポート番号チェック (0, 1, 2) と初期化チェック
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX ||
      !uart_initialized[port_num_int]) {
    mrbc_raise(vm, MRBC_CLASS(StandardError),
               "UART port not initialized or invalid");
    return;  // エラー時は-1が返る
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  size_t available_bytes = 0;
  // uart_get_buffered_data_len は成功すれば ESP_OK を返す
  esp_err_t err = uart_get_buffered_data_len(port_num, &available_bytes);

  if (err == ESP_OK) {
    SET_INT_RETURN((int)available_bytes);
  } else {
    ESP_LOGE(TAG, "Failed to get available bytes for UART%d: %s", port_num,
             esp_err_to_name(err));
    SET_INT_RETURN(-1);  // エラー時は-1を返す
  }
}