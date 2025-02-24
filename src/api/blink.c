/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#include "blink.h"

#include "../../.pio/libdeps/m5stack-stamps3/mrubyc/src/mrubyc.h"
#include "../lib/fn.h"
#include "../main.h"

// **************************************************************************
// forward declaration
static void c_get_reload(mrb_vm *vm, mrb_value *v, int argc);

// **************************************************************************
// api_blink_define
fn_t api_blink_define(void) {
  mrb_class *class_blink;
  class_blink = mrbc_define_class(0, "Blink", mrbc_class_object);
  mrbc_define_method(0, class_blink, "req_reload?", c_get_reload);
  return kSuccess;
}

// **************************************************************************
// c_get_reload
static void c_get_reload(mrb_vm *vm, mrb_value *v, int argc) {
  SET_BOOL_RETURN(app_mrubyc_vm_get_reload());
}
