#ifndef __EM_BLE_H__
#define __EM_BLE_H__

#include "em_config.h"
#include  <ctype.h>

void uart_cmd_handle(uint8_t data);
void ble_Init(void);
void ble_report(void);
void clean_blepack_count(void);
uint32_t get_blepack_count(void);
bool get_ble_connect(void);
void ble_status_data_clean(void);
#endif
