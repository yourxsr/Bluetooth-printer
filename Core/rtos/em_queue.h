#ifndef __EM_QUEUE_H__
#define __EM_QUEUE_H__

#include "em_config.h"
#include "em_spi.h"

// 一行最大byte
#define MAX_ONELINE_BYTE 48
// 最大行数
#define MAX_LINE 275

typedef struct{
    uint8_t buffer[MAX_ONELINE_BYTE];//一行48byte
} ble_rx_buffer_t;

typedef struct{
	ble_rx_buffer_t printer_buffer[MAX_LINE];//275行
	uint32_t r_index;//读取行
	uint32_t w_index;//写入行
	uint32_t left_line;//总行数
}ble_rw_t;

void Clean_ble_rw_t(void);
void Queue_Init(void);
uint32_t get_ble_rw_leftline(void);
void Write_to_buffer(uint8_t *pdata,size_t length);
uint8_t* read_from_buffer(void);

#endif
