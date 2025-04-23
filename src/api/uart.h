/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.h
 * @brief API interface for UART control in mruby/c
 *
 * Defines the interface for the UART class in mruby/c, which provides
 * methods for serial communication on the ESP32 hardware.
 */
#ifndef API_UART_H
#define API_UART_H

#include "../lib/fn.h"

/**
 * @brief Defines the UART class and methods for mruby/c
 *
 * Creates the UART class and registers the methods for
 * initializing, sending, and receiving data over UART.
 *
 * @return kSuccess always
 */
fn_t api_uart_define(void);

#endif  // API_UART_H