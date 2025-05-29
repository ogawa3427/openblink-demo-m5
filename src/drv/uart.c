/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.c
 * @brief UART driver implementation
 *
 * Implements functions for controlling UART communication on ESP32.
 */
#include "uart.h"

#include "driver/uart.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"  // Required for pdMS_TO_TICKS
#include "freertos/task.h"      // Required for pdMS_TO_TICKS

#define TAG "UART_DRV"
#define DEBUG_UART 1  // デバッグログの有効/無効を切り替える

// デバッグログマクロ
#if DEBUG_UART
#define UART_DEBUG(fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
#define UART_PERF(fmt, ...) ESP_LOGI(TAG, "[PERF] " fmt, ##__VA_ARGS__)
#else
#define UART_DEBUG(fmt, ...)
#define UART_PERF(fmt, ...)
#endif

// UART設定と初期化状態を保存するための構造体
// UART_NUM_MAX を使用して ESP32 の全ポートに対応
static struct {
  bool initialized;
  // Keep track of pins/baud for logging or potential re-init?
  // int tx_pin;
  // int rx_pin;
  // int baud_rate;
} uart_status[UART_NUM_MAX] = {0};  // Initialize all to false

/**
 * @brief Initialize UART port
 */
fn_t drv_uart_init(uart_port_num_t uart_num, int tx_pin, int rx_pin,
                   int baud_rate, int rx_buffer_size, int tx_buffer_size) {
  UART_DEBUG(
      "UART init called: port=%d, tx=%d, rx=%d, baud=%d, rx_buf=%d, tx_buf=%d",
      uart_num, tx_pin, rx_pin, baud_rate, rx_buffer_size, tx_buffer_size);

  // パラメータチェック (ポート番号、ピン、ボーレート)
  // Note: ESP-IDF allows GPIO_NUM_NC for pins
  if (uart_num < 0 || uart_num >= UART_NUM_MAX || baud_rate <= 0) {
    ESP_LOGE(TAG, "Invalid UART parameters (port: %d, baud: %d)", uart_num,
             baud_rate);
    return kFailure;
  }

  // すでに初期化されていたら終了処理を行う (ESP-IDF driver handles this? Let's
  // do it explicitly)
  if (uart_status[uart_num].initialized) {
    ESP_LOGW(TAG, "UART%d already initialized. Deinitializing first.",
             uart_num);
    // drv_uart_deinit should handle driver deletion
    if (drv_uart_deinit(uart_num) != kSuccess) {
      ESP_LOGE(TAG, "Failed to deinitialize UART%d before re-init.", uart_num);
      // Continue? Or return failure? Let's return failure.
      return kFailure;
    }
    // Ensure status is updated after deinit
    uart_status[uart_num].initialized = false;
  }

  // UART設定
  uart_config_t config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,  // Common defaults
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,  // Or UART_SCLK_APB
  };
  UART_DEBUG("UART config: data_bits=%d, parity=%d, stop_bits=%d, flow_ctrl=%d",
             config.data_bits, config.parity, config.stop_bits,
             config.flow_ctrl);

  // UARTパラメータ設定
  ESP_ERROR_CHECK(uart_param_config(uart_num, &config));

  // ピン設定 (GPIO_NUM_NC を受け入れる)
  ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));

  // ドライバインストール (バッファサイズを引数から取得)
  // Queue size, interrupt flags are set to 0/default for now
  ESP_ERROR_CHECK(uart_driver_install(uart_num, rx_buffer_size, tx_buffer_size,
                                      0, NULL, 0));

  // 状態を保存
  uart_status[uart_num].initialized = true;
  // Store other params if needed
  // uart_status[uart_num].tx_pin = tx_pin;
  // uart_status[uart_num].rx_pin = rx_pin;
  // uart_status[uart_num].baud_rate = baud_rate;

  ESP_LOGI(TAG,
           "UART%d initialized: TX=%d, RX=%d, Baud=%d, RX_buf=%d, TX_buf=%d",
           uart_num, tx_pin, rx_pin, baud_rate, rx_buffer_size, tx_buffer_size);
  return kSuccess;
}

/**
 * @brief Write data to UART port
 */
int drv_uart_write(uart_port_num_t uart_num, const void* data, size_t len) {
  UART_DEBUG("UART write: port=%d, len=%zu", uart_num, len);
  TickType_t start_time = xTaskGetTickCount();

  // パラメータチェック
  if (uart_num < 0 || uart_num >= UART_NUM_MAX || !data || len == 0) {
    ESP_LOGE(TAG, "Invalid UART write parameters");
    return -1;
  }

  // 初期化チェック
  if (!uart_status[uart_num].initialized) {
    ESP_LOGE(TAG, "UART%d not initialized for write", uart_num);
    return -1;
  }

  // データ送信 (uart_write_bytes expects const char*)
  int written = uart_write_bytes(uart_num, (const char*)data, len);
  TickType_t end_time = xTaskGetTickCount();
  UART_PERF("UART%d write took %lu ms for %d bytes", uart_num,
            (end_time - start_time) * portTICK_PERIOD_MS, written);

  if (written < 0) {
    ESP_LOGE(TAG, "UART%d write failed", uart_num);
    // Return ESP-IDF error code? Or just -1?
    return -1;
  } else if (written != len) {
    ESP_LOGW(TAG, "UART%d write incomplete: wrote %d of %d bytes", uart_num,
             written, len);
    // Return written count anyway
  }

  return written;
}

/**
 * @brief Read data from UART port
 */
int drv_uart_read(uart_port_num_t uart_num, void* buf, size_t len,
                  uint32_t timeout_ms) {
  UART_DEBUG("UART read: port=%d, len=%zu, timeout=%lu ms", uart_num, len,
             timeout_ms);
  TickType_t start_time = xTaskGetTickCount();

  // パラメータチェック
  if (uart_num < 0 || uart_num >= UART_NUM_MAX || !buf || len == 0) {
    ESP_LOGE(TAG, "Invalid UART read parameters");
    return -1;
  }

  // 初期化チェック
  if (!uart_status[uart_num].initialized) {
    ESP_LOGE(TAG, "UART%d not initialized for read", uart_num);
    return -1;
  }

  // タイムアウト設定（ティック単位）
  // 無限待機は許可しない
  TickType_t ticks_to_wait = pdMS_TO_TICKS(timeout_ms);
  if (ticks_to_wait == 0) {
    ticks_to_wait = 1;  // 最小待機時間を1ティックに設定
  }

  // データ受信 (uart_read_bytes expects uint8_t* for buffer)
  int read_bytes = uart_read_bytes(uart_num, (uint8_t*)buf, len, ticks_to_wait);
  TickType_t end_time = xTaskGetTickCount();
  UART_PERF("UART%d read took %lu ms, got %d bytes", uart_num,
            (end_time - start_time) * portTICK_PERIOD_MS, read_bytes);

  if (read_bytes < 0) {
    ESP_LOGE(TAG, "UART%d read failed (error code: %d)", uart_num, read_bytes);
    return -1;
  }

  return read_bytes;
}

/**
 * @brief Read data from UART until delimiter character is found or buffer is
 * full
 */
int drv_uart_read_until(uart_port_num_t uart_num, void* buf, size_t len,
                        char delimiter, uint32_t timeout_ms) {
  UART_DEBUG(
      "UART read_until: port=%d, len=%zu, delimiter='%c', timeout=%lu ms",
      uart_num, len, delimiter, timeout_ms);
  TickType_t start_time = xTaskGetTickCount();
  TickType_t timeout_end = start_time + pdMS_TO_TICKS(timeout_ms);

  // パラメータチェック
  if (uart_num < 0 || uart_num >= UART_NUM_MAX || !buf || len == 0) {
    ESP_LOGE(TAG, "Invalid UART read_until parameters");
    return -1;
  }

  // 初期化チェック
  if (!uart_status[uart_num].initialized) {
    ESP_LOGE(TAG, "UART%d not initialized for read_until", uart_num);
    return -1;
  }

  uint8_t* buffer = (uint8_t*)buf;
  size_t total_read = 0;

  // バッファサイズが1バイトの場合は通常のreadを使用
  if (len == 1) {
    return drv_uart_read(uart_num, buf, 1, timeout_ms);
  }

  // 一度に最大128バイト読み込む（ESP32のFIFOサイズに合わせる）
  const size_t chunk_size = 128;
  uint8_t temp_buf[chunk_size];

  while (total_read < len) {
    // 残り時間を計算
    TickType_t current_time = xTaskGetTickCount();
    if (current_time >= timeout_end) {
      break;  // タイムアウト
    }

    // 読み込むサイズを計算
    size_t remaining = len - total_read;
    size_t to_read = (remaining < chunk_size) ? remaining : chunk_size;

    // データ読み込み
    int read_bytes = uart_read_bytes(uart_num, temp_buf, to_read, 1);
    UART_DEBUG("UART read_until chunk: read=%d bytes", read_bytes);

    if (read_bytes <= 0) {
      break;  // エラーまたはタイムアウト
    }

    // デリミタを探す
    for (int i = 0; i < read_bytes; i++) {
      buffer[total_read++] = temp_buf[i];
      if (temp_buf[i] == (uint8_t)delimiter) {
        return total_read;  // デリミタを見つけた
      }
    }
  }

  TickType_t actual_end_time = xTaskGetTickCount();
  UART_PERF("UART%d read_until took %lu ms, total_read=%d", uart_num,
            (actual_end_time - start_time) * portTICK_PERIOD_MS, total_read);

  return total_read;
}

/**
 * @brief Get the number of bytes available in the UART RX buffer
 */
fn_t drv_uart_get_available(uart_port_num_t uart_num, size_t* available_bytes) {
  UART_DEBUG("UART get_available: port=%d", uart_num);

  if (uart_num < 0 || uart_num >= UART_NUM_MAX || !available_bytes) {
    ESP_LOGE(TAG, "Invalid parameters for drv_uart_get_available");
    return kFailure;
  }

  // 初期化チェック
  if (!uart_status[uart_num].initialized) {
    ESP_LOGW(TAG, "UART%d not initialized for get_available", uart_num);
    *available_bytes = 0;
    return kFailure;  // Or return kSuccess with 0 bytes? Let's return failure.
  }

  ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_num, available_bytes));
  UART_DEBUG("UART get_available result: available=%d bytes", *available_bytes);
  return kSuccess;  // ESP_ERROR_CHECK ensures err == ESP_OK if we reach here
}

/**
 * @brief Deinitialize UART port
 */
fn_t drv_uart_deinit(uart_port_num_t uart_num) {
  UART_DEBUG("UART deinit: port=%d", uart_num);

  // パラメータチェック
  if (uart_num < 0 || uart_num >= UART_NUM_MAX) {
    ESP_LOGE(TAG, "Invalid UART number %d for deinit", uart_num);
    return kFailure;
  }

  // 初期化済みかチェック
  if (!uart_status[uart_num].initialized) {
    ESP_LOGI(TAG, "UART%d already deinitialized.", uart_num);
    return kSuccess;  // Already deinitialized is success
  }

  // ドライバ削除
  // Note: This will also free the buffers allocated by uart_driver_install
  ESP_ERROR_CHECK(uart_driver_delete(uart_num));

  // 状態をリセット
  uart_status[uart_num].initialized = false;
  // Reset other params if stored
  // uart_status[uart_num].tx_pin = -1;
  // uart_status[uart_num].rx_pin = -1;
  // uart_status[uart_num].baud_rate = 0;

  ESP_LOGI(TAG, "UART%d deinitialized", uart_num);
  return kSuccess;
}