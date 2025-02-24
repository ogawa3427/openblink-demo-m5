#ifndef DRV_BLE_H
#define DRV_BLE_H

#include <stdbool.h>
#include <stdint.h>

void ble_init(void);
extern uint16_t hrs_hrm_handle;
extern bool notify_state;
extern uint16_t conn_handle;

#endif
