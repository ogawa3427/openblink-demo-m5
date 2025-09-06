/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file usb_serial.c
 * @brief Implementation of USB Serial API for mruby/c
 *
 * Implements the USBSerial class and its methods for the mruby/c VM,
 * providing functionality to communicate using USB Serial/JTAG on ESP32.
 */
#include "usb_serial.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Include the driver layer header
#include "../drv/usb_serial.h"
#include "../lib/fn.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mrubyc.h"

static const char *TAG = "mrbc_usb_serial";

/**
 * @brief mruby/c用のメソッド実装の前方宣言
 */
static void c_usb_serial_init(mrb_vm *vm, mrb_value *v, int argc);
static void c_usb_serial_write(mrb_vm *vm, mrb_value *v, int argc);
static void c_usb_serial_read(mrb_vm *vm, mrb_value *v, int argc);
static void c_usb_serial_deinit(mrb_vm *vm, mrb_value *v, int argc);
static void c_usb_serial_available(mrb_vm *vm, mrb_value *v, int argc);
static void c_usb_serial_connected(mrb_vm *vm, mrb_value *v, int argc);

/**
 * @brief mruby/c用のUSBSerialクラスとメソッドを定義
 *
 * USBSerialクラスを作成し、init、write、read、deinitメソッドを登録
 *
 * @return kSuccess 常に成功
 */
fn_t api_usb_serial_define(void) {
  mrb_class *class_usb_serial;
  class_usb_serial = mrbc_define_class(0, "USBSerial", mrbc_class_object);

  mrbc_define_method(0, class_usb_serial, "init", c_usb_serial_init);
  mrbc_define_method(0, class_usb_serial, "write", c_usb_serial_write);
  mrbc_define_method(0, class_usb_serial, "read", c_usb_serial_read);
  mrbc_define_method(0, class_usb_serial, "deinit", c_usb_serial_deinit);
  mrbc_define_method(0, class_usb_serial, "available", c_usb_serial_available);
  mrbc_define_method(0, class_usb_serial, "connected?", c_usb_serial_connected);

  return kSuccess;
}

/**
 * @brief USBSerialクラスのinitメソッドの実装
 *
 * USB Serial/JTAGを初期化します。
 * 引数: なし
 * 戻り値: 成功時true、失敗時false
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_usb_serial_init(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // Default to failure

  // USB Serial/JTAGは引数不要
  if (argc != 0) {
    ESP_LOGW(TAG, "init: USB Serial/JTAG init takes no arguments (given %d)",
             argc);
  }

  // Call driver init function
  fn_t result = drv_usb_serial_init();

  if (result == kSuccess) {
    ESP_LOGI(TAG, "USB Serial/JTAG init succeeded");
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "USB Serial/JTAG init failed");
    SET_FALSE_RETURN();
  }
}

/**
 * @brief USBSerialクラスのwriteメソッドの実装
 *
 * USB Serial/JTAGからデータを送信します。
 * 引数: data - 送信データ（文字列またはバイト配列）
 * 戻り値: 送信したバイト数、エラー時は-1
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_usb_serial_write(mrb_vm *vm, mrb_value *v, int argc) {
  SET_INT_RETURN(-1);  // Default to error (-1 bytes written)

  // Args: data (String or Array)
  if (argc < 1) {
    ESP_LOGE(TAG, "write: missing data argument");
    return;
  }

  const void *data_ptr = NULL;
  size_t data_len = 0;
  uint8_t *temp_buf = NULL;  // For array conversion

  if (v[1].tt == MRBC_TT_STRING) {
    // String data
    data_ptr = (const void *)mrbc_string_cstr(&v[1]);
    data_len = mrbc_string_size(&v[1]);
  } else if (v[1].tt == MRBC_TT_ARRAY) {
    // Array data (byte array)
    data_len = mrbc_array_size(&v[1]);
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
      mrb_value item = mrbc_array_get(&v[1], i);
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
  int written = drv_usb_serial_write(data_ptr, data_len);

  // Free temporary buffer if allocated
  if (temp_buf) {
    mrbc_free(vm, temp_buf);
  }

  // Set mrbc return value
  SET_INT_RETURN(written);
}

/**
 * @brief USBSerialクラスのreadメソッドの実装
 *
 * USB Serial/JTAGからデータを受信します。
 * 引数: length - 読み込むバイト数, [timeout_ms] -
 * タイムアウト（ミリ秒、デフォルト100ms） 戻り値:
 * 受信したデータ（文字列）、データなしの場合は空文字列、エラー時はnil
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_usb_serial_read(mrb_vm *vm, mrb_value *v, int argc) {
  SET_NIL_RETURN();  // Default to nil on error

  // Args: length, [timeout_ms]
  if (argc < 1 || v[1].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "read: invalid argument count or type");
    SET_NIL_RETURN();
    return;
  }

  int length = v[1].i;
  uint32_t timeout_ms = 100;  // Default timeout

  // Length check
  if (length <= 0) {
    mrbc_value result =
        mrbc_string_new_cstr(vm, "");  // Return empty string for 0 length
    SET_RETURN(result);
    return;
  }

  // Optional timeout argument
  if (argc >= 2 && v[2].tt == MRBC_TT_INTEGER) {
    int timeout_arg = v[2].i;
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
  int read_bytes = drv_usb_serial_read(buf, length, timeout_ms);

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
    ESP_LOGE(TAG, "USB Serial read failed (returned %d)", read_bytes);
    SET_NIL_RETURN();  // Return nil on error
  }

  // Free buffer
  mrbc_free(vm, buf);
}

/**
 * @brief USBSerialクラスのdeinitメソッドの実装
 *
 * USB Serial/JTAGを終了します。
 * 引数: なし
 * 戻り値: 成功時true、失敗時false
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_usb_serial_deinit(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // Default to failure

  // USB Serial/JTAGは引数不要
  if (argc != 0) {
    ESP_LOGW(TAG,
             "deinit: USB Serial/JTAG deinit takes no arguments (given %d)",
             argc);
  }

  // Call driver deinit function
  fn_t result = drv_usb_serial_deinit();

  if (result == kSuccess) {
    ESP_LOGI(TAG, "USB Serial/JTAG deinit succeeded");
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "USB Serial/JTAG deinit failed");
    SET_FALSE_RETURN();
  }
}

/**
 * @brief USBSerialクラスのavailableメソッドの実装
 *
 * USB Serial/JTAGの受信バッファにある読み出し可能なバイト数を返します。
 * 引数: なし
 * 戻り値: 読み出し可能なバイト数、エラー時は0
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_usb_serial_available(mrb_vm *vm, mrb_value *v, int argc) {
  SET_INT_RETURN(0);  // Default to 0 (error or no bytes)

  // USB Serial/JTAGは引数不要
  if (argc != 0) {
    ESP_LOGW(
        TAG,
        "available: USB Serial/JTAG available takes no arguments (given %d)",
        argc);
  }

  size_t available_bytes = 0;
  // Call driver get_available function
  fn_t result = drv_usb_serial_get_available(&available_bytes);

  if (result == kSuccess) {
    SET_INT_RETURN((int)available_bytes);
  } else {
    SET_INT_RETURN(0);  // Return 0 on failure
  }
}

/**
 * @brief USBSerialクラスのconnected?メソッドの実装
 *
 * USB Serial/JTAGが接続されているかを確認します。
 * 引数: なし
 * 戻り値: 接続時true、未接続時false
 *
 * @param vm mruby/c VMへのポインタ
 * @param v メソッド引数へのポインタ
 * @param argc 引数の数
 */
static void c_usb_serial_connected(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // Default to false

  // USB Serial/JTAGは引数不要
  if (argc != 0) {
    ESP_LOGW(
        TAG,
        "connected?: USB Serial/JTAG connected? takes no arguments (given %d)",
        argc);
  }

  // Call driver is_connected function
  bool connected = drv_usb_serial_is_connected();

  if (connected) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}
