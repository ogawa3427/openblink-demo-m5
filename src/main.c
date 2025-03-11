/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "../.pio/libdeps/m5stack-stamps3/mrubyc/src/mrubyc.h"
#include "api/blink.h"
#include "api/gpio.h"
#include "api/input.h"
#include "api/led.h"
#include "app/blink.h"
#include "app/init.h"
#include "drv/ble_blink.h"
#include "lib/fn.h"
#include "rb/slot1.h"
#include "rb/slot2.h"

#define MRBC_HEAP_MEMORY_SIZE (15 * 1024)
#define MRUBYC_VM_MAIN_STACK_SIZE (50 * 1024)

static bool request_mruby_reload = false;

static uint8_t memory_pool[MRBC_HEAP_MEMORY_SIZE] = {0};
static uint8_t bytecode_slot2[BLINK_MAX_BYTECODE_SIZE] = {0};

void app_main() {
  app_init();
  while (1) {
    mrbc_tcb *tcb[MAX_VM_COUNT] = {NULL};

    // mruby/c initialize
    mrbc_init(memory_pool, MRBC_HEAP_MEMORY_SIZE);

    ////////////////////
    // Class, Method
    api_led_define();    // LED.*
    api_input_define();  // Input.*
    api_blink_define();  // Blink.*
    api_gpio_define();   // GPIO.*

    ////////////////////
    // Clear reload request flag
    request_mruby_reload = false;
    // Load mruby bytecode
    if (0 == blink_load(bytecode_slot2,
                        sizeof(bytecode_slot2) / sizeof(bytecode_slot2[0]))) {
      memcpy(bytecode_slot2, slot2, sizeof(slot2));
    }

    ////////////////////
    // mruby/c create task
    tcb[0] = mrbc_create_task(slot1, NULL);
    tcb[1] = mrbc_create_task(bytecode_slot2, NULL);

    if ((tcb[0] == NULL) || (tcb[1] == NULL)) {
    }
    // set priority
    mrbc_change_priority(tcb[0], 1);
    mrbc_change_priority(tcb[1], 2);

    ////////////////////
    mrbc_run();

    ble_print("mruby/c finished");
    ////////////////////
    // mruby/c cleanup
    mrbc_cleanup();
  }
}

// **************************************************************************
// app_mrubyc_vm_set_reload
fn_t app_mrubyc_vm_set_reload(void) {
  request_mruby_reload = true;
  return kSuccess;
}

// **************************************************************************
// app_mrubyc_vm_get_reload
bool app_mrubyc_vm_get_reload(void) { return request_mruby_reload; }
