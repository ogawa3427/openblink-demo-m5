/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.h
 * @brief UART driver interface
 *
 * Provides functions for initializing and controlling UART communication
 * on the ESP32 hardware.
 */
#ifndef DRV_UART_H
#define DRV_UART_H

#include <stdint.h>

#include "../lib/fn.h"
#include "driver/gpio.h"
#include "driver/uart.h"

/**
 * @brief Initializes the UART driver
 *
 * Sets up the UART peripheral with specified parameters.
 *
 * @param port UART port number (UART_NUM_0, UART_NUM_1, etc.)
 * @param txd_pin GPIO number for TXD
 * @param rxd_pin GPIO number for RXD
 * @param baud_rate Baud rate
 * @return kSuccess on successful initialization, kFailure otherwise
 */
fn_t drv_uart_init(uart_port_t port, gpio_num_t txd_pin, gpio_num_t rxd_pin,
                   uint32_t baud_rate);

/**
 * @brief Sends data over UART
 *
 * @param port UART port number
 * @param data Pointer to data buffer to send
 * @param len Number of bytes to send
 * @param written Pointer to variable to store the number of bytes sent
 * @return kSuccess on successful operation, kFailure otherwise
 */
fn_t drv_uart_write(uart_port_t port, const uint8_t* data, size_t len,
                    size_t* written);

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
fn_t drv_uart_read(uart_port_t port, uint8_t* buffer, size_t len,
                   uint32_t timeout_ms, size_t* read);

/**
 * @brief Deinitializes the UART driver
 *
 * @param port UART port number
 * @return kSuccess on successful deinitialization, kFailure otherwise
 */
fn_t drv_uart_deinit(uart_port_t port);

#endif  // DRV_UART_H