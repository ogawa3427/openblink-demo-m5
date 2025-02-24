/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#include "input.h"

#include <M5Unified.h>
#include <stdbool.h>

#include "../../mrubyc/src/mrubyc.h"
#include "../lib/fn.h"

// **************************************************************************
// forward declaration
static void c_get_sw_pressed(mrb_vm *vm, mrb_value *v, int argc);
static void c_get_sw_released(mrb_vm *vm, mrb_value *v, int argc);

// **************************************************************************
// api_input_define
fn_t api_input_define(void) {
  mrb_class *class_input;
  class_input = mrbc_define_class(0, "Input", mrbc_class_object);
  mrbc_define_method(0, class_input, "pressed?", c_get_sw_pressed);
  mrbc_define_method(0, class_input, "released?", c_get_sw_released);
  return kSuccess;
}

// **************************************************************************
// c_get_sw_pressed
static void c_get_sw_pressed(mrb_vm *vm, mrb_value *v, int argc) {
  M5.update();
  if (true == M5.BtnA.isPressed()) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}

// **************************************************************************
// c_get_sw_released
static void c_get_sw_released(mrb_vm *vm, mrb_value *v, int argc) {
  M5.update();
  if (true == M5.BtnA.isReleased()) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}
