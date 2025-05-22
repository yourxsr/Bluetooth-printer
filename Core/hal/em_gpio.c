#include "em_gpio.h"

void digitalWrite(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, int PinState)
{
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, (GPIO_PinState)PinState);
}

int digitalRead(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}
