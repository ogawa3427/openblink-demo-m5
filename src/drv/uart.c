/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.c
 * @brief UART driver implementation
 *
 * Implements functions for initializing and controlling UART communication
 * on the ESP32 hardware.
 */
#include "uart.h"

#include <string.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RX_BUF_SIZE 256  // Default receive buffer size
static const char *TAG = "DRV_UART";

/**
 * @brief Initializes the UART driver
 *
 * @param port UART port number
 * @param txd_pin GPIO number for TXD
 * @param rxd_pin GPIO number for RXD
 * @param baud_rate Baud rate
 * @return kSuccess on successful initialization, kFailure otherwise
 */
fn_t drv_uart_init(uart_port_t port, gpio_num_t txd_pin, gpio_num_t rxd_pin,
                   uint32_t baud_rate) {
  // UART configuration
  uart_config_t uart_config = {
      .baud_rate = baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_DEFAULT,
  };

  // Error handling within the function
  esp_err_t err;

  // Install UART driver
  err = uart_driver_install(port, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(err));
    return kFailure;
  }

  // Configure UART parameters
  err = uart_param_config(port, &uart_config);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure UART parameters: %s",
             esp_err_to_name(err));
    uart_driver_delete(port);
    return kFailure;
  }

  // Set UART pins
  err = uart_set_pin(port, txd_pin, rxd_pin, UART_PIN_NO_CHANGE,
                     UART_PIN_NO_CHANGE);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(err));
    uart_driver_delete(port);
    return kFailure;
  }

  ESP_LOGI(TAG, "UART%d initialized: TX=%d, RX=%d, baud=%d", port, txd_pin,
           rxd_pin, baud_rate);

  return kSuccess;
}

/**
 * @brief Sends data over UART
 *
 * @param port UART port number
 * @param data Pointer to data buffer to send
 * @param len Number of bytes to send
 * @param written Pointer to variable to store the number of bytes sent
 * @return kSuccess on successful operation, kFailure otherwise
 */
fn_t drv_uart_write(uart_port_t port, const uint8_t *data, size_t len,
                    size_t *written) {
  if (data == NULL || written == NULL) {
    return kFailure;
  }

  int bytes_sent = uart_write_bytes(port, (const char *)data, len);
  if (bytes_sent < 0) {
    ESP_LOGE(TAG, "Failed to write to UART%d", port);
    *written = 0;
    return kFailure;
  }

  *written = (size_t)bytes_sent;
  ESP_LOGD(TAG, "UART%d sent %d bytes", port, bytes_sent);
  return kSuccess;
}

/**
 * @brief Receives data from UART
 *
 * @param port UART port number
 * @param buffer Pointer to buffer to store received data
 * @param len Maximum number of bytes to receive
 * @param timeout_ms Timeout in milliseconds (0 for no timeout)
 * @param read Pointer to variable to store the number of bytes received
 * @return kSuccess on successful operation, kFailure otherwise
 */
fn_t drv_uart_read(uart_port_t port, uint8_t *buffer, size_t len,
                   uint32_t timeout_ms, size_t *read) {
  if (buffer == NULL || read == NULL) {
    return kFailure;
  }

  TickType_t ticks_to_wait = 0;
  if (timeout_ms > 0) {
    ticks_to_wait = timeout_ms / portTICK_PERIOD_MS;
  }

  int bytes_read = uart_read_bytes(port, buffer, len, ticks_to_wait);
  if (bytes_read < 0) {
    ESP_LOGE(TAG, "Failed to read from UART%d", port);
    *read = 0;
    return kFailure;
  }

  *read = (size_t)bytes_read;
  if (bytes_read > 0) {
    ESP_LOGD(TAG, "UART%d received %d bytes", port, bytes_read);
  }
  return kSuccess;
}

/**
 * @brief Deinitializes the UART driver
 *
 * @param port UART port number
 * @return kSuccess on successful deinitialization, kFailure otherwise
 */
fn_t drv_uart_deinit(uart_port_t port) {
  esp_err_t err = uart_driver_delete(port);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to delete UART%d driver: %s", port,
             esp_err_to_name(err));
    return kFailure;
  }

  ESP_LOGI(TAG, "UART%d driver deinitialized", port);
  return kSuccess;
} 