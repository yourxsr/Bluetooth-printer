#include "em_device.h"

device_state_t g_device_state;


void device_state_Init(void)
{
	memset(&g_device_state,0,sizeof(device_state_t));//将g_device_state全部置0
	g_device_state.battery = 100;
	g_device_state.papre_state = PAPER_STATUS_NORMAL;
	g_device_state.printer_state =PRINTER_STATUS_INIT;
	g_device_state.read_ble_finish = false;
	g_device_state.temperture = 30;
}

//获取设备状态
device_state_t* get_device_state(void)
{
	return &g_device_state;
}

//设置打印纸状态
void set_paper_state(paper_state_e papre_state)
{
	g_device_state.papre_state = papre_state;
}

//设置蓝牙数据是否接收完成
void set_read_ble_finish(bool finish)
{
	g_device_state.read_ble_finish = finish;//设置数据要根据具体的结构体设置
}

