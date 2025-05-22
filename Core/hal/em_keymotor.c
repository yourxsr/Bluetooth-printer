#include "em_motor.h"
#include "em_device.h"
#include "em_hal.h"

bool printer_test = false;//可以进行测试
bool is_long_ever_click = false;//判断是否进入过长按了？

//短按事件
void click_slow(void)
{
	printf("Button 单击\r\n");
	printer_test = true;
}

bool get_printer_test(void)
{
	return printer_test;
}

void set_printer_test(bool state)
{
	printer_test = state;
}

//长按事件结束
void click_long_free(void)
{
		//设置电机停止运行
	printf("停止走纸\r\n");
	is_long_ever_click=false;//长按结束
	motor_stop();
}


//长按中
void long_click_run(void)
{
	if(is_long_ever_click==true)
		return;
	is_long_ever_click=true;
	bool need_warn = false;
	printf("Button 长按!\r\n");
	device_state_t* pcdevice = get_device_state();
	//获取当前状态，在打印机有纸张
	//在当前打印机打印完成或者当前打印机在初始化的情况下运行
	if(pcdevice->papre_state==PAPER_STATUS_NORMAL)
	{
		if(pcdevice->printer_state==PRINTER_STATUS_FINISH||pcdevice->printer_state==PRINTER_STATUS_INIT)
		{
			printf("开始走纸\r\n");
			motor_start();//打印机开始旋转
		}else
		{
			need_warn = true;
		}
	}else
	{
		need_warn = true;
	}
	
	if(need_warn)
	{
		Run_Led(LED_WARN);//200ms闪烁2次
	}
}
