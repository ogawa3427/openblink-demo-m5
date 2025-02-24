/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#include "init.h"

#include <M5Unified.h>

#include "../lib/fn.h"
extern "C" {
#include "../drv/ble.h"
#include "../drv/led.h"
}

fn_t app_init(void) {
  M5.begin();
  drv_led_init();
  ble_init();
  return kSuccess;
}
