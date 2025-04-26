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
#include "esp_log.h"

#define TAG "UART_DRV"
#define UART_BUF_SIZE 1024

// UART設定を保存するための構造体
static struct {
  bool initialized;
  int tx_pin;
  int rx_pin;
  int baud_rate;
} uart_config[3] = {
    {false, -1, -1, 0},  // UART0（使用しない）
    {false, -1, -1, 0},  // UART1
    {false, -1, -1, 0}   // UART2
};

/**
 * @brief UART初期化
 *
 * 指定されたUARTポートを初期化します
 *
 * @param uart_num UARTポート番号 (1または2)
 * @param tx_pin 送信ピン番号
 * @param rx_pin 受信ピン番号
 * @param baud_rate ボーレート
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_uart_init(uart_port_num_t uart_num, int tx_pin, int rx_pin,
                   int baud_rate) {
  // パラメータチェック
  if (uart_num <= 0 || uart_num > 2 || tx_pin < 0 || rx_pin < 0 ||
      baud_rate <= 0) {
    ESP_LOGE(TAG, "Invalid UART parameters");
    return kFailure;
  }

  // すでに初期化されていたら終了処理を行う
  if (uart_config[uart_num].initialized) {
    drv_uart_deinit(uart_num);
  }

  // UART設定
  uart_config_t config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DEFAULT_DATA_BITS,
      .parity = UART_DEFAULT_PARITY,
      .stop_bits = UART_DEFAULT_STOP_BITS,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 0,
      .source_clk = UART_SCLK_APB,
  };

  // UART初期化
  esp_err_t err = uart_param_config(uart_num, &config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "UART config failed: %d", err);
    return kFailure;
  }

  // ピン設定
  err = uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE,
                     UART_PIN_NO_CHANGE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "UART set pin failed: %d", err);
    return kFailure;
  }

  // ドライバインストール
  err = uart_driver_install(uart_num, UART_BUF_SIZE, UART_BUF_SIZE, 0, NULL, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "UART driver install failed: %d", err);
    return kFailure;
  }

  // 設定を保存
  uart_config[uart_num].initialized = true;
  uart_config[uart_num].tx_pin = tx_pin;
  uart_config[uart_num].rx_pin = rx_pin;
  uart_config[uart_num].baud_rate = baud_rate;

  ESP_LOGI(TAG, "UART%d initialized: TX=%d, RX=%d, Baud=%d", uart_num, tx_pin,
           rx_pin, baud_rate);
  return kSuccess;
}

/**
 * @brief UARTデータ送信
 *
 * 指定されたUARTポートからデータを送信します
 *
 * @param uart_num UARTポート番号
 * @param data 送信するデータ
 * @param len データの長さ
 * @return 送信されたバイト数、エラー時は負の値
 */
int drv_uart_write(uart_port_num_t uart_num, const uint8_t* data, size_t len) {
  // パラメータチェック
  if (uart_num <= 0 || uart_num > 2 || !data || len == 0) {
    ESP_LOGE(TAG, "Invalid UART write parameters");
    return -1;
  }

  // 初期化チェック
  if (!uart_config[uart_num].initialized) {
    ESP_LOGE(TAG, "UART%d not initialized", uart_num);
    return -1;
  }

  // データ送信
  int written = uart_write_bytes(uart_num, (const char*)data, len);
  if (written < 0) {
    ESP_LOGE(TAG, "UART write failed");
    return -1;
  }

  return written;
}

/**
 * @brief UARTデータ受信
 *
 * 指定されたUARTポートからデータを受信します
 *
 * @param uart_num UARTポート番号
 * @param data 受信データを格納するバッファ
 * @param len 受信するバイト数の最大値
 * @param timeout_ms タイムアウト（ミリ秒）
 * @return 受信したバイト数、エラー時は負の値
 */
int drv_uart_read(uart_port_num_t uart_num, uint8_t* data, size_t len,
                  uint32_t timeout_ms) {
  // パラメータチェック
  if (uart_num <= 0 || uart_num > 2 || !data || len == 0) {
    ESP_LOGE(TAG, "Invalid UART read parameters");
    return -1;
  }

  // 初期化チェック
  if (!uart_config[uart_num].initialized) {
    ESP_LOGE(TAG, "UART%d not initialized", uart_num);
    return -1;
  }

  // タイムアウト設定（ティック単位）
  TickType_t ticks_to_wait = timeout_ms / portTICK_PERIOD_MS;
  if (timeout_ms > 0 && ticks_to_wait == 0) {
    ticks_to_wait = 1;  // 最低1ティックは待つ
  }

  // データ受信
  int read_bytes = uart_read_bytes(uart_num, data, len, ticks_to_wait);
  if (read_bytes < 0) {
    ESP_LOGE(TAG, "UART read failed");
    return -1;
  }

  return read_bytes;
}

/**
 * @brief UART終了処理
 *
 * 指定されたUARTポートを終了します
 *
 * @param uart_num UARTポート番号
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_uart_deinit(uart_port_num_t uart_num) {
  // パラメータチェック
  if (uart_num <= 0 || uart_num > 2) {
    ESP_LOGE(TAG, "Invalid UART number");
    return kFailure;
  }

  // 初期化済みかチェック
  if (!uart_config[uart_num].initialized) {
    return kSuccess;  // すでに終了処理済み
  }

  // ドライバ削除
  esp_err_t err = uart_driver_delete(uart_num);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "UART driver delete failed: %d", err);
    return kFailure;
  }

  // 設定をリセット
  uart_config[uart_num].initialized = false;
  uart_config[uart_num].tx_pin = -1;
  uart_config[uart_num].rx_pin = -1;
  uart_config[uart_num].baud_rate = 0;

  ESP_LOGI(TAG, "UART%d deinitialized", uart_num);
  return kSuccess;
}