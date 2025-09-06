/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file usb_serial.h
 * @brief Header for USB Serial/JTAG driver
 */
#ifndef DRV_USB_SERIAL_H_
#define DRV_USB_SERIAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../lib/fn.h"

/**
 * @brief Initialize USB Serial/JTAG
 *
 * @return kSuccess on success, kFailure on failure
 */
fn_t drv_usb_serial_init(void);

/**
 * @brief Write data to USB Serial/JTAG
 *
 * @param data Pointer to data buffer
 * @param len Length of data to write
 * @return Number of bytes written, or -1 on error
 */
int drv_usb_serial_write(const void* data, size_t len);

/**
 * @brief Read data from USB Serial/JTAG
 *
 * @param buf Pointer to buffer to store read data
 * @param len Maximum number of bytes to read
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes read (can be 0 if timeout), or -1 on error
 */
int drv_usb_serial_read(void* buf, size_t len, uint32_t timeout_ms);

/**
 * @brief Get the number of bytes available in the USB Serial/JTAG RX buffer
 *
 * @param available_bytes Pointer to store the number of available bytes
 * @return kSuccess on success, kFailure on failure
 */
fn_t drv_usb_serial_get_available(size_t* available_bytes);

/**
 * @brief Deinitialize USB Serial/JTAG
 *
 * @return kSuccess on success, kFailure on failure
 */
fn_t drv_usb_serial_deinit(void);

/**
 * @brief Check if USB Serial/JTAG is connected
 *
 * @return true if connected, false otherwise
 */
bool drv_usb_serial_is_connected(void);

#endif  // DRV_USB_SERIAL_H_
