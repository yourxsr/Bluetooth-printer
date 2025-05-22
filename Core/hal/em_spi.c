#include "em_spi.h"

void spiCommand(uint8_t *data_buffer, uint8_t data_len)
{
      HAL_SPI_Transmit(&hspi1, data_buffer, data_len, 0xFFFF);
}

void init_spi()
{
//   已经在main函数中初始化
}
