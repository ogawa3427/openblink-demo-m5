#ifndef DRV_LED_H
#define DRV_LED_H

#include <stdint.h>

#include "../lib/fn.h"

fn_t drv_led_init(void);
fn_t drv_led_set(const uint8_t kNum, const uint8_t kRed, const uint8_t kGreen,
                 const uint8_t kBlue);

#endif
