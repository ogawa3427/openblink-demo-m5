/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file usb_serial.h
 * @brief Header for USB Serial API for mruby/c
 */
#ifndef API_USB_SERIAL_H_
#define API_USB_SERIAL_H_

#include "../lib/fn.h"

/**
 * @brief Define USBSerial class and methods for mruby/c
 *
 * Creates USBSerial class and registers init, write, read, deinit methods
 *
 * @return kSuccess always succeeds
 */
fn_t api_usb_serial_define(void);

#endif  // API_USB_SERIAL_H_
