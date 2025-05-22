#ifndef __EM_SPI_H__
#define __EM_SPI_H__

#include "em_config.h"
#include "spi.h"

void init_spi(void);
void spiCommand(uint8_t *data_buffer, uint8_t data_len);

#endif
