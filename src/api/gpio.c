#include "gpio.h"

#include <driver/gpio.h>
#include <stdbool.h>

#include "../../.pio/libdeps/m5stack-stamps3/mrubyc/src/mrubyc.h"
#include "../lib/fn.h"

// **************************************************************************
// forward declarations
static void c_gpio_set_mode(mrb_vm *vm, mrb_value *v, int argc);
static void c_gpio_digital_write(mrb_vm *vm, mrb_value *v, int argc);
static void c_gpio_digital_read(mrb_vm *vm, mrb_value *v, int argc);

// **************************************************************************
// api_gpio_define
fn_t api_gpio_define(void) {
  mrb_class *class_gpio;
  class_gpio = mrbc_define_class(0, "GPIO", mrbc_class_object);
  
  // Define GPIO mode constants
  mrbc_define_const(0, class_gpio, "INPUT", &mrbc_integer_value(GPIO_MODE_INPUT));
  mrbc_define_const(0, class_gpio, "OUTPUT", &mrbc_integer_value(GPIO_MODE_OUTPUT));
  
  // Define methods
  mrbc_define_method(0, class_gpio, "set_mode", c_gpio_set_mode);
  mrbc_define_method(0, class_gpio, "digital_write", c_gpio_digital_write);
  mrbc_define_method(0, class_gpio, "digital_read", c_gpio_digital_read);
  
  return kSuccess;
}

// **************************************************************************
// c_gpio_set_mode
static void c_gpio_set_mode(mrb_vm *vm, mrb_value *v, int argc) {
  int pin = GET_INT_ARG(1);
  int mode = GET_INT_ARG(2);
  
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << pin),
    .mode = mode,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };
  
  esp_err_t err = gpio_config(&io_conf);
  SET_BOOL_RETURN(err == ESP_OK);
}

// **************************************************************************
// c_gpio_digital_write
static void c_gpio_digital_write(mrb_vm *vm, mrb_value *v, int argc) {
  int pin = GET_INT_ARG(1);
  int level = GET_INT_ARG(2);
  
  esp_err_t err = gpio_set_level(pin, level);
  SET_BOOL_RETURN(err == ESP_OK);
}

// **************************************************************************
// c_gpio_digital_read
static void c_gpio_digital_read(mrb_vm *vm, mrb_value *v, int argc) {
  int pin = GET_INT_ARG(1);
  
  int level = gpio_get_level(pin);
  SET_INT_RETURN(level);
} 