/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#ifndef LIB_FN_H
#define LIB_FN_H

typedef enum {
  kUndetermined = 0x00000000U,
  kInitialized = 0x0F0F0F0FU,
  kSuccess = 0x5A5A5A5AU,
  kFailure = 0xFFFFFFFFU
} fn_t;

#endif
