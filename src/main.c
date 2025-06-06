/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file main.c
 * @brief Main application entry point for OpenBlink demo on M5Stack
 *
 * This file contains the main application logic for the OpenBlink demo,
 * including mruby/c VM initialization and execution.
 */
#include "main.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "api/blink.h"
#include "api/input.h"
#include "api/led.h"
#include "api/pwm.h"
#include "api/uart.h"
#include "app/blink.h"
#include "app/init.h"
// #include "driver/gpio.h"
#include "drv/ble_blink.h"
#include "esp_task_wdt.h"
#include "lib/fn.h"
#include "mrubyc.h"
#include "rb/slot1.h"
#include "rb/slot2.h"
#include "rb/slot_err.h"

extern void init_c_m5u();  // for features in m5u directory

#define MRBC_HEAP_MEMORY_SIZE (32 * 1024)
// #define BUTTON_GPIO GPIO_NUM_0

static bool request_mruby_reload = false;

static uint8_t memory_pool[MRBC_HEAP_MEMORY_SIZE] = {0};
static uint8_t bytecode_slot2[BLINK_MAX_BYTECODE_SIZE] = {0};

/**
 * @brief Main application entry point
 *
 * Initializes the application and runs the mruby/c VM in an infinite loop.
 * Handles loading bytecode, setting up API classes, and managing VM tasks.
 */
void app_main() {
  app_init();

  // Initialize WDT
  esp_task_wdt_config_t wdt_config = {.timeout_ms = 5000,
                                      .idle_core_mask = (1 << 0) | (1 << 1),
                                      .trigger_panic = false};
  ESP_ERROR_CHECK(esp_task_wdt_init(&wdt_config));
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

  // gpio_config_t io_conf = {
  //     .pin_bit_mask = (1ULL << BUTTON_GPIO),
  //     .mode = GPIO_MODE_INPUT,
  //     .pull_up_en = GPIO_PULLUP_ENABLE,
  //     .pull_down_en = GPIO_PULLDOWN_DISABLE,
  //     .intr_type = GPIO_INTR_DISABLE,
  // };
  // gpio_config(&io_conf);

  bool detect_abnormality = false;
  if (esp_reset_reason() == ESP_RST_PANIC) {
    detect_abnormality = true;
  }

  while (1) {
    // Reset WDT at the start of each loop iteration
    esp_task_wdt_reset();

    // if (gpio_get_level(BUTTON_GPIO) == 0) {
    //   printf("Button pressed, clearing slot 2 and reloading VM...\n");
    //   memset(bytecode_slot2, 0, sizeof(bytecode_slot2));
    //   request_mruby_reload = true;
    //   vTaskDelay(pdMS_TO_TICKS(200));
    // }

    mrbc_tcb *tcb[MAX_VM_COUNT] = {NULL};

    mrbc_init(memory_pool, MRBC_HEAP_MEMORY_SIZE);

    api_led_define();    // LED.*
    api_input_define();  // Input.*
    api_blink_define();  // Blink.*
    api_pwm_define();    // PWM.*
    api_uart_define();   // UART.*

    init_c_m5u();  // for features in m5u directory

    request_mruby_reload = false;

    if (detect_abnormality) {
      memcpy(bytecode_slot2, slot_err, sizeof(slot_err));
      printf("ERROR DETECTED \n");
    } else if (0 == blink_load(bytecode_slot2, sizeof(bytecode_slot2) /
                                                   sizeof(bytecode_slot2[0]))) {
      memcpy(bytecode_slot2, slot2, sizeof(slot2));
      printf("DEFAULT CODE LOADED \n");
    }
    detect_abnormality = false;

    tcb[0] = mrbc_create_task(slot1, NULL);
    tcb[1] = mrbc_create_task(bytecode_slot2, NULL);

    if ((tcb[0] == NULL) || (tcb[1] == NULL)) {
    }
    mrbc_change_priority(tcb[0], 1);
    mrbc_change_priority(tcb[1], 2);

    int ret = mrbc_run();
    printf("MRUBYC RUN RESULT:%d\n", ret);
    if (ret != 0) {
      detect_abnormality = true;
    }

    ble_print("mruby/c finished");
    mrbc_cleanup();
    request_mruby_reload = false;

    // Reset WDT before the end of loop
    esp_task_wdt_reset();
  }
}

/**
 * @brief Sets the mruby/c VM reload request flag
 *
 * @return kSuccess always
 */
fn_t app_mrubyc_vm_set_reload(void) {
  request_mruby_reload = true;
  return kSuccess;
}

/**
 * @brief Gets the current state of the mruby/c VM reload request flag
 *
 * @return true if reload is requested, false otherwise
 */
bool app_mrubyc_vm_get_reload(void) { return request_mruby_reload; }
