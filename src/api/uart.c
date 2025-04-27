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

// Include the driver layer header
#include "../drv/uart.h"  // Use the driver layer
#include "../lib/fn.h"
// #include "driver/gpio.h"        // GPIO included via drv/uart.h or not needed
// #include "driver/uart.h"        // UART driver included via drv/uart.h
#include "esp_log.h"            // For logging
#include "freertos/FreeRTOS.h"  // For pdMS_TO_TICKS indirectly if needed for timeouts
#include "freertos/task.h"  // For pdMS_TO_TICKS indirectly if needed for timeouts
#include "mrubyc.h"

// Initialization status is now managed by the driver layer
// static bool uart_initialized[UART_NUM_MAX] = {false};
static const char *TAG = "mrbc_uart";

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

  mrbc_define_method(0, class_uart, "init", c_uart_init);
  mrbc_define_method(0, class_uart, "write", c_uart_write);
  mrbc_define_method(0, class_uart, "read", c_uart_read);
  mrbc_define_method(0, class_uart, "deinit", c_uart_deinit);
  mrbc_define_method(0, class_uart, "available", c_uart_available);

  return kSuccess;
}

/**
 * @brief UARTクラスのinitメソッドの実装 (ドライバ層呼び出し)
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
  SET_FALSE_RETURN();  // Default to failure

  // Args: port_num, tx_pin, rx_pin, [baud_rate], [rx_buffer_size],
  // [tx_buffer_size]
  if (argc < 3) {
    ESP_LOGE(TAG, "init: wrong number of arguments (given %d, expected 3..6)",
             argc);
    return;
  }

  // Argument type check
  if (v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER ||
      v[3].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "init: invalid argument type for port/tx/rx");
    return;
  }

  int port_num_int = v[1].i;
  int tx_pin = v[2].i;
  int rx_pin = v[3].i;
  int baud_rate = 115200;     // Default baud
  int rx_buffer_size = 1024;  // Default RX buffer
  int tx_buffer_size = 1024;  // Default TX buffer

  // Process optional arguments
  if (argc >= 4 && v[4].tt == MRBC_TT_INTEGER) baud_rate = v[4].i;
  if (argc >= 5 && v[5].tt == MRBC_TT_INTEGER) rx_buffer_size = v[5].i;
  if (argc >= 6 && v[6].tt == MRBC_TT_INTEGER) tx_buffer_size = v[6].i;

  // Port number check (use UART_NUM_MAX from ESP-IDF via drv layer)
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX) {
    ESP_LOGE(TAG, "init: invalid UART port number %d", port_num_int);
    return;
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // Call driver init function
  fn_t result = drv_uart_init(port_num, tx_pin, rx_pin, baud_rate,
                              rx_buffer_size, tx_buffer_size);

  if (result == kSuccess) {
    ESP_LOGI(TAG, "drv_uart_init for UART%d succeeded.", port_num);
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "drv_uart_init for UART%d failed.", port_num);
    SET_FALSE_RETURN();
  }
}

/**
 * @brief UARTクラスのwriteメソッドの実装 (ドライバ層呼び出し)
 *
 * UARTポートからデータを送信します。
 * 引数: port_num - ポート番号(1または2), data -
 * 送信データ（文字列またはバイト配列） 戻り値: 送信したバイト数、エラー時は-1
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_write(mrb_vm *vm, mrb_value *v, int argc) {
  SET_INT_RETURN(-1);  // Default to error (-1 bytes written)

  // Args: port_num, data (String or Array)
  if (argc < 2 || v[1].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "write: invalid argument count or type for port");
    return;
  }

  int port_num_int = v[1].i;

  // Port number check
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX) {
    ESP_LOGE(TAG, "write: invalid UART port number %d", port_num_int);
    return;
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // Initialization check is handled by the driver layer

  const void *data_ptr = NULL;
  size_t data_len = 0;
  uint8_t *temp_buf = NULL;  // For array conversion
  if (v[2].tt == MRBC_TT_STRING) {
    // String data
    data_ptr =
        (const void *)mrbc_string_cstr(&v[2]);  // Use safe C string getter
    data_len = mrbc_string_size(&v[2]);
  } else if (v[2].tt == MRBC_TT_ARRAY) {
    // Array data (byte array)
    data_len = mrbc_array_size(&v[2]);
    if (data_len == 0) {
      SET_INT_RETURN(0);  // Wrote 0 bytes
      return;
    }

    // Allocate temporary buffer
    temp_buf = (uint8_t *)mrbc_alloc(vm, data_len);
    if (!temp_buf) {
      ESP_LOGE(TAG, "write: failed to allocate buffer for array write");
      return;  // Returns -1
    }

    // Extract bytes from array
    bool type_error = false;
    for (int i = 0; i < data_len; i++) {
      mrb_value item = mrbc_array_get(&v[2], i);
      if (item.tt == MRBC_TT_INTEGER) {
        int val = item.i;
        // Clip to 0-255
        if (val < 0) val = 0;
        if (val > 255) val = 255;
        temp_buf[i] = (uint8_t)val;
      } else {
        type_error = true;
        break;
      }
    }

    if (type_error) {
      ESP_LOGE(TAG, "write: array contains non-integer elements");
      mrbc_free(vm, temp_buf);
      return;  // Returns -1
    }
    data_ptr = (const void *)temp_buf;
  } else {
    ESP_LOGE(TAG, "write: data must be String or Array");
    return;  // Returns -1
  }

  // Call driver write function
  int written = drv_uart_write(port_num, data_ptr, data_len);

  // Free temporary buffer if allocated
  if (temp_buf) {
    mrbc_free(vm, temp_buf);
  }

  // Set mrbc return value
  SET_INT_RETURN(written);
}

/**
 * @brief UARTクラスのreadメソッドの実装 (ドライバ層呼び出し)
 *
 * UARTポートからデータを受信します。
 * 引数: port_num - ポート番号(1または2), length - 読み込むバイト数,
 * [timeout_ms] - タイムアウト（ミリ秒、デフォルト100ms） 戻り値:
 * 受信したデータ（文字列）、データなしの場合は空文字列、エラー時はnil
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_read(mrb_vm *vm, mrb_value *v, int argc) {
  SET_NIL_RETURN();  // Default to nil on error or no data? Let's return empty
                     // string for no data.

  // Args: port_num, length, [timeout_ms]
  if (argc < 2 || v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "read: invalid argument count or type");
    SET_NIL_RETURN();  // Return nil for argument errors
    return;
  }

  int port_num_int = v[1].i;
  int length = v[2].i;
  uint32_t timeout_ms = 100;  // Default timeout

  // Port number check
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX) {
    ESP_LOGE(TAG, "read: invalid UART port number %d", port_num_int);
    SET_NIL_RETURN();
    return;
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // Length check
  if (length <= 0) {
    mrbc_value result =
        mrbc_string_new_cstr(vm, "");  // Return empty string for 0 length
    SET_RETURN(result);
    return;
  }

  // Optional timeout argument
  if (argc >= 3 && v[3].tt == MRBC_TT_INTEGER) {
    int timeout_arg = v[3].i;
    timeout_ms = (timeout_arg < 0) ? 0 : (uint32_t)timeout_arg;
  }

  // Allocate buffer for reading
  uint8_t *buf = (uint8_t *)mrbc_alloc(vm, length);
  if (!buf) {
    ESP_LOGE(TAG, "read: failed to allocate buffer (%d bytes)", length);
    SET_NIL_RETURN();
    return;
  }

  // Call driver read function
  int read_bytes = drv_uart_read(port_num, buf, length, timeout_ms);

  if (read_bytes > 0) {
    // Success: return string with read data
    mrbc_value result = mrbc_string_new(vm, (const char *)buf, read_bytes);
    SET_RETURN(result);
  } else if (read_bytes == 0) {
    // Timeout or no data: return empty string
    mrbc_value result = mrbc_string_new_cstr(vm, "");
    SET_RETURN(result);
  } else {
    // Error from driver layer
    ESP_LOGE(TAG, "drv_uart_read for UART%d failed (returned %d)", port_num,
             read_bytes);
    SET_NIL_RETURN();  // Return nil on error
  }

  // Free buffer
  mrbc_free(vm, buf);
}

/**
 * @brief UARTクラスのdeinitメソッドの実装 (ドライバ層呼び出し)
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
  SET_FALSE_RETURN();  // Default to failure

  // Args: port_num
  if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "deinit: invalid argument count or type");
    return;
  }

  int port_num_int = v[1].i;

  // Port number check
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX) {
    ESP_LOGE(TAG, "deinit: invalid UART port number %d", port_num_int);
    return;
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  // Call driver deinit function
  fn_t result = drv_uart_deinit(port_num);

  if (result == kSuccess) {
    // ESP_LOGI(TAG, "drv_uart_deinit for UART%d succeeded.", port_num); //
    // Logged in drv layer
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "drv_uart_deinit for UART%d failed.", port_num);
    SET_FALSE_RETURN();
  }
}

/**
 * @brief UARTクラスのavailableメソッドの実装 (ドライバ層呼び出し)
 *
 * UARTポートの受信バッファにある読み出し可能なバイト数を返します。
 * 引数: port_num - ポート番号(1または2)
 * 戻り値: 読み出し可能なバイト数、エラー時は0
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_uart_available(mrb_vm *vm, mrb_value *v, int argc) {
  SET_INT_RETURN(0);  // Default to 0 (error or no bytes)

  // Args: port_num
  if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "available: invalid argument count or type");
    return;
  }

  int port_num_int = v[1].i;

  // Port number check
  if (port_num_int < 0 || port_num_int >= UART_NUM_MAX) {
    ESP_LOGE(TAG, "available: invalid UART port number %d", port_num_int);
    return;
  }
  uart_port_t port_num = (uart_port_t)port_num_int;

  size_t available_bytes = 0;
  // Call driver get_available function
  fn_t result = drv_uart_get_available(port_num, &available_bytes);

  if (result == kSuccess) {
    SET_INT_RETURN((int)available_bytes);
  } else {
    // Error logged in drv layer or port not initialized
    // ESP_LOGE(TAG, "drv_uart_get_available for UART%d failed.", port_num);
    SET_INT_RETURN(0);  // Return 0 on failure
  }
}