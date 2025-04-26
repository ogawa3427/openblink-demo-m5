/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.h
 * @brief API interface for UART control in mruby/c
 *
 * Defines the interface for the UART class in mruby/c, which provides
 * methods for controlling UART communication on the ESP32.
 */
#ifndef API_UART_H
#define API_UART_H

#include "../lib/fn.h"

/**
 * @brief Defines the UART class and methods for mruby/c
 *
 * Creates the UART class and registers methods for UART control
 * which allows Ruby code to communicate via UART.
 *
 * @return kSuccess always
 */
fn_t api_uart_define(void);

#endif  // API_UART_H