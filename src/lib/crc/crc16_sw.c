/*
 * Copyright (c) 2017 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <stdint.h>

uint16_t crc16_reflect(uint16_t poly, uint16_t seed, const uint8_t *src,
                       size_t len) {
  uint16_t crc = seed;
  size_t i, j;

  for (i = 0; i < len; i++) {
    crc ^= (uint16_t)src[i];

    for (j = 0; j < 8; j++) {
      if (crc & 0x0001UL) {
        crc = (crc >> 1U) ^ poly;
      } else {
        crc = crc >> 1U;
      }
    }
  }

  return crc;
}
