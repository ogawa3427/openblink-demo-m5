/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

#include "lib/fn.h"

fn_t app_mrubyc_vm_set_reload(void);
bool app_mrubyc_vm_get_reload(void);

#endif
