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

int cmd_index = 0;//cmd_index索引
uint8_t cmd_buffer[100];//命令缓存区
bool need_clean_ble_status = false;//清空connected disconnected errordevice

uint32_t packcount = 0;
bool is_ble_connected = false;

bool get_ble_connect(void)
{
  return is_ble_connected;
}


void clean_pack_count(void)
{
	packcount = 0;
}

uint32_t get_pack_count(void)
{
	return packcount;
}

void uart_cmd_handle(uint8_t data)
{
	cmd_buffer[cmd_index++] = data;
	char *ptr_char = (char *)cmd_buffer;
	if(now_ble_init_step==BLE_INIT_FINISH)
	{
		if(strstr(ptr_char,"CONNECTED")!=NULL){
			need_clean_ble_status = true;//设置当前连接成功
			is_ble_connected = true;
			Run_Led(LED_CONNECT);
		}
		if(strstr(ptr_char,"DISCONNECTED")!=NULL){
			need_clean_ble_status = true;
			is_ble_connected = false;
			Run_Led(LED_CONNECT);
		}
		if(strstr(ptr_char,"DEVICE ERROR")!=NULL){
			need_clean_ble_status = true;
			Run_Led(LED_CONNECT);
		}
		//手机发送过来的指令
		if(cmd_index==5){
			//打印密度设置
			if(cmd_buffer[0]==0xA5&&cmd_buffer[1]==0xA5&&cmd_buffer[2]==0xA5&&cmd_buffer[3]==0xA5){
				if(cmd_buffer[4]==1){
					//set_heat_density(30);
				}else if(cmd_buffer[4]==2){
					//set_heat_density(60);
				}else{
					//set_heat_density(100);
				}
				cmd_index = 0;//初始化为0
				memset(cmd_buffer,0,sizeof(cmd_buffer));//清空缓存区数据
				return;//跳出当前函数
			}
			//开始打印指令--蓝牙接收数据完成
			if(cmd_buffer[0]==0xA6&&cmd_buffer[1]==0xA6&&cmd_buffer[2]==0xA6&&cmd_buffer[3]==0xA6){
				//set_read_ble_finish(true);
				printf("read finish = %d\n", packcount);//显示接收了多少个数据包
				cmd_index = 0;//初始化为0
				memset(cmd_buffer,0,sizeof(cmd_buffer));//清空缓存区数据
				return;//跳出当前函数
			}
		}
		if(cmd_index>=48){//获取了48个字节的数据存储在消息队列当中
			packcount++;
			//write_to_printbuffer(cmd_buffer, cmd_index);
			cmd_index = 0;//初始化为0
			memset(cmd_buffer,0,sizeof(cmd_buffer));//清空缓存区数据
		}
	}else{
		if(strstr(ptr_char,"OK\r\n")!=NULL){//一次只能修改一个--所有的指令都会返回OK
			if(now_ble_init_step==BLE_IN_AT_MODE)
				now_ble_init_step = BLE_IN_AT_MODE_SUCCESS;
			else if(now_ble_init_step==BLE_CLOSE_STATUS)
				now_ble_init_step = BLE_CLOSE_STATUS_SUCCESS;
			else if(now_ble_init_step==BLE_QUERY_STATUS)
				//查询显示设置状态功能的状态
				if(strstr(ptr_char,"AT+STATUS=0")!=NULL){
					now_ble_init_step = BLE_QUERY_STATUS0_SUCCESS;
				}else{
					now_ble_init_step = BLE_CLOSE_STATUS;//重新再次关闭
				}
			else if(now_ble_init_step==BLE_QUERY_NAME)
				if(strstr(ptr_char,"RF-CRAZY")!=NULL){
					now_ble_init_step = BLE_NEED_SET_NAME; //是默认名字则需要设置名字
				}else{
					now_ble_init_step = BLE_NONEED_SET_NAME;
				}
			else if(now_ble_init_step==BLE_SET_NAME)
				now_ble_init_step = BLE_SET_NAME_SUCCESS;
			else if(now_ble_init_step==BLE_OUT_AT_MODE)
				now_ble_init_step = BLE_INIT_FINISH;
			else if(now_ble_init_step==BLE_RESET)//错误重置
				now_ble_init_step = BLE_INIT_START;

			//每次处理完之后都要清空
			cmd_index = 0;//初始化为0
			memset(cmd_buffer,0,sizeof(cmd_buffer));//清空缓存区数据
			return;
		}
		if(strstr(ptr_char,"ERROR\r\n")!=NULL){
			now_ble_init_step = BLE_RESET;
		}
		if(cmd_index>sizeof(cmd_buffer))
		{
			cmd_index=0;
		}
	}
}

int retry_count = 0;//循环次数
void ble_Init(void)	//蓝牙初始化
{
	while(1)
	{
		retry_count++;
		vTaskDelay(50);
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
						break;
		
					case BLE_RESET:
						printf("BLE: Resetting module with Exit AT mode\n");
						HAL_UART_Transmit(&huart2, (uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xFFFF);
						break;
				}
		
				printf("Current BLE step: %d\n", now_ble_init_step);
				Run_Led(LED_BLE_INIT);
				if(now_ble_init_step==BLE_INIT_FINISH)
					{break;}//退出当前while循环
			}
		
			//是否改名，需要重启
			if (if_need_reset_ble) {
				printf("BLE: Configuration complete. Please reboot.\n");
			} else {
				printf("BLE: Configuration complete. Ready to use.\n");
			}
			vTaskDelay(1000);
			cmd_index = 0;//初始化为0
			memset(cmd_buffer,0,sizeof(cmd_buffer));//清空缓存区数据
	
}

// 这步操作是因为厂家的蓝牙模组，现在status只关了busy、connect timeout、device start、wake up
// 所以需要把CONNECTED DISCONNECTED DEVICE ERROR这些业务无关数据清掉
void ble_status_data_clean()
{
	if(need_clean_ble_status){
		vTaskDelay(200);
		printf("clean --->%s\n", cmd_buffer);
    	cmd_index = 0;
    	memset(cmd_buffer, 0, sizeof(cmd_buffer));
    	need_clean_ble_status = false;
	}
}

