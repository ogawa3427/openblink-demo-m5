/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.c
 * @brief Implementation of UART API for mruby/c
 *
 * Implements the UART class and its methods for the mruby/c VM,
 * providing functionality for serial communication on the ESP32 hardware.
 */
#include "uart.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../drv/uart.h"
#include "../lib/fn.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "mrubyc.h"

// Forward declarations for method implementations
static void c_uart_initialize(mrb_vm *vm, mrb_value *v, int argc);
static void c_uart_write(mrb_vm *vm, mrb_value *v, int argc);
static void c_uart_read(mrb_vm *vm, mrb_value *v, int argc);
static void c_uart_close(mrb_vm *vm, mrb_value *v, int argc);

// Data structure to store UART instance information
typedef struct {
  uart_port_t port;
  bool initialized;
} mrb_uart_t;

static const char *TAG = "API_UART";

/**
 * @brief Defines the UART class and methods for mruby/c
 *
 * Creates the UART class and registers methods for
 * initializing, sending, and receiving data over UART.
 *
 * @return kSuccess always
 */
fn_t api_uart_define(void) {
  mrb_class *class_uart;

  // Define UART class
  class_uart = mrbc_define_class(0, "UART", mrbc_class_object);

  // Register methods
  mrbc_define_method(0, class_uart, "initialize", c_uart_initialize);
  mrbc_define_method(0, class_uart, "write", c_uart_write);
  mrbc_define_method(0, class_uart, "read", c_uart_read);
  mrbc_define_method(0, class_uart, "close", c_uart_close);

  ESP_LOGI(TAG, "UART class defined");
  return kSuccess;
}

/**
 * @brief Ruby method: UART#initialize
 *
 * Initializes a UART instance with specified port, pins, and baud rate.
 * Usage: uart = UART.new(port, tx_pin, rx_pin, baud_rate)
 *
 * @param vm mruby/c VM
 * @param v method arguments array
 * @param argc number of arguments
 */
static void c_uart_initialize(mrb_vm *vm, mrb_value *v, int argc) {
  if (argc != 4 || v[1].tt != MRBC_TT_INTEGER || v[2].tt != MRBC_TT_INTEGER ||
      v[3].tt != MRBC_TT_INTEGER || v[4].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "Invalid arguments for UART.new");
    SET_FALSE_RETURN();
    return;
  }

  // Extract arguments
  uart_port_t port = (uart_port_t)v[1].i;
  gpio_num_t txd_pin = (gpio_num_t)v[2].i;
  gpio_num_t rxd_pin = (gpio_num_t)v[3].i;
  uint32_t baud_rate = (uint32_t)v[4].i;

  // Allocate UART data structure
  mrb_uart_t *uart = (mrb_uart_t *)mrbc_alloc(vm, sizeof(mrb_uart_t));
  if (uart == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for UART object");
    SET_FALSE_RETURN();
    return;
  }

  // Initialize UART driver
  fn_t result = drv_uart_init(port, txd_pin, rxd_pin, baud_rate);
  if (result != kSuccess) {
    mrbc_free(vm, uart);
    ESP_LOGE(TAG, "Failed to initialize UART driver");
    SET_FALSE_RETURN();
    return;
  }

  // Store UART port in the Ruby object
  uart->port = port;
  uart->initialized = true;

  // Attach the data structure to the Ruby object
  v[0].tt = MRBC_TT_DATA;
  v[0].data = uart;

  ESP_LOGI(TAG, "UART%d initialized from Ruby", port);
  SET_TRUE_RETURN();
}

/**
 * @brief Ruby method: UART#write
 *
 * Sends data over UART.
 * Usage: bytes_sent = uart.write(data)
 *
 * @param vm mruby/c VM
 * @param v method arguments array
 * @param argc number of arguments
 */
static void c_uart_write(mrb_vm *vm, mrb_value *v, int argc) {
  mrb_uart_t *uart = (mrb_uart_t *)v[0].data;
  if (uart == NULL || !uart->initialized || argc != 1) {
    ESP_LOGE(TAG, "Invalid UART object or arguments for write");
    SET_FALSE_RETURN();
    return;
  }

  // Check if argument is a string
  if (v[1].tt != MRBC_TT_STRING) {
    ESP_LOGE(TAG, "Argument must be a string");
    SET_FALSE_RETURN();
    return;
  }

  size_t written = 0;
  const uint8_t *data = (const uint8_t *)v[1].string->data;
  size_t len = v[1].string->len;

  // Write data to UART
  fn_t result = drv_uart_write(uart->port, data, len, &written);
  if (result != kSuccess) {
    ESP_LOGE(TAG, "Failed to write to UART%d", uart->port);
    SET_FALSE_RETURN();
    return;
  }

  // Return number of bytes written
  SET_INT_RETURN((int)written);
}

/**
 * @brief Ruby method: UART#read
 *
 * Reads data from UART.
 * Usage: data = uart.read(max_len, timeout_ms)
 *
 * @param vm mruby/c VM
 * @param v method arguments array
 * @param argc number of arguments
 */
static void c_uart_read(mrb_vm *vm, mrb_value *v, int argc) {
  mrb_uart_t *uart = (mrb_uart_t *)v[0].data;
  if (uart == NULL || !uart->initialized || argc < 1 || argc > 2) {
    ESP_LOGE(TAG, "Invalid UART object or arguments for read");
    SET_FALSE_RETURN();
    return;
  }

  // Extract max bytes to read
  if (v[1].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "First argument must be an integer");
    SET_FALSE_RETURN();
    return;
  }

  size_t len = (size_t)v[1].i;
  if (len <= 0) {
    ESP_LOGW(TAG, "Invalid read length: %d", (int)len);
    SET_NIL_RETURN();
    return;
  }

  // Extract timeout
  uint32_t timeout_ms = 0;
  if (argc == 2) {
    if (v[2].tt != MRBC_TT_INTEGER) {
      ESP_LOGE(TAG, "Timeout must be an integer");
      SET_FALSE_RETURN();
      return;
    }
    timeout_ms = (uint32_t)v[2].i;
  }

  // Allocate buffer for received data
  uint8_t *buffer = (uint8_t *)mrbc_alloc(vm, len + 1);
  if (buffer == NULL) {
    ESP_LOGE(TAG, "Failed to allocate memory for read buffer");
    SET_FALSE_RETURN();
    return;
  }

  // Read data from UART
  size_t bytes_read = 0;
  fn_t result = drv_uart_read(uart->port, buffer, len, timeout_ms, &bytes_read);

  // Handle read result
  if (result != kSuccess || bytes_read == 0) {
    mrbc_free(vm, buffer);
    SET_NIL_RETURN();
    return;
  }

  // Create Ruby string from received data
  buffer[bytes_read] = '\0';  // Null-terminate for string
  mrb_value str = mrbc_string_new_cstr(vm, (const char *)buffer);
  mrbc_free(vm, buffer);
  
  SET_RETURN(str);
}

/**
 * @brief Ruby method: UART#close
 *
 * Closes the UART instance and releases resources.
 * Usage: uart.close
 *
 * @param vm mruby/c VM
 * @param v method arguments array
 * @param argc number of arguments
 */
static void c_uart_close(mrb_vm *vm, mrb_value *v, int argc) {
  mrb_uart_t *uart = (mrb_uart_t *)v[0].data;
  if (uart == NULL) {
    ESP_LOGE(TAG, "Invalid UART object");
    SET_FALSE_RETURN();
    return;
  }

  if (!uart->initialized) {
    ESP_LOGW(TAG, "UART%d already closed", uart->port);
    SET_TRUE_RETURN();  // Not an error if already closed
    return;
  }

  // Deinitialize UART driver
  fn_t result = drv_uart_deinit(uart->port);
  uart->initialized = false;  // Mark as closed regardless of result

  if (result != kSuccess) {
    ESP_LOGE(TAG, "Failed to close UART%d", uart->port);
    SET_FALSE_RETURN();
    return;
  }

  ESP_LOGI(TAG, "UART%d closed from Ruby", uart->port);
  SET_TRUE_RETURN();
} 