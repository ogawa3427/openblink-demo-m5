/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
#include "blink.h"

#include <stddef.h>
#include <stdint.h>

#include "../lib/fn.h"
#include "nvs.h"
#include "nvs_flash.h"

// **************************************************************************//
// blink_load
size_t blink_load(uint8_t *const data, const size_t kLength) {
  nvs_handle_t handle;
  size_t length = kLength;
  if (kLength < blink_get_data_length()) {
    return 0;
  }
  if (ESP_OK != nvs_open("openblink", NVS_READONLY, &handle)) {
    return 0;
  }
  if (ESP_OK != nvs_get_blob(handle, "slot2", data, &length)) {
    nvs_close(handle);
    return 0;
  }
  nvs_close(handle);
  return length;
}

// **************************************************************************
// blink_store
size_t blink_store(const uint8_t *const kData, const size_t kLength) {
  nvs_handle_t handle;
  if (ESP_OK != nvs_open("openblink", NVS_READWRITE, &handle)) {
    return 0;
  }
  if (ESP_OK != nvs_set_blob(handle, "slot2", kData, kLength)) {
    nvs_close(handle);
    return 0;
  }
  if (ESP_OK != nvs_commit(handle)) {
    nvs_close(handle);
    return 0;
  }
  nvs_close(handle);
  return kLength;
}

// **************************************************************************
// blink_get_data_length
size_t blink_get_data_length(void) {
  nvs_handle_t handle;
  size_t length = 0;
  if (ESP_OK != nvs_open("openblink", NVS_READONLY, &handle)) {
    return 0;
  }
  if (ESP_OK != nvs_get_blob(handle, "slot2", 0, &length)) {
    nvs_close(handle);
    return 0;
  }
  nvs_close(handle);
  return length;
}

// **************************************************************************
// blink_delete
int blink_delete(void) {
  nvs_handle_t handle;
  if (ESP_OK != nvs_open("openblink", NVS_READWRITE, &handle)) {
    nvs_close(handle);
    return -1;
  }
  if (ESP_OK != nvs_erase_key(handle, "slot2")) {
    nvs_close(handle);
    return -1;
  }
  if (ESP_OK != nvs_commit(handle)) {
    nvs_close(handle);
    return -1;
  }
  nvs_close(handle);
  return 0;
}
