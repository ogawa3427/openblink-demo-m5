/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file init.cpp
 * @brief Application initialization implementation
 *
 * Implements the initialization of application components including
 * M5Stack hardware, LED driver, and BLE functionality.
 */
#include "init.h"

#include <M5Unified.h>

#include "../lib/fn.h"
extern "C" {
#include "../drv/ble.h"
#include "../drv/led.h"
}

/**
 * @brief Initializes the application components
 *
 * Initializes the M5Stack hardware, LED driver, and BLE functionality.
 *
 * @return kSuccess always
 */
fn_t app_init(void) {
  M5.begin();
  drv_led_init();
  ble_init();
  return kSuccess;
}
