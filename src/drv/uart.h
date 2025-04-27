/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.h
 * @brief Header for UART driver
 */
#ifndef DRV_UART_H_
#define DRV_UART_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../lib/fn.h"
#include "driver/uart.h"  // For uart_port_t

// Define uart_port_num_t based on ESP-IDF's uart_port_t for clarity
typedef uart_port_t uart_port_num_t;

// Remove definition of uart_port_num_t if it conflicts or is redundant
// typedef enum { UART_NUM_0 = 0, UART_NUM_1, UART_NUM_2 } uart_port_num_t;

/**
 * @brief Initialize UART port
 *
 * @param uart_num UART port number (UART_NUM_0, UART_NUM_1, or UART_NUM_2)
 * @param tx_pin TX pin number (use GPIO_NUM_NC if not used)
 * @param rx_pin RX pin number (use GPIO_NUM_NC if not used)
 * @param baud_rate Baud rate
 * @param rx_buffer_size RX ring buffer size
 * @param tx_buffer_size TX ring buffer size (0 for no buffer)
 * @return kSuccess on success, kFailure on failure
 */
fn_t drv_uart_init(uart_port_num_t uart_num, int tx_pin, int rx_pin,
                   int baud_rate, int rx_buffer_size, int tx_buffer_size);

/**
 * @brief Write data to UART port
 *
 * @param uart_num UART port number
 * @param data Pointer to data buffer
 * @param len Length of data to write
 * @return Number of bytes written, or -1 on error
 */
int drv_uart_write(uart_port_num_t uart_num, const void* data, size_t len);

/**
 * @brief Read data from UART port
 *
 * @param uart_num UART port number
 * @param buf Pointer to buffer to store read data
 * @param len Maximum number of bytes to read
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes read (can be 0 if timeout), or -1 on error
 */
int drv_uart_read(uart_port_num_t uart_num, void* buf, size_t len,
                  uint32_t timeout_ms);

/**
 * @brief Get the number of bytes available in the UART RX buffer
 *
 * @param uart_num UART port number
 * @param available_bytes Pointer to store the result
 * @return kSuccess on success, kFailure if port not initialized or error
 */
fn_t drv_uart_get_available(uart_port_num_t uart_num, size_t* available_bytes);

/**
 * @brief Deinitialize UART port
 *
 * @param uart_num UART port number
 * @return kSuccess on success, kFailure on failure
 */
fn_t drv_uart_deinit(uart_port_num_t uart_num);

#endif /* DRV_UART_H_ */