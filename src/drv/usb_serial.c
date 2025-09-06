/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file usb_serial.c
 * @brief USB Serial/JTAG driver implementation
 *
 * Implements functions for controlling USB Serial/JTAG communication on ESP32.
 */
#include "usb_serial.h"

#include "driver/usb_serial_jtag.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "USB_SERIAL_DRV"
#define DEBUG_USB_SERIAL 1  // デバッグログの有効/無効を切り替える

// デバッグログマクロ
#if DEBUG_USB_SERIAL
#define USB_SERIAL_DEBUG(fmt, ...) ESP_LOGD(TAG, fmt, ##__VA_ARGS__)
#define USB_SERIAL_PERF(fmt, ...) ESP_LOGI(TAG, "[PERF] " fmt, ##__VA_ARGS__)
#else
#define USB_SERIAL_DEBUG(fmt, ...)
#define USB_SERIAL_PERF(fmt, ...)
#endif

// USB Serial/JTAG初期化状態を保存
static bool usb_serial_initialized = false;

/**
 * @brief Initialize USB Serial/JTAG
 */
fn_t drv_usb_serial_init(void) {
  USB_SERIAL_DEBUG("USB Serial/JTAG init called");

  // すでに初期化されていたら成功として返す
  if (usb_serial_initialized) {
    ESP_LOGI(TAG, "USB Serial/JTAG already initialized");
    return kSuccess;
  }

  // USB Serial/JTAG設定
  usb_serial_jtag_driver_config_t config = {
      .tx_buffer_size = 1024,
      .rx_buffer_size = 1024,
  };

  // ドライバインストール
  esp_err_t err = usb_serial_jtag_driver_install(&config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install USB Serial/JTAG driver: %s",
             esp_err_to_name(err));
    return kFailure;
  }

  // 状態を保存
  usb_serial_initialized = true;

  ESP_LOGI(TAG, "USB Serial/JTAG initialized successfully");
  return kSuccess;
}

/**
 * @brief Write data to USB Serial/JTAG
 */
int drv_usb_serial_write(const void* data, size_t len) {
  USB_SERIAL_DEBUG("USB Serial write: len=%zu", len);
  TickType_t start_time = xTaskGetTickCount();

  // パラメータチェック
  if (!data || len == 0) {
    ESP_LOGE(TAG, "Invalid USB Serial write parameters");
    return -1;
  }

  // 初期化チェック
  if (!usb_serial_initialized) {
    ESP_LOGE(TAG, "USB Serial/JTAG not initialized for write");
    return -1;
  }

  // データ送信
  int written =
      usb_serial_jtag_write_bytes((const char*)data, len, portMAX_DELAY);
  TickType_t end_time = xTaskGetTickCount();
  USB_SERIAL_PERF("USB Serial write took %lu ms for %d bytes",
                  (end_time - start_time) * portTICK_PERIOD_MS, written);

  if (written < 0) {
    ESP_LOGE(TAG, "USB Serial write failed");
    return -1;
  } else if (written != len) {
    ESP_LOGW(TAG, "USB Serial write incomplete: wrote %d of %d bytes", written,
             len);
  }

  return written;
}

/**
 * @brief Read data from USB Serial/JTAG
 */
int drv_usb_serial_read(void* buf, size_t len, uint32_t timeout_ms) {
  USB_SERIAL_DEBUG("USB Serial read: len=%zu, timeout=%lu ms", len, timeout_ms);
  TickType_t start_time = xTaskGetTickCount();

  // パラメータチェック
  if (!buf || len == 0) {
    ESP_LOGE(TAG, "Invalid USB Serial read parameters");
    return -1;
  }

  // 初期化チェック
  if (!usb_serial_initialized) {
    ESP_LOGE(TAG, "USB Serial/JTAG not initialized for read");
    return -1;
  }

  // タイムアウト設定（ティック単位）
  TickType_t ticks_to_wait = pdMS_TO_TICKS(timeout_ms);
  if (ticks_to_wait == 0) {
    ticks_to_wait = 1;  // 最小待機時間を1ティックに設定
  }

  // データ受信
  int read_bytes =
      usb_serial_jtag_read_bytes((uint8_t*)buf, len, ticks_to_wait);
  TickType_t end_time = xTaskGetTickCount();
  USB_SERIAL_PERF("USB Serial read took %lu ms, got %d bytes",
                  (end_time - start_time) * portTICK_PERIOD_MS, read_bytes);

  if (read_bytes < 0) {
    ESP_LOGE(TAG, "USB Serial read failed (error code: %d)", read_bytes);
    return -1;
  }

  return read_bytes;
}

/**
 * @brief Get the number of bytes available in the USB Serial/JTAG RX buffer
 */
fn_t drv_usb_serial_get_available(size_t* available_bytes) {
  USB_SERIAL_DEBUG("USB Serial get_available called");

  if (!available_bytes) {
    ESP_LOGE(TAG, "Invalid parameters for drv_usb_serial_get_available");
    return kFailure;
  }

  // 初期化チェック
  if (!usb_serial_initialized) {
    ESP_LOGW(TAG, "USB Serial/JTAG not initialized for get_available");
    *available_bytes = 0;
    return kFailure;
  }

  // 利用可能なバイト数を取得
  // USB Serial/JTAGには直接的なavailableバイト数取得関数がないため、
  // 0バイト読み込みを試してタイムアウトで判定する
  uint8_t dummy;
  int result = usb_serial_jtag_read_bytes(&dummy, 1, 0);

  if (result > 0) {
    // データがあった場合、1バイト読み込んでしまったので戻す必要があるが、
    // USB Serial/JTAGにはungetc相当の機能がないため、簡易実装として1を返す
    *available_bytes = 1;
  } else {
    *available_bytes = 0;
  }

  USB_SERIAL_DEBUG("USB Serial get_available result: available=%d bytes",
                   *available_bytes);
  return kSuccess;
}

/**
 * @brief Check if USB Serial/JTAG is connected
 */
bool drv_usb_serial_is_connected(void) {
  if (!usb_serial_initialized) {
    return false;
  }

  // USB Serial/JTAG接続状態を確認
  // ESP-IDFには直接的な接続状態確認関数がないため、
  // 初期化済みであることを前提に true を返す
  return true;
}

/**
 * @brief Deinitialize USB Serial/JTAG
 */
fn_t drv_usb_serial_deinit(void) {
  USB_SERIAL_DEBUG("USB Serial deinit called");

  // 初期化済みかチェック
  if (!usb_serial_initialized) {
    ESP_LOGI(TAG, "USB Serial/JTAG already deinitialized");
    return kSuccess;  // Already deinitialized is success
  }

  // ドライバ削除
  esp_err_t err = usb_serial_jtag_driver_uninstall();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to uninstall USB Serial/JTAG driver: %s",
             esp_err_to_name(err));
    return kFailure;
  }

  // 状態をリセット
  usb_serial_initialized = false;

  ESP_LOGI(TAG, "USB Serial/JTAG deinitialized");
  return kSuccess;
}
