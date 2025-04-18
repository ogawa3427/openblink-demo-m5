/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright (c) 2025 ViXion Inc. All Rights Reserved.
 */
/**
 * @file pwm.h
 * @brief API interface for PWM control in mruby/c
 *
 * Defines the interface for the PWM class in mruby/c, which provides
 * methods for controlling PWM outputs on GPIO pins.
 */
#ifndef API_PWM_H
#define API_PWM_H

#include "../lib/fn.h"

/**
 * @brief Defines the PWM class and methods for mruby/c
 *
 * Creates the PWM class and registers methods for PWM control
 * which allows Ruby code to control PWM outputs.
 *
 * @return kSuccess always
 */
fn_t api_pwm_define(void);

#endif  // API_PWM_H