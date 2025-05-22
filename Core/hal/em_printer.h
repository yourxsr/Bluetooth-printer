#ifndef __EM_PRINTER_H__
#define __EM_PRINTER_H__

#include "em_config.h"
#include "em_gpio.h"
//每行总点数
#define TPH_DOTS_PER_LINE 384
//每行字节长度 384/8
#define TPH_DI_LEN 48
//所有通道打印
#define ALL_STB_NUM 0xFF

void Printer_Init(void);

void Printing_Init(void);

void set_heat_density(uint8_t density);

void start_printing_by_queuebuf(void);

void start_printing_by_onestb(uint8_t stbnum, uint8_t *data, uint32_t len);

void testSTB(void);

#endif
