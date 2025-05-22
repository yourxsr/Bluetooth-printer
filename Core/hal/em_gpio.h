#ifndef __EM_GPIO_H__
#define __EM_GPIO_H__

#include <main.h>
#include <stdio.h>

void digitalWrite(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, int PinState);
int digitalRead(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#endif
