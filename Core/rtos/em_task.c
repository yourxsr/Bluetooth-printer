#include "em_task.h"
#include "em_adc.h"
#include "em_button.h"
#include "em_motor.h"
#include "em_printer.h"
#include "em_keymotor.h"

//获取定时器定时到了的true，进行获取及上报状态
void run_report(void)
{
	if(get_state_timeout())//获取是否进入超时回调函数
	{
		clean_state_timeout();//清除超时标志
		read_hal_state();//获取及更新所有状态
		if(get_ble_connect())// 仅当蓝牙读取完成时才上报
		{
			ble_report();
		}
	}
	if(read_if_need_add_paper())
	{
		//缺纸中断产生
		printf("report device status : paper irq\r\n");
    ble_report();//蓝牙更新状态
	}
}

//处理打印相关事件
void run_printer(void)
{
	device_state_t *pdevice = get_device_state();
	//#ifdef START_PRINTER_WHEN_FINISH_RAED //模式1：数据接收完成后进行打印
	if(pdevice->read_ble_finish==true)//蓝牙读取数据完成后
	{
		if (pdevice->printer_state == PRINTER_STATUS_FINISH ||
                pdevice->printer_state == PRINTER_STATUS_INIT){
			pdevice->read_ble_finish = false;
			pdevice->printer_state = PRINTER_STATUS_START;
			ble_report();//更新设置的数据
			printf("report device status : printing start %d\r\n",get_ble_rw_leftline());
			Run_Led(LED_PRINTER_START);
		}
	}
    if (pdevice->printer_state == PRINTER_STATUS_START)
    {
        // 正常打印
        start_printing_by_queuebuf();// 从队列缓冲区打印数据
				//set_read_ble_finish(true);
        pdevice->printer_state = PRINTER_STATUS_FINISH;
    }
}

void task_report( void * param)
{
	int count = 0;
	printf("task report init\r\n");
	for( ;; )
    {
			run_report();
			vTaskDelay(100);
			count++;
			if(count>=50){
				count = 0;
				printf("task_report run\r\n");
			}
    }
}

void task_button( void * param)
{
	int count = 0;
	printf("task button init\r\n");
	for( ;; )
    {
			Key_run();
			vTaskDelay(20);
			count++;
			if(count>=250){
				count = 0;
				printf("task_button run\r\n");
			}
    }
}

void task_printer( void * param)
{
	int count = 0;
	ble_Init();//蓝牙初始化，进入AT模式
	printf("task printer init\r\n");
	for( ;; )
    {
			ble_status_data_clean();//清空connect dis error
			run_printer();
			vTaskDelay(1);
			if(get_printer_test())
			{
				testSTB();
				set_printer_test(false);
			}
			count++;
			if(count>=5000){
				count = 0;
				printf("task_printer run\r\n");
			}
    }
}

void task_Init(void)
{
	printf("init_task\r\n");
	device_state_Init();//设备状态
	adc_Init();//adc初始化
	Hal_Init();//led熄灭
	Timer_Init();//创建定时器--10s上报状态
	Queue_Init();//清空缓存区,创建互斥量
	Key_Init();//NULL
	Printer_Init();//打印 电机 VH SPI
//	xTaskCreate(task_report,"TaskReport",128,NULL,1,NULL);
//	xTaskCreate(task_button,"TaskButton",128,NULL,0,NULL);
//	xTaskCreate(task_printer,"TaskPrinter",256,NULL,2,NULL);
	
	if (xTaskCreate(task_report, "TaskReport", 128, NULL, 1, NULL) != pdPASS) {
        printf("ERROR: Failed to create TaskReport!\n");
        while(1);  // 死循环，防止继续执行（或采取恢复措施）
    }

    if (xTaskCreate(task_button, "TaskButton", 128, NULL, 0, NULL) != pdPASS) {
        printf("ERROR: Failed to create TaskButton!\n");
        while(1);
    }

    if (xTaskCreate(task_printer, "TaskPrinter", 256, NULL, 2, NULL) != pdPASS) {
        printf("ERROR: Failed to create TaskPrinter!\n");
        while(1);
    }

    //printf("All tasks created successfully!\n");
}
	
