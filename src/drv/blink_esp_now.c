/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file blink_esp_now.c
 * @brief ESP-NOW driver implementation for OpenBlink
 *
 * Implements ESP-NOW functionality for low-latency P2P communication
 * between OpenBlink devices using the ESP-IDF ESP-NOW API.
 */
#include "blink_esp_now.h"

#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_now.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <string.h>

static const char *TAG = "BLINK_ESP_NOW";

// Static variables
static esp_now_mode_t current_mode = ESP_NOW_MODE_SEND;
static esp_now_status_t current_status = ESP_NOW_STATUS_READY;
static int counter = 0;
static int received_counter = 0;
static bool is_initialized = false;

// Default peer MAC address (broadcast)
static uint8_t peer_mac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/**
 * @brief ESP-NOW send callback
 */
static void on_data_sent(const uint8_t *mac_addr,
                         esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    ESP_LOGI(TAG, "Send Success");
    current_status = ESP_NOW_STATUS_READY;
  } else {
    ESP_LOGE(TAG, "Send Failed");
    current_status = ESP_NOW_STATUS_ERROR;
  }
}

/**
 * @brief ESP-NOW receive callback
 */
static void on_data_recv(const esp_now_recv_info_t *recv_info,
                         const uint8_t *incomingData, int len) {
  if (len == sizeof(esp_now_data_t)) {
    esp_now_data_t data;
    memcpy(&data, incomingData, sizeof(data));
    received_counter = data.counter_value;

    ESP_LOGI(TAG, "Received: counter=%d, timestamp=%lu", data.counter_value,
             data.timestamp);
  } else {
    ESP_LOGW(TAG, "Invalid data length: %d", len);
  }
}

/**
 * @brief Initialize WiFi stack for ESP-NOW
 */
static esp_err_t init_wifi(void) {
  // NVS initialization
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    ret = nvs_flash_init();
  }

  // Network interface initialization
  esp_netif_init();
  esp_event_loop_create_default();

  // WiFi initialization (lightweight configuration)
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  cfg.nvs_enable = 0;  // Disable NVS for ESP-NOW only
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_start();

  // Enable promiscuous mode for ESP-NOW
  esp_wifi_set_promiscuous(true);

  ESP_LOGI(TAG, "WiFi initialized for ESP-NOW");
  return ESP_OK;
}

/**
 * @brief Initialize ESP-NOW driver
 */
fn_t drv_esp_now_init(esp_now_mode_t mode) {
  if (is_initialized) {
    ESP_LOGW(TAG, "ESP-NOW already initialized");
    return kSuccess;
  }

  current_mode = mode;

  // Initialize WiFi stack
  if (init_wifi() != ESP_OK) {
    ESP_LOGE(TAG, "WiFi initialization failed");
    return kFailure;
  }

  // Initialize ESP-NOW
  esp_err_t ret = esp_now_init();
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing ESP-NOW: 0x%x", ret);
    return kFailure;
  }
  ESP_LOGI(TAG, "ESP-NOW initialized successfully");

  // Register callbacks
  esp_now_register_send_cb(on_data_sent);
  esp_now_register_recv_cb(on_data_recv);

  // Add peer for send mode
  if (mode == ESP_NOW_MODE_SEND || mode == ESP_NOW_MODE_BOTH) {
    drv_esp_now_add_peer(peer_mac);
  }

  is_initialized = true;
  current_status = ESP_NOW_STATUS_READY;
  ESP_LOGI(TAG, "ESP-NOW driver initialized in mode %d", mode);

  return kSuccess;
}

/**
 * @brief Deinitialize ESP-NOW driver
 */
fn_t drv_esp_now_deinit(void) {
  if (!is_initialized) {
    return kSuccess;
  }

  esp_now_deinit();
  is_initialized = false;
  ESP_LOGI(TAG, "ESP-NOW driver deinitialized");

  return kSuccess;
}

/**
 * @brief Send data via ESP-NOW
 */
fn_t drv_esp_now_send_data(int value) {
  if (!is_initialized) {
    ESP_LOGE(TAG, "ESP-NOW not initialized");
    return kFailure;
  }

  if (current_mode == ESP_NOW_MODE_RECV) {
    ESP_LOGE(TAG, "Cannot send in receive-only mode");
    return kFailure;
  }

  esp_now_data_t data;
  data.counter_value = value;
  data.timestamp = esp_timer_get_time() / 1000;  // Convert to ms

  current_status = ESP_NOW_STATUS_SENDING;
  esp_err_t result = esp_now_send(peer_mac, (uint8_t *)&data, sizeof(data));

  if (result == ESP_OK) {
    ESP_LOGI(TAG, "Sending counter: %d", value);
    counter = value;
    return kSuccess;
  } else {
    ESP_LOGE(TAG, "Error sending data: 0x%x", result);
    current_status = ESP_NOW_STATUS_ERROR;
    return kFailure;
  }
}

/**
 * @brief Add peer to ESP-NOW
 */
fn_t drv_esp_now_add_peer(const uint8_t *mac_addr) {
  if (!is_initialized) {
    ESP_LOGE(TAG, "ESP-NOW not initialized");
    return kFailure;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac_addr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  esp_err_t ret = esp_now_add_peer(&peerInfo);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add peer: 0x%x", ret);
    return kFailure;
  } else {
    ESP_LOGI(TAG, "Peer added successfully");
    // Update local peer MAC
    memcpy(peer_mac, mac_addr, 6);
    return kSuccess;
  }
}

/**
 * @brief Remove peer from ESP-NOW
 */
fn_t drv_esp_now_remove_peer(const uint8_t *mac_addr) {
  if (!is_initialized) {
    ESP_LOGE(TAG, "ESP-NOW not initialized");
    return kFailure;
  }

  esp_err_t ret = esp_now_del_peer(mac_addr);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to remove peer: 0x%x", ret);
    return kFailure;
  } else {
    ESP_LOGI(TAG, "Peer removed successfully");
    return kSuccess;
  }
}

/**
 * @brief Get current ESP-NOW status
 */
esp_now_status_t drv_esp_now_get_status(void) { return current_status; }

/**
 * @brief Get received counter value
 */
int drv_esp_now_get_received_counter(void) { return received_counter; }

/**
 * @brief Get current counter value
 */
int drv_esp_now_get_counter(void) { return counter; }

/**
 * @brief Set ESP-NOW mode
 */
fn_t drv_esp_now_set_mode(esp_now_mode_t mode) {
  current_mode = mode;
  ESP_LOGI(TAG, "ESP-NOW mode set to %d", mode);
  return kSuccess;
}