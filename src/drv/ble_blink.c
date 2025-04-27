/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file ble_blink.c
 * @brief Implementation of BLE interface for Blink functionality
 *
 * Implements the BLE services and characteristics for the Blink feature,
 * handling bytecode transfer and device control over BLE.
 */
#include "ble_blink.h"

#include <inttypes.h>
#include <string.h>

#include "../app/blink.h"
#include "../lib/crc/crc.h"
#include "../main.h"
#include "ble.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "BLE_BLINK";

static uint8_t blink_bytecode[BLINK_MAX_BYTECODE_SIZE] = {0};

/**
 * @brief GATT write callback for the Program characteristic
 *
 * Handles write operations to the Program characteristic, processing
 * various commands for bytecode transfer and device control.
 *
 * @param conn_handle Connection handle
 * @param attr_handle Attribute handle
 * @param ctxt GATT access context
 * @param arg User argument (unused)
 * @return 0 on success, non-zero on failure
 */
static int blink_write_program(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

/**
 * @brief GATT read callback for the MTU characteristic
 *
 * Handles read operations to the MTU characteristic, returning
 * the current MTU size.
 *
 * @param conn_handle Connection handle
 * @param attr_handle Attribute handle
 * @param ctxt GATT access context
 * @param arg User argument (unused)
 * @return 0 on success, non-zero on failure
 */
static int blink_read_mtu(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg);

/**
 * @brief Processes a Data command
 *
 * Handles the 'D' command for transferring bytecode data chunks.
 *
 * @param header Pointer to the command header
 * @param len Total length of the command data
 * @return 0 on success, non-zero on failure
 */
static int blink_program_command_D(BLINK_CHUNK_HEADER *header,
                                   struct ble_gatt_access_ctxt *ctxt);

/**
 * @brief Processes a Program command
 *
 * Handles the 'P' command for finalizing bytecode transfer and
 * verifying CRC.
 *
 * @param header Pointer to the command header
 * @return 0 on success, non-zero on failure
 */
static int blink_program_command_P(BLINK_CHUNK_HEADER *header);

static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     // Service: 22-7d-a5-2c-e1-3a-41-2b-be-fb-ba-22-56-bb-7f-be
     .uuid =
         BLE_UUID128_DECLARE(0xbe, 0x7f, 0xbb, 0x56, 0x22, 0xba, 0xfb, 0xbe,
                             0x2b, 0x41, 0x3a, 0xe1, 0x2c, 0xa5, 0x7d, 0x22),
     .characteristics =
         (struct ble_gatt_chr_def[]){
             {
                 // Program: ad-9f-dd-56-11-35-4a-84-92-3c-ce-5a-24-43-85-e7
                 .uuid = BLE_UUID128_DECLARE(0xe7, 0x85, 0x43, 0x24, 0x5a, 0xce,
                                             0x3c, 0x92, 0x84, 0x4a, 0x35, 0x11,
                                             0x56, 0xdd, 0x9f, 0xad),
                 .access_cb = blink_write_program,
                 .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
             },
             {
                 // Console: a0-15-b3-de-18-5a-42-52-aa-04-7a-87-d3-8c-e1-48
                 .uuid = BLE_UUID128_DECLARE(0x48, 0xe1, 0x8c, 0xd3, 0x87, 0x7a,
                                             0x04, 0xaa, 0x52, 0x42, 0x5a, 0x18,
                                             0xde, 0xb3, 0x15, 0xa0),
                 .access_cb = blink_read_mtu,
                 .val_handle = &hrs_hrm_handle,
                 .flags = BLE_GATT_CHR_F_NOTIFY,
             },
             {
                 // Mtu: ca-14-11-51-31-13-44-8b-b2-1a-6a-62-03-d2-53-ff
                 .uuid = BLE_UUID128_DECLARE(0xff, 0x53, 0xd2, 0x03, 0x62, 0x6a,
                                             0x1a, 0xb2, 0x8b, 0x44, 0x13, 0x31,
                                             0x51, 0x11, 0x14, 0xca),
                 .access_cb = blink_read_mtu,
                 .flags = BLE_GATT_CHR_F_READ,
             },
             {
                 0,
             },
         }},
    {
        0,
    },
};

/**
 * @brief Initializes the BLE Blink service
 *
 * Sets up the GATT services and characteristics for the Blink functionality.
 *
 * @return 0 on success, non-zero on failure
 */
int ble_blink_init(void) {
  ESP_LOGI(TAG, "Initializing BLE Blink service...");
  ESP_LOGI(TAG, "Free heap: %" PRIu32 " bytes", esp_get_free_heap_size());
  ESP_LOGI(TAG, "Min free heap: %" PRIu32 " bytes",
           esp_get_minimum_free_heap_size());

  int rc;

  ble_svc_gap_init();
  ble_svc_gatt_init();

  rc = ble_gatts_count_cfg(gatt_svcs);
  if (rc != 0) {
    ESP_LOGE(TAG, "ble_gatts_count_cfg failed: %d", rc);
    return rc;
  }

  rc = ble_gatts_add_svcs(gatt_svcs);
  if (rc != 0) {
    ESP_LOGE(TAG, "ble_gatts_add_svcs failed: %d", rc);
    return rc;
  }

  ESP_LOGI(TAG, "BLE Blink service initialized successfully.");
  ESP_LOGI(TAG, "Free heap after init: %" PRIu32 " bytes",
           esp_get_free_heap_size());
  ESP_LOGI(TAG, "Min free heap after init: %" PRIu32 " bytes",
           esp_get_minimum_free_heap_size());
  return 0;
}

/**
 * @brief GATT read callback for the MTU characteristic
 *
 * Handles read operations to the MTU characteristic, returning
 * the current MTU size.
 *
 * @param conn_handle Connection handle
 * @param attr_handle Attribute handle
 * @param ctxt GATT access context
 * @param arg User argument (unused)
 * @return 0 on success, non-zero on failure
 */
static int blink_read_mtu(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg) {
  uint16_t mtu = ble_att_mtu(conn_handle);
  os_mbuf_append(ctxt->om, &mtu, sizeof(mtu));
  return 0;
}

/**
 * @brief Sends a string over BLE
 *
 * Sends a text message to the connected BLE client as a notification.
 *
 * @param data Null-terminated string to send
 * @return 0 on success, negative value on failure
 */
int ble_print(const char *data) {
  struct os_mbuf *om = os_msys_get_pkthdr(strlen(data), 0);
  if (!notify_state) {
    return -1;
  }
  if (om == NULL) {
    return -1;
  }
  os_mbuf_append(om, data, strlen(data));
  return ble_gatts_notify_custom(conn_handle, hrs_hrm_handle, om);
}

/**
 * @brief GATT write callback for the Program characteristic
 *
 * Handles write operations to the Program characteristic, processing
 * various commands for bytecode transfer and device control.
 *
 * @param conn_handle Connection handle
 * @param attr_handle Attribute handle
 * @param ctxt GATT access context
 * @param arg User argument (unused)
 * @return 0 on success, non-zero on failure
 */
static int blink_write_program(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg) {
  ESP_LOGD(TAG, "blink_write_program called.");
  ESP_LOGI(TAG, "Free heap before write: %" PRIu32 " bytes",
           esp_get_free_heap_size());
  ESP_LOGD(TAG, "Min free heap: %" PRIu32 " bytes",
           esp_get_minimum_free_heap_size());

  BLINK_CHUNK_HEADER *header = (BLINK_CHUNK_HEADER *)ctxt->om->om_data;
  if (header->version != BLINK_VERSION) {
    ESP_LOGE(TAG, "Invalid BLINK version: %d", header->version);
    return BLE_ATT_ERR_INVALID_PDU;
  }

  switch (header->command) {
    case BLINK_CMD_DATA:
      ESP_LOGD(TAG, "Processing BLINK_CMD_DATA");
      if (blink_program_command_D(header, ctxt) != 0) {
        ESP_LOGE(TAG, "blink_program_command_D failed");
        return BLE_ATT_ERR_INVALID_PDU;
      }
      break;
    case BLINK_CMD_PROG:
      ESP_LOGD(TAG, "Processing BLINK_CMD_PROG");
      if (blink_program_command_P(header) != 0) {
        ESP_LOGE(TAG, "blink_program_command_P failed");
        return BLE_ATT_ERR_INVALID_PDU;
      }
      break;
    case BLINK_CMD_RESET:
      ESP_LOGI(TAG, "Processing BLINK_CMD_RESET");
      esp_restart();
      break;
    case BLINK_CMD_RELOAD:
      ESP_LOGI(TAG, "Processing BLINK_CMD_RELOAD");
      app_mrubyc_vm_set_reload();
      break;
    default:
      ESP_LOGW(TAG, "Unknown BLINK command: %c", header->command);
      return BLE_ATT_ERR_INVALID_PDU;
  }

  ESP_LOGD(TAG, "blink_write_program finished successfully.");
  ESP_LOGI(TAG, "Free heap after write: %" PRIu32 " bytes",
           esp_get_free_heap_size());
  ESP_LOGD(TAG, "Min free heap after write: %" PRIu32 " bytes",
           esp_get_minimum_free_heap_size());
  return 0;
}

/**
 * @brief Processes a Data command
 *
 * Handles the 'D' command for transferring bytecode data chunks.
 *
 * @param header Pointer to the command header
 * @param len Total length of the command data
 * @return 0 on success, non-zero on failure
 */
static int blink_program_command_D(BLINK_CHUNK_HEADER *header,
                                   struct ble_gatt_access_ctxt *ctxt) {
  ESP_LOGI(TAG, "Free heap before D command: %" PRIu32 " bytes",
           esp_get_free_heap_size());
  if (ctxt->om->om_len < sizeof(BLINK_CHUNK_DATA)) {
    ESP_LOGE(TAG, "Data chunk too small: %d", ctxt->om->om_len);
    return -1;
  }
  BLINK_CHUNK_DATA *data_chunk = (BLINK_CHUNK_DATA *)header;
  const uint16_t size = data_chunk->size;
  const uint16_t offset = data_chunk->offset;
  ESP_LOGD(TAG, "Receiving data chunk: offset=%d, size=%d", offset, size);
  if (offset + size > BLINK_MAX_BYTECODE_SIZE) {
    ESP_LOGE(TAG, "Data chunk exceeds max size: offset=%d, size=%d", offset,
             size);
    return -1;
  }

  int ofs = 0;
  struct os_mbuf *om = ctxt->om;
  while (1) {
    uint8_t *buf = (uint8_t *)om->om_data;
    uint16_t len = om->om_len;
    if (om == ctxt->om) {
      buf += sizeof(BLINK_CHUNK_DATA);
      len -= sizeof(BLINK_CHUNK_DATA);
    }
    if (ofs + len > size) {
      return -1;
    }
    memcpy(&blink_bytecode[offset + ofs], buf, len);
    ofs += len;
    if (om->om_next.sle_next == NULL) break;
    om = om->om_next.sle_next;
  }
  if (ofs != size) {
    ESP_LOGE(TAG,
             "Incomplete data transfer: expected %d bytes, received %d bytes",
             size, ofs);
    return -1;
  }
  ESP_LOGD(TAG, "Data chunk received successfully.");
  ESP_LOGI(TAG, "Free heap after D command: %" PRIu32 " bytes",
           esp_get_free_heap_size());
  return 0;
}

/**
 * @brief Processes a Program command
 *
 * Handles the 'P' command for finalizing bytecode transfer and
 * verifying CRC.
 *
 * @param header Pointer to the command header
 * @return 0 on success, non-zero on failure
 */
static int blink_program_command_P(BLINK_CHUNK_HEADER *header) {
  BLINK_CHUNK_PROGRAM *p = (BLINK_CHUNK_PROGRAM *)header;
  ESP_LOGD(TAG, "Processing Program command: length=%d, crc=0x%04X", p->length,
           p->crc);
  ESP_LOGI(TAG, "Free heap before P command: %" PRIu32 " bytes",
           esp_get_free_heap_size());

  if (p->length > BLINK_MAX_BYTECODE_SIZE) {
    ESP_LOGE(TAG, "Program length exceeds max size: %d", p->length);
    return -1;
  }

  uint16_t crc16 = crc16_reflect(0xd175U, 0xFFFFU, blink_bytecode, p->length);
  ESP_LOGD(TAG, "Calculated CRC: 0x%04X", crc16);

  if (crc16 == p->crc) {
    ESP_LOGI(TAG, "CRC check passed. Storing bytecode (length: %d)", p->length);
    blink_store(blink_bytecode, p->length);
  } else {
    ESP_LOGE(TAG, "CRC check failed. Expected 0x%04X, got 0x%04X", p->crc,
             crc16);
    memset(blink_bytecode, 0, sizeof(blink_bytecode));
    return -1;
  }

  memset(blink_bytecode, 0, sizeof(blink_bytecode));
  ESP_LOGD(TAG, "Program command processed successfully.");
  ESP_LOGI(TAG, "Free heap after P command: %" PRIu32 " bytes",
           esp_get_free_heap_size());
  return 0;
}
