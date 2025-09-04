/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file blink_esp_now.h
 * @brief ESP-NOW driver implementation for OpenBlink
 *
 * Provides ESP-NOW functionality for low-latency P2P communication
 * between OpenBlink devices.
 */
#ifndef BLINK_ESP_NOW_H
#define BLINK_ESP_NOW_H

#include <stdbool.h>
#include <stdint.h>

#include "../lib/fn.h"
#include "esp_err.h"

/**
 * @brief ESP-NOW operating mode
 */
typedef enum {
  ESP_NOW_MODE_SEND = 0,  ///< Send mode only
  ESP_NOW_MODE_RECV = 1,  ///< Receive mode only
  ESP_NOW_MODE_BOTH = 2   ///< Both send and receive
} esp_now_mode_t;

/**
 * @brief ESP-NOW data structure
 */
typedef struct {
  int counter_value;   ///< Counter value
  uint32_t timestamp;  ///< Timestamp in milliseconds
} esp_now_data_t;

/**
 * @brief ESP-NOW status
 */
typedef enum {
  ESP_NOW_STATUS_READY = 0,    ///< Ready for operation
  ESP_NOW_STATUS_SENDING = 1,  ///< Currently sending
  ESP_NOW_STATUS_ERROR = 2     ///< Error state
} esp_now_status_t;

// Function prototypes
fn_t drv_esp_now_init(esp_now_mode_t mode);
fn_t drv_esp_now_deinit(void);
fn_t drv_esp_now_send_data(int value);
fn_t drv_esp_now_add_peer(const uint8_t *mac_addr);
fn_t drv_esp_now_remove_peer(const uint8_t *mac_addr);
esp_now_status_t drv_esp_now_get_status(void);
int drv_esp_now_get_received_counter(void);
int drv_esp_now_get_counter(void);
fn_t drv_esp_now_set_mode(esp_now_mode_t mode);

#endif  // BLINK_ESP_NOW_H