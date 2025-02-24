/*
 * Copyright (c) 2018 Workaround GmbH.
 * Copyright (c) 2017 Intel Corporation.
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright (c) 2018 Google LLC.
 * Copyright (c) 2022 Meta
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/** @file
 * @brief CRC computation function
 */

#ifndef ZEPHYR_INCLUDE_SYS_CRC_H_
#define ZEPHYR_INCLUDE_SYS_CRC_H_

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic function for computing a CRC-16 with input and output
 *        reflection.
 *
 * Compute CRC-16 by passing in the address of the input, the input length
 * and polynomial used in addition to the initial value. This is O(n*8) where n
 * is the length of the buffer provided. Both input and output are reflected.
 *
 * @note If you are planning to use a CRC based on poly 0x1012 the function
 * crc16_ccitt() is faster and thus recommended over this one.
 *
 * The following checksums can, among others, be calculated by this function,
 * depending on the value provided for the initial seed and the value the final
 * calculated CRC is XORed with:
 *
 * - CRC-16/ANSI, CRC-16/MODBUS, CRC-16/USB, CRC-16/IBM
 *   https://reveng.sourceforge.io/crc-catalogue/16.htm#crc.cat.crc-16-modbus
 *   poly: 0x8005 (0xA001) initial seed: 0xffff, xor output: 0x0000
 *
 * @param poly The polynomial to use omitting the leading x^16
 *             coefficient. Important: please reflect the poly. For example,
 *             use 0xA001 instead of 0x8005 for CRC-16-MODBUS.
 * @param seed Initial value for the CRC computation
 * @param src Input bytes for the computation
 * @param len Length of the input in bytes
 *
 * @return The computed CRC16 value (without any XOR applied to it)
 */
uint16_t crc16_reflect(uint16_t poly, uint16_t seed, const uint8_t *src,
                       size_t len);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
