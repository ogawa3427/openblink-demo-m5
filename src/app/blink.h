/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#ifndef APP_BLINK_H
#define APP_BLINK_H

#include <stddef.h>
#include <stdint.h>

#define BLINK_MAX_BYTECODE_SIZE (15 * 1024)

size_t blink_load(uint8_t *const data, const size_t kLength);
size_t blink_store(const uint8_t *const kData, const size_t kLength);
size_t blink_get_data_length(void);
int blink_delete(void);

#endif
