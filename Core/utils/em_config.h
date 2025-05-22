#ifndef __EM_CONFIG_H__
#define __EM_CONFIG_H__


#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"//方便不同操作系统迁移
#include <main.h>
#include <string.h>  //string
#include <stdbool.h> //bool
#include <stdio.h>

#define LOW               0x0
#define HIGH              0x1

#define PRINT_TIME 2600         //打印加热时间
#define PRINT_END_TIME 200      //冷却时间
#define MOTOR_WATI_TIME 4000    //电机一步时间
#define LAT_TIME 1              //数据锁存时间  

//#define START_PRINTER_WHEN_FINISH_RAED 1

#include "tim.h"
#define DLY_TIM_Handle (&htim1)  // Timer handle
#define hal_delay_us(nus) do { \
    __HAL_TIM_SET_COUNTER(DLY_TIM_Handle, 0); \
    __HAL_TIM_ENABLE(DLY_TIM_Handle); \
    while (__HAL_TIM_GET_COUNTER(DLY_TIM_Handle) < (nus)); \
    __HAL_TIM_DISABLE(DLY_TIM_Handle); \
} while(0)

#define us_delay(us) hal_delay_us(us)

#endif
