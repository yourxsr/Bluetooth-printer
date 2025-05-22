#include "em_ble.h"
#include "em_config.h"
#include "em_hal.h"
#include "em_device.h"
#include "usart.h"
#include "em_queue.h"
#include "em_printer.h"

bool bleConnected = true;
uint32_t packcount = 0;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

// 定义一个字符串
char *ble_in_at_mode = "+++";
char *ble_set_status = "AT+STATUS=0\r\n"; //关闭设备状态显示
char *ble_query_status = "AT+STATUS?\r\n"; //关闭设备状态显示
char *ble_query_name = "AT+NAME?\r\n"; //AT+NAME=RF-CRAZY\r\nOK
char *ble_set_name = "AT+NAME=Mini-Printer\r\n"; //OK 大写？
//char *ble_set_name = "AT+NAME=RF-CRAZY\r\n"; //OK 大写？
char *ble_out_at_mode = "AT+EXIT\r\n";

typedef enum{
        BLE_INIT_START = 0,
        BLE_IN_AT_MODE,
        BLE_IN_AT_MODE_SUCCESS,
        BLE_CLOSE_STATUS,
        BLE_CLOSE_STATUS_SUCCESS,
		BLE_QUERY_STATUS,
		BLE_QUERY_STATUS0_SUCCESS,
        BLE_QUERY_NAME,
        BLE_NEED_SET_NAME,
        BLE_NONEED_SET_NAME,
        BLE_SET_NAME,
        BLE_SET_NAME_SUCCESS,
        BLE_OUT_AT_MODE,
        BLE_INIT_FINISH,
		BLE_RESET,
}e_ble_init_step;

e_ble_init_step g_ble_init_step = BLE_INIT_START;
bool need_reboot_ble = false;


void clean_blepack_count(void){
    packcount = 0;
}


uint32_t get_blepack_count(void){
    return packcount;
}

bool get_ble_connect(void){
    return bleConnected;
}

void ble_report(void)
{
	if(get_ble_connect())//当蓝牙连接成功时，通过usart2上报给蓝牙，蓝牙发给手机
	{
		device_state_t* pdevice = get_device_state();//获取当前设备状态
		uint8_t status[4];
		status[0] = pdevice->battery;
		status[1] = pdevice->temperture;
		status[2] = pdevice->papre_state;
		status[3] = pdevice->printer_state;
		//这个数组本来就可以指向首地址吧
		//printf("卡在这里了\n");
		HAL_UART_Transmit(&huart2,(uint8_t*)&status,sizeof(status),0xffff);
//		if (tx_status != HAL_OK) {
//    printf("ble_2 TX 失败! 状态: %d\r\n", tx_status);
//		}
	}
}

int cmd_index = 0;
uint8_t cmd_buffer[100];
bool need_clean_ble_status = false;

void uart_cmd_handle(uint8_t data){
        cmd_buffer[cmd_index++] = data;
        char *ptr_char = (char*)cmd_buffer;
        if(g_ble_init_step == BLE_INIT_FINISH){
												if(strstr(ptr_char, "CONNECTED") != NULL){
													need_clean_ble_status = true;
													Run_Led(LED_CONNECT);
												}
												if(strstr(ptr_char, "DISCONNECTED") != NULL){
													need_clean_ble_status = true;
													Run_Led(LED_DISCONNECT);
												}
												if(strstr(ptr_char, "DEVICE ERROR") != NULL){
													need_clean_ble_status = true;
													Run_Led(LED_CONNECT);
												}
                        if(cmd_index == 5){
                                 if(cmd_buffer[0] == 0xA5 && cmd_buffer[1] == 0xA5 && cmd_buffer[2] == 0xA5 && cmd_buffer[3] == 0xA5){
                                                 if(cmd_buffer[4] == 1){
                                                                 set_heat_density(30);
                                                 }else if(cmd_buffer[4] == 2){
                                                                 set_heat_density(60); 
                                                 }else{
                                                                 set_heat_density(100);
                                                 }
                                                 cmd_index = 0;
																								 memset(cmd_buffer,0,sizeof(cmd_buffer));
                                                 return;
                                        }
                                 if(cmd_buffer[0] == 0xA6 && cmd_buffer[1] == 0xA6 && cmd_buffer[2] == 0xA6 && cmd_buffer[3] == 0xA6){
                                                //printf("finish1 set set_read_ble_finish(true)\r\n");
																								set_read_ble_finish(true);
                                                printf("---->read finish 1 = %d\r\n",packcount);
                                                cmd_index = 0;
																								memset(cmd_buffer,0,sizeof(cmd_buffer));
                                                return;
                                 }
                        }
												if(cmd_index >= 5){
													if(cmd_buffer[cmd_index-1] == 0x01){//加入打印完成判断
														if(cmd_buffer[cmd_index-2] == 0xA6 && cmd_buffer[cmd_index-3] == 0xA6 && cmd_buffer[cmd_index-4] == 0xA6 && cmd_buffer[cmd_index-5] == 0xA6){
															printf("---->read finish 2 = %d\r\n",packcount);
															cmd_index = 0;
															memset(cmd_buffer,0,sizeof(cmd_buffer));
															set_read_ble_finish(true);
											
															return;
														}
													}
												}
													
                        if(cmd_index >= 48){
                                packcount++;
//																
                                Write_to_buffer(cmd_buffer,cmd_index);
                                cmd_index = 0;
                                memset(cmd_buffer,0,sizeof(cmd_buffer));
																//printf("packcount = %d\r\n",packcount);	
																//主要是这里会导致混乱
                        }
        }else{
								
                if(strstr(ptr_char, "OK\r\n") != NULL){
					if(g_ble_init_step == BLE_IN_AT_MODE){
									g_ble_init_step = BLE_IN_AT_MODE_SUCCESS;
					}else if(g_ble_init_step == BLE_CLOSE_STATUS){
									g_ble_init_step = BLE_CLOSE_STATUS_SUCCESS;
					}else if(g_ble_init_step == BLE_QUERY_NAME){
									if(strstr(ptr_char,"RF-CRAZY") != NULL){
									//if(strstr(ptr_char,"Mini-Printer") != NULL){
										g_ble_init_step = BLE_NEED_SET_NAME;
									}else{
										g_ble_init_step = BLE_NONEED_SET_NAME;
									}
					}else if(g_ble_init_step == BLE_SET_NAME){
									g_ble_init_step = BLE_SET_NAME_SUCCESS;
					}else if(g_ble_init_step == BLE_OUT_AT_MODE){
									g_ble_init_step = BLE_INIT_FINISH;
					}else if(g_ble_init_step == BLE_RESET){
									g_ble_init_step = BLE_INIT_START;
					}else if(g_ble_init_step == BLE_QUERY_STATUS){
									if(strstr(ptr_char,"AT+STATUS=0") != NULL){
										g_ble_init_step = BLE_QUERY_STATUS0_SUCCESS;
									}else{
										g_ble_init_step = BLE_CLOSE_STATUS;
									}
					}
					cmd_index = 0;
					memset(cmd_buffer,0,sizeof(cmd_buffer));
					return;
                }
				if(strstr(ptr_char, "ERROR\r\n") != NULL){
					g_ble_init_step = BLE_RESET;
				}
				if(cmd_index >= sizeof(cmd_buffer)){
						cmd_index = 0;
				}
        }
}



int retry_count = 0;

void ble_Init()
{
                while(1){
												retry_count ++;
                        vTaskDelay(50);
                        if(g_ble_init_step == BLE_INIT_START || g_ble_init_step == BLE_IN_AT_MODE){
																printf("BLE:正进入AT模式\r\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_in_at_mode, strlen(ble_in_at_mode), 0xffff);
                                g_ble_init_step = BLE_IN_AT_MODE;
                        }else if(g_ble_init_step == BLE_IN_AT_MODE_SUCCESS || g_ble_init_step == BLE_CLOSE_STATUS){
																printf("BLE:正设置status为0 关闭状态显示\r\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_set_status, strlen(ble_set_status), 0xffff);
                                g_ble_init_step = BLE_CLOSE_STATUS;
                        }else if(g_ble_init_step == BLE_CLOSE_STATUS_SUCCESS || g_ble_init_step == BLE_QUERY_STATUS){
																printf("BLE:正查询状态是否为0\r\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_query_status, strlen(ble_query_status), 0xffff);
                                g_ble_init_step = BLE_QUERY_STATUS;
                        }else if(g_ble_init_step == BLE_QUERY_STATUS0_SUCCESS || g_ble_init_step == BLE_QUERY_NAME){
																printf("BLE:正查询设备名称\r\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_query_name, strlen(ble_query_name), 0xffff);
                                g_ble_init_step = BLE_QUERY_NAME;
                        }
												else if(g_ble_init_step == BLE_NEED_SET_NAME || g_ble_init_step == BLE_SET_NAME){
																printf("BLE:正设置设备名称\r\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_set_name, strlen(ble_set_name), 0xffff);
                                g_ble_init_step = BLE_SET_NAME;
                                need_reboot_ble = true;
                        }else if(g_ble_init_step == BLE_SET_NAME_SUCCESS || g_ble_init_step == BLE_NONEED_SET_NAME || g_ble_init_step == BLE_OUT_AT_MODE){
																printf("BLE:正退出AT模式\r\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xffff);
                                g_ble_init_step = BLE_OUT_AT_MODE;
                        }else if(g_ble_init_step == BLE_INIT_FINISH){
                                break;
                        }else if(g_ble_init_step == BLE_RESET){
								printf("BLE:BLE RESET 退出AT模式\r\n");
								HAL_UART_Transmit(&huart2, (uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xffff);
                                
						}
						printf("g_ble_init_step = %d\r\n",g_ble_init_step);
						Run_Led(LED_BLE_INIT);
                }
                if(need_reboot_ble){
                        printf("配置完成-请重启设备使用\r\n");
                }else{
                        printf("配置完成-可以正常使用\r\n");
												
                }
				vTaskDelay(1000);
				cmd_index = 0;
				memset(cmd_buffer,0,sizeof(cmd_buffer));
}

//这步操作是因为厂家的蓝牙模组，现在status只关了busy、connect timeout、device start、wake up
//所以需要把CONNECTED DISCONNECTED DEVICE ERROR这些业务无关数据清掉
void ble_status_data_clean(void){
	if(need_clean_ble_status){
		vTaskDelay(200);
		printf("clean --->%s\r\n",cmd_buffer);
		cmd_index = 0;
		memset(cmd_buffer,0,sizeof(cmd_buffer));
		need_clean_ble_status = false;
	}
}
