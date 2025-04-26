/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file uart.h
 * @brief UART driver interface
 *
 * Defines functions for controlling UART communication on ESP32.
 */
#ifndef DRV_UART_H
#define DRV_UART_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../lib/fn.h"

// UART設定のデフォルト値
#define UART_DEFAULT_BAUD_RATE 115200
#define UART_DEFAULT_DATA_BITS UART_DATA_8_BITS
#define UART_DEFAULT_PARITY UART_PARITY_DISABLE
#define UART_DEFAULT_STOP_BITS UART_STOP_BITS_1

/**
 * @brief 使用可能なUARTポート番号
 *
 * UART0はコンソール用に使用されているため、UART1とUART2を使用します
 */
typedef enum {
  UART_PORT_1 = 1,  // UART1
  UART_PORT_2 = 2,  // UART2
} uart_port_num_t;

/**
 * @brief UART初期化
 *
 * 指定されたUARTポートを初期化します
 *
 * @param uart_num UARTポート番号 (1または2)
 * @param tx_pin 送信ピン番号
 * @param rx_pin 受信ピン番号
 * @param baud_rate ボーレート
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_uart_init(uart_port_num_t uart_num, int tx_pin, int rx_pin,
                   int baud_rate);

/**
 * @brief UARTデータ送信
 *
 * 指定されたUARTポートからデータを送信します
 *
 * @param uart_num UARTポート番号
 * @param data 送信するデータ
 * @param len データの長さ
 * @return 送信されたバイト数、エラー時は負の値
 */
int drv_uart_write(uart_port_num_t uart_num, const uint8_t* data, size_t len);

/**
 * @brief UARTデータ受信
 *
 * 指定されたUARTポートからデータを受信します
 *
 * @param uart_num UARTポート番号
 * @param data 受信データを格納するバッファ
 * @param len 受信するバイト数の最大値
 * @param timeout_ms タイムアウト（ミリ秒）
 * @return 受信したバイト数、エラー時は負の値
 */
int drv_uart_read(uart_port_num_t uart_num, uint8_t* data, size_t len,
                  uint32_t timeout_ms);

/**
 * @brief UART終了処理
 *
 * 指定されたUARTポートを終了します
 *
 * @param uart_num UARTポート番号
 * @return kSuccess 成功時、kFailure 失敗時
 */
fn_t drv_uart_deinit(uart_port_num_t uart_num);

#endif  // DRV_UART_H