#ifndef __EM_HAL_H__
#define __EM_HAL_H__

#include "em_config.h"

typedef enum{
	LED_CONNECT = 0,//LED连接
	LED_DISCONNECT,//失连
	LED_WARN,//警告
	LED_PRINTER_START,//开始打印
	LED_BLE_INIT,//蓝牙初始化
}led_type_e;

void Hal_Init(void);
void Run_Led(led_type_e type);
bool read_if_need_add_paper(void);
void read_hal_state(void);

#endif
