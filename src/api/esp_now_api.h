/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file esp_now_api.h
 * @brief ESP-NOW API header for mruby/c
 *
 * Provides the ESP-NOW class definition function for the mruby/c VM.
 */
#ifndef API_ESP_NOW_API_H
#define API_ESP_NOW_API_H

#include "../lib/fn.h"

/**
 * @brief Defines the ESPNow class and methods for mruby/c
 *
 * @return kSuccess always
 */
fn_t api_esp_now_define(void);

#endif  // API_ESP_NOW_API_H