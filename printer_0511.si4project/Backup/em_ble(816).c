#include "em_ble.h"
#include "em_config.h"
#include "em_hal.h"
#include "usart.h"

//设置这些AT指令
char *ble_in_at_mode = "+++";//进入AT模式 OK
char *ble_set_close_status = "AT+STATUS=0\r\n";//关闭设备状态显示功能 OK
char *ble_query_status = "AT+STATUS?\r\n";//查询设备状态显示功能状态？0/1
char *ble_query_name = "AT+NAME?\r\n";//查询设备名称 AT+NAME=RF-CRAZY
char *ble_set_name = "AT+NAME=Mini-Printer\r\n";//设置设备名称 OK
char *ble_out_at_mode = "AT+EXIT\r\n";//退出AT模式

//蓝牙初始化过程中每一步的状态
typedef enum{
	BLE_INIT_START = 0,//初始化开始
	BLE_IN_AT_MODE,//进入AT模式
	BLE_IN_AT_MODE_SUCCESS,//进入AT模式成功
	BLE_CLOSE_STATUS,//关闭状态显示
	BLE_CLOSE_STATUS_SUCCESS,//关闭状态显示成功
	BLE_QUERY_STATUS,//查询当前状态
	BLE_QUERY_STATUS0_SUCCESS,//查询状态关闭成功
	BLE_QUERY_NAME,//查询设备名称
	BLE_NEED_SET_NAME,//需要设置名称
	BLE_NONEED_SET_NAME,//无需设置名称
	BLE_SET_NAME,//设置名称
	BLE_SET_NAME_SUCCESS,//设置名称成功
	BLE_OUT_AT_MODE,//退出AT模式
	BLE_INIT_FINISH,//初始化完成
	BLE_RESET,//出错重置
}e_ble_init_step;

e_ble_init_step now_ble_init_step = BLE_INIT_START;//设置保存状态变量，设置为初始值
bool if_need_reset_ble = false;//是否需要重启设备--设置新名字后需要重启

int retry_count = 0;
void ble_Init(void)	//
{
	while(1)
	{
		retry_count++;
		vTaskDelay(100);
		 switch (now_ble_init_step) {
					case BLE_INIT_START:
					case BLE_IN_AT_MODE:
						printf("BLE: Entering AT mode\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_in_at_mode, strlen(ble_in_at_mode), 0xFFFF);
						now_ble_init_step = BLE_IN_AT_MODE;
						break;
		
					case BLE_IN_AT_MODE_SUCCESS:
					case BLE_CLOSE_STATUS:
						printf("BLE: Disabling status display\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_set_close_status, strlen(ble_set_close_status), 0xFFFF);
						now_ble_init_step = BLE_CLOSE_STATUS;
						break;
		
					case BLE_CLOSE_STATUS_SUCCESS:
					case BLE_QUERY_STATUS:
						printf("BLE: Querying status\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_query_status, strlen(ble_query_status), 0xFFFF);
						now_ble_init_step = BLE_QUERY_STATUS;
						break;
		
					case BLE_QUERY_STATUS0_SUCCESS:
					case BLE_QUERY_NAME:
						printf("BLE: Querying device name\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_query_name, strlen(ble_query_name), 0xFFFF);
						now_ble_init_step = BLE_QUERY_NAME;
						break;
		
					case BLE_NEED_SET_NAME:
					case BLE_SET_NAME:
						printf("BLE: Setting device name\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_set_name, strlen(ble_set_name), 0xFFFF);
						now_ble_init_step = BLE_SET_NAME;
						if_need_reset_ble = true;
						break;
		
					case BLE_SET_NAME_SUCCESS:
					case BLE_NONEED_SET_NAME:
					case BLE_OUT_AT_MODE:
						printf("BLE: Exiting AT mode\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xFFFF);
						now_ble_init_step = BLE_OUT_AT_MODE;
						break;
		
					case BLE_INIT_FINISH:
						goto init_done;
		
					case BLE_RESET:
						printf("BLE: Resetting module\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xFFFF);
						break;
				}
		
				printf("Current BLE step: %d\n", now_ble_init_step);
				Run_Led(LED_BLE_INIT);
			}
		
		init_done:
			// 初始化完成处理
			if (if_need_reset_ble) {
				printf("BLE: Configuration complete. Please reboot.\n");
			} else {
				printf("BLE: Configuration complete. Ready to use.\n");
			}
	
}
