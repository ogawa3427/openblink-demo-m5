/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file esp_now_api.c
 * @brief Implementation of ESP-NOW API for mruby/c
 *
 * Implements the ESPNow class and its methods for the mruby/c VM,
 * providing functionality for ESP-NOW wireless communication.
 */
#include "esp_now_api.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../drv/blink_esp_now.h"
#include "../lib/fn.h"
#include "esp_log.h"
#include "mrubyc.h"

static const char *TAG = "mrbc_esp_now";

/**
 * @brief Forward declarations for mruby/c method implementations
 */
static void c_esp_now_init(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_send(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_add_peer(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_remove_peer(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_status(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_received_counter(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_counter(mrb_vm *vm, mrb_value *v, int argc);
static void c_esp_now_deinit(mrb_vm *vm, mrb_value *v, int argc);

/**
 * @brief Defines the ESPNow class and methods for mruby/c
 *
 * Creates the ESPNow class and registers all ESP-NOW methods
 * for wireless communication functionality.
 *
 * @return kSuccess always
 */
fn_t api_esp_now_define(void) {
  mrb_class *class_esp_now;
  class_esp_now = mrbc_define_class(0, "ESPNow", mrbc_class_object);

  mrbc_define_method(0, class_esp_now, "init", c_esp_now_init);
  mrbc_define_method(0, class_esp_now, "send", c_esp_now_send);
  mrbc_define_method(0, class_esp_now, "add_peer", c_esp_now_add_peer);
  mrbc_define_method(0, class_esp_now, "remove_peer", c_esp_now_remove_peer);
  mrbc_define_method(0, class_esp_now, "status", c_esp_now_status);
  mrbc_define_method(0, class_esp_now, "received_counter",
                     c_esp_now_received_counter);
  mrbc_define_method(0, class_esp_now, "counter", c_esp_now_counter);
  mrbc_define_method(0, class_esp_now, "deinit", c_esp_now_deinit);

  ESP_LOGI(TAG, "ESPNow class defined for mruby/c");
  return kSuccess;
}

/**
 * @brief Implementation of the init method for the ESPNow class
 *
 * Initializes ESP-NOW with the specified mode.
 * Usage: ESPNow.init(:send) or ESPNow.init(:recv) or ESPNow.init(:both)
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_init(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // Default to failure

  if (argc < 1) {
    ESP_LOGE(TAG, "ESPNow.init requires mode argument");
    return;
  }

  esp_now_mode_t mode = ESP_NOW_MODE_SEND;  // Default mode

  // Parse mode argument (symbol)
  if (v[1].tt == MRBC_TT_SYMBOL) {
    mrbc_sym sym = v[1].i;
    if (sym == mrbc_str_to_symid("send")) {
      mode = ESP_NOW_MODE_SEND;
    } else if (sym == mrbc_str_to_symid("recv")) {
      mode = ESP_NOW_MODE_RECV;
    } else if (sym == mrbc_str_to_symid("both")) {
      mode = ESP_NOW_MODE_BOTH;
    } else {
      ESP_LOGE(TAG, "Invalid mode symbol");
      return;
    }
  } else {
    ESP_LOGE(TAG, "Mode must be a symbol (:send, :recv, or :both)");
    return;
  }

  if (drv_esp_now_init(mode) == kSuccess) {
    ESP_LOGI(TAG, "ESP-NOW initialized with mode %d", mode);
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "Failed to initialize ESP-NOW");
  }
}

/**
 * @brief Implementation of the send method for the ESPNow class
 *
 * Sends an integer value via ESP-NOW.
 * Usage: ESPNow.send(123)
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_send(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // Default to failure

  if (argc < 1) {
    ESP_LOGE(TAG, "ESPNow.send requires value argument");
    return;
  }

  if (v[1].tt != MRBC_TT_INTEGER) {
    ESP_LOGE(TAG, "Send value must be an integer");
    return;
  }

  int value = v[1].i;

  if (drv_esp_now_send_data(value) == kSuccess) {
    ESP_LOGI(TAG, "ESP-NOW data sent: %d", value);
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "Failed to send ESP-NOW data");
  }
}

/**
 * @brief Implementation of the add_peer method for the ESPNow class
 *
 * Adds a peer MAC address for ESP-NOW communication.
 * Usage: ESPNow.add_peer("AA:BB:CC:DD:EE:FF")
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_add_peer(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // Default to failure

  if (argc < 1) {
    ESP_LOGE(TAG, "ESPNow.add_peer requires MAC address argument");
    return;
  }

  if (v[1].tt != MRBC_TT_STRING) {
    ESP_LOGE(TAG, "MAC address must be a string");
    return;
  }

  const char *mac_str = (const char *)v[1].string->data;
  uint8_t mac_addr[6];

  // Parse MAC address string (format: "AA:BB:CC:DD:EE:FF")
  if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac_addr[0],
             &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4],
             &mac_addr[5]) != 6) {
    ESP_LOGE(TAG, "Invalid MAC address format");
    return;
  }

  if (drv_esp_now_add_peer(mac_addr) == kSuccess) {
    ESP_LOGI(TAG, "ESP-NOW peer added: %s", mac_str);
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "Failed to add ESP-NOW peer");
  }
}

/**
 * @brief Implementation of the remove_peer method for the ESPNow class
 *
 * Removes a peer MAC address from ESP-NOW communication.
 * Usage: ESPNow.remove_peer("AA:BB:CC:DD:EE:FF")
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_remove_peer(mrb_vm *vm, mrb_value *v, int argc) {
  SET_FALSE_RETURN();  // Default to failure

  if (argc < 1) {
    ESP_LOGE(TAG, "ESPNow.remove_peer requires MAC address argument");
    return;
  }

  if (v[1].tt != MRBC_TT_STRING) {
    ESP_LOGE(TAG, "MAC address must be a string");
    return;
  }

  const char *mac_str = (const char *)v[1].string->data;
  uint8_t mac_addr[6];

  // Parse MAC address string
  if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac_addr[0],
             &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4],
             &mac_addr[5]) != 6) {
    ESP_LOGE(TAG, "Invalid MAC address format");
    return;
  }

  if (drv_esp_now_remove_peer(mac_addr) == kSuccess) {
    ESP_LOGI(TAG, "ESP-NOW peer removed: %s", mac_str);
    SET_TRUE_RETURN();
  } else {
    ESP_LOGE(TAG, "Failed to remove ESP-NOW peer");
  }
}

/**
 * @brief Implementation of the status method for the ESPNow class
 *
 * Returns the current ESP-NOW status as a symbol.
 * Usage: status = ESPNow.status  # Returns :ready, :sending, or :error
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_status(mrb_vm *vm, mrb_value *v, int argc) {
  esp_now_status_t status = drv_esp_now_get_status();

  switch (status) {
    case ESP_NOW_STATUS_READY:
      SET_RETURN(mrbc_symbol_new(vm, "ready"));
      break;
    case ESP_NOW_STATUS_SENDING:
      SET_RETURN(mrbc_symbol_new(vm, "sending"));
      break;
    case ESP_NOW_STATUS_ERROR:
      SET_RETURN(mrbc_symbol_new(vm, "error"));
      break;
    default:
      SET_RETURN(mrbc_symbol_new(vm, "unknown"));
      break;
  }
}

/**
 * @brief Implementation of the received_counter method for the ESPNow class
 *
 * Returns the last received counter value.
 * Usage: count = ESPNow.received_counter
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_received_counter(mrb_vm *vm, mrb_value *v, int argc) {
  int counter = drv_esp_now_get_received_counter();
  SET_INT_RETURN(counter);
}

/**
 * @brief Implementation of the counter method for the ESPNow class
 *
 * Returns the current counter value.
 * Usage: count = ESPNow.counter
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_counter(mrb_vm *vm, mrb_value *v, int argc) {
  int counter = drv_esp_now_get_counter();
  SET_INT_RETURN(counter);
}

/**
 * @brief Implementation of the deinit method for the ESPNow class
 *
 * Deinitializes ESP-NOW.
 * Usage: ESPNow.deinit
 *
 * @param vm Pointer to the mruby/c VM
 * @param v Pointer to the method arguments
 * @param argc Number of arguments
 */
static void c_esp_now_deinit(mrb_vm *vm, mrb_value *v, int argc) {
  if (drv_esp_now_deinit() == kSuccess) {
    ESP_LOGI(TAG, "ESP-NOW deinitialized");
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}