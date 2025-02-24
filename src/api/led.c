/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#include "led.h"

#include <stdbool.h>
#include <stdint.h>

#include "../../.pio/libdeps/m5stack-stamps3/mrubyc/src/mrubyc.h"
#include "../drv/led.h"
#include "../lib/fn.h"

// **************************************************************************
// forward declarations
static void c_set_led(mrb_vm *vm, mrb_value *v, int argc);

// **************************************************************************
// api_led_define
fn_t api_led_define(void) {
  mrb_class *class_led;
  class_led = mrbc_define_class(0, "LED", mrbc_class_object);
  mrbc_define_method(0, class_led, "set", c_set_led);
  return kSuccess;
}

// **************************************************************************
// c_set_led
static void c_set_led(mrb_vm *vm, mrb_value *v, int argc) {
  uint8_t rgb[3] = {0};
  SET_FALSE_RETURN();

  do {
    if (MRBC_TT_ARRAY == v[1].tt) {
      if (3 != v[1].array->n_stored) break;
      for (size_t i = 0; i < 3; i++) {
        rgb[i] = (uint8_t)v[1].array->data[i].i;
      }
    } else {
      break;
    }
  } while (0);

  if (kSuccess == drv_led_set(0, rgb[0], rgb[1], rgb[2])) {
    SET_TRUE_RETURN();
  }
}
