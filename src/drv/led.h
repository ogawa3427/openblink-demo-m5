/**
 * @file led.h
 * @brief LED driver interface
 *
 * Provides functions for initializing and controlling RGB LEDs
 * on the M5Stack hardware.
 */
#ifndef LED_H
#define LED_H

#include <stdint.h>

#include "../lib/fn.h"
#include "driver/gpio.h"

/**
 * @brief Initializes the LED driver
 *
 * Sets up the RMT peripheral for controlling WS2812 RGB LEDs.
 *
 * @param pin_num GPIO number
 * @param size Number of LEDs
 * @return kSuccess on successful initialization
 */
fn_t drv_led_init(gpio_num_t pin_num, uint8_t size);

/**
 * @brief Sets the RGB LED color
 *
 * @param kNum LED index (0-based)
 * @param kRed Red component (0-255)
 * @param kGreen Green component (0-255)
 * @param kBlue Blue component (0-255)
 * @return kSuccess on successful operation, kFailure if LED index is invalid
 */
fn_t drv_led_set(const uint8_t kNum, const uint8_t kRed, const uint8_t kGreen,
                 const uint8_t kBlue);

// PWM制御関数
fn_t drv_pwm_init(void);
int drv_pwm_setup_pin(gpio_num_t gpio_pin, uint8_t initial_duty);
fn_t drv_pwm_set_duty(int channel, uint8_t duty);
fn_t drv_pwm_disable(int channel);

#endif
