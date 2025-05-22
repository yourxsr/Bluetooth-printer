#ifndef __EM_DEIVE_H__
#define __EM_DEIVE_H__

#include <main.h>
#include "em_config.h"

typedef enum{
	PRINTER_STATUS_INIT = 0,//初始状态
	PRINTER_STATUS_START,//已经开始启动
	PRINTER_STATUS_WORKING,//正在执行打印任务
	PRINTER_STATUS_FINISH,//已经完成当前的打印任务，处于空闲状态
}printer_state_e;//打印机打印状态

typedef enum{
	PAPER_STATUS_NORMAL = 0,//打印机纸张充足
	PAPER_STATUS_LACK,//纸张不足或者已经用完
}paper_state_e;

//定义设备状态结构体
typedef struct
{
	uint8_t battery;//电量
	uint8_t temperture;//温度
	paper_state_e papre_state;//缺纸状态
	printer_state_e printer_state;//打印状态
	bool read_ble_finish;//蓝牙接收数据完成
}device_state_t;

void device_state_Init(void);
device_state_t* get_device_state(void);
void set_paper_state(paper_state_e papre_state);
void set_read_ble_finish(bool finish);

#endif
