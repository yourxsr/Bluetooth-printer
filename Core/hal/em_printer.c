#include "em_printer.h"
#include "em_gpio.h"
#include "em_motor.h"
#include "em_spi.h"
#include "em_timer.h"
#include "em_device.h"
#include "em_queue.h"
#include "em_ble.h"
#include "em_hal.h"

float addTime[6] = {0};
// 点数-增加时间系数
#define kAddTime 0.001
// 根据打印头实际打印效果修改打印时间偏移值
#define STB1_ADDTIME 0
#define STB2_ADDTIME 0
#define STB3_ADDTIME 0
#define STB4_ADDTIME 0
#define STB5_ADDTIME 0
#define STB6_ADDTIME 0

uint8_t heat_density = 64;//初始为中,打印热密度，打印的时候停留多久的时间

void set_heat_density(uint8_t density)
{
	//打印密度设置
	printf("打印密度设置 %d\n", density);
	heat_density = density;
}

static void set_stb_low(void)
{
	digitalWrite(STB1_GPIO_Port,STB1_Pin,LOW);
	digitalWrite(STB2_GPIO_Port,STB2_Pin,LOW);
	digitalWrite(STB3_GPIO_Port,STB3_Pin,LOW);
	digitalWrite(STB4_GPIO_Port,STB4_Pin,LOW);
	digitalWrite(STB5_GPIO_Port,STB5_Pin,LOW);
	digitalWrite(STB6_GPIO_Port,STB6_Pin,LOW);
}

void Printer_Init(void)
{
	//设置 VH LOW 
	//MOTOR_iNIT
	//STP INIT
	motor_Init();
	set_stb_low();
	init_spi();
	digitalWrite(VH_EN_GPIO_Port,VH_EN_Pin,LOW);
}

//开始打印,开启VH LAT,这里会循环操作,每次STB都要失能,开启打印超时监听
void Printing_Init(void)
{
	open_printer_timeout_timer();
	set_stb_low();
	digitalWrite(LAT_GPIO_Port,LAT_Pin,HIGH);
	digitalWrite(VH_EN_GPIO_Port,VH_EN_Pin,HIGH);
}

//打印停止
static void stop_printing(void)
{
	close_printer_timeout_timer();
	digitalWrite(VH_EN_GPIO_Port,VH_EN_Pin,LOW);
	set_stb_low();
	digitalWrite(LAT_GPIO_Port,LAT_Pin,HIGH);
}

//清空计算的点数加热时间
void clearAddTime(void)
{
    addTime[0] = addTime[1] = addTime[2] = addTime[3] = addTime[4] = addTime[5] = 0;
}

/**
 * @brief 发送一行数据
 *
 * @param data
 */
static void send_one_line_data(uint8_t *data)
{
	//根据发送过来的数据计算加热时间
	//将数据通过SPI发送到打印头
	//锁存
	//延时
	//拉高
	float tmpAddTime = 0;
	clearAddTime();
	 printf("send_one_line_data 输入数据[0-7]:");
    for(int k=0; k<8; k++) {
        printf(" %02X", data[k]);
    }
    printf("\r\n");
	for(uint8_t i = 0;i<6;i++)
	{
		for(uint8_t j = 0;j<8;j++)
		{
			addTime[i]+=data[i*8+j];//每个通道获取8Byte通道计算
		}
		tmpAddTime = addTime[i]*addTime[i];
		addTime[i] = kAddTime*tmpAddTime;
	}
	spiCommand(data,TPH_DI_LEN);
	digitalWrite(LAT_GPIO_Port,LAT_Pin,LOW);//LAT锁存
	us_delay(LAT_TIME);
	digitalWrite(LAT_GPIO_Port,LAT_Pin,HIGH);
}


/**
 * @brief 通道打印运行,轮流运行每个通道，并停留各个计算的加热时间
 *
 * @param now_stb_num
 */
static void run_stb(uint8_t now_stb_num)
{
	//set_stb_low(); // 先关闭所有STB
	switch(now_stb_num)
	{
		case 0:
			digitalWrite(STB1_GPIO_Port,STB1_Pin,HIGH);
			us_delay((PRINT_TIME+addTime[0]+STB1_ADDTIME)*((double)heat_density/100));
			digitalWrite(STB1_GPIO_Port,STB1_Pin,LOW);
			us_delay(PRINT_END_TIME);
			break;
		case 1:
			digitalWrite(STB2_GPIO_Port,STB2_Pin,HIGH);
			us_delay((PRINT_TIME+addTime[1]+STB2_ADDTIME)*((double)heat_density/100));
			digitalWrite(STB2_GPIO_Port,STB2_Pin,LOW);
			us_delay(PRINT_END_TIME);
			break;
		case 2:
			digitalWrite(STB3_GPIO_Port,STB3_Pin,HIGH);
			us_delay((PRINT_TIME+addTime[2]+STB3_ADDTIME)*((double)heat_density/100));
			digitalWrite(STB3_GPIO_Port,STB3_Pin,LOW);
			us_delay(PRINT_END_TIME);
			break;
		case 3:
			digitalWrite(STB4_GPIO_Port,STB4_Pin,HIGH);
			us_delay((PRINT_TIME+addTime[3]+STB4_ADDTIME)*((double)heat_density/100));
			digitalWrite(STB4_GPIO_Port,STB4_Pin,LOW);
			us_delay(PRINT_END_TIME);
			break;
		case 4:
			digitalWrite(STB5_GPIO_Port,STB5_Pin,HIGH);
			us_delay((PRINT_TIME+addTime[4]+STB5_ADDTIME)*((double)heat_density/100));
			digitalWrite(STB5_GPIO_Port,STB5_Pin,LOW);
			us_delay(PRINT_END_TIME);
			break;
		case 5:
			digitalWrite(STB6_GPIO_Port,STB6_Pin,HIGH);
			us_delay((PRINT_TIME+addTime[5]+STB6_ADDTIME)*((double)heat_density/100));
			digitalWrite(STB6_GPIO_Port,STB6_Pin,LOW);
			us_delay(PRINT_END_TIME);
			break;
		default:
			break;
	}
}

/**
 * @brief 移动电机&开始打印&是否立即暂停
 *
 * @param need_stop
 * @param stbnum
 */
bool move_and_start_std(bool need_stop, uint8_t stbnum)
{
	if(need_stop==true)//是否立即停止打印
	{
		printf("停止打印\r\n");
		motor_stop();
		stop_printing();
		return true;
	}
	motor_run();
	if(stbnum==ALL_STB_NUM)
	{
		//所有通道打印
		for(uint8_t index = 0;index<6;index++)
		{
			run_stb(index);
			//把电机运行信号插入通道加热中,减少打印卡断和耗时
			if(index == 1||index==3||index==5)
			{
				motor_run();//一起算下来走了4步
			}
		}
		//motor_run_step(3);
	}else
	{
		//单通道打印
		run_stb(stbnum);
		motor_run_step(3);
	}
	return false;
}

/**
 * @brief 打印错误检查
 * 
 * @param need_report 是否需BLE上报
 * @return true 打印出错
 * @return false 打印正常
 */
//打印超时，缺纸，温度异常
bool printing_error_check(bool need_report)
{
	if(get_printer_timeout_status())
	{
		printf("打印超时\r\n");
		return true;
	}
	if(get_device_state()->papre_state == PAPER_STATUS_LACK)
	{
		if(need_report)
		{
			Clean_ble_rw_t();//清除打印缓存区标志位
			ble_report();//蓝牙上报
		}
		printf("缺纸\r\n");
		Run_Led(LED_WARN);
		return true;
	}
	if(get_device_state()->temperture>65)
	{
		if(need_report)
		{
			Clean_ble_rw_t();//清除打印缓存区标志位
			ble_report();//蓝牙上报
		}
		printf("温度异常\r\n");
		Run_Led(LED_WARN);
		return true;
	}
	return false;
}

//void start_printing(uint8_t *data, uint32_t len)
//{
//	
//}

/**
 * @brief 可变队列打印
 *
 */
void start_printing_by_queuebuf(void)
{
	uint8_t *pdata = NULL;
	uint32_t printer_count = 0;
	Printing_Init();
	while(1)
	{
		if(get_ble_rw_leftline()>0)
		{
			pdata = read_from_buffer();//读取数据
			if(pdata!=NULL)
			{
				printer_count++;
				printf("队列->打印: 行#%d, 数据[0-7]:", printer_count); // %lu 用于 unsigned long
				for(int k=0; k<8; k++) { // 记录前8字节
						printf(" %02X", pdata[k]);
				}
				send_one_line_data(pdata);
				if(move_and_start_std(false,ALL_STB_NUM))
				{
					break;
				}
			}
		}else
		{
			//当前缓存区为空
			if(move_and_start_std(true,ALL_STB_NUM))//立马暂停
				break;
		}
		if(get_printer_timeout_status())//打印机超时则退出打印
			break;
		if(printing_error_check(true))//需要上报返回true,说明有问题
			break;
	}
	motor_run_step(140);//往前移动方便下一个
	motor_stop();
	clean_blepack_count();//应该要先获取再清除
	printf("printer finish !!! read=%d printer:%d\r\n",get_blepack_count(),printer_count);
}

/**
 * @brief 单通道数组打印
 *
 * @param stbnum
 * @param data
 * @param len
 */
void start_printing_by_onestb(uint8_t stbnum, uint8_t *data, uint32_t len)
{
	uint32_t offset = 0;//数据偏移量
	uint8_t *ptr = data;
	bool need_stop = false;
	Printing_Init();
	while(1)
	{
		printf("printer %d\r\n",offset);
		if(len>offset)
		{
			send_one_line_data(ptr);//一次只能发48Byte
			offset+=TPH_DI_LEN;//移动数据偏移量
			ptr+=TPH_DI_LEN;//移动指针
		}else
		{
			need_stop = true;
		}
		if(move_and_start_std(need_stop,stbnum))
			break;
		if(get_printer_timeout_status())
			break;
		if(printing_error_check(false))//这个不需要上报--单通道打印
			break;
	}
	printf("printer end\r\n");
	motor_run_step(40);//通道之间的空白差距
	motor_stop();
	printf("printer end2\r\n");
}

static void setDebugData(uint8_t *print_data)//向打印缓存区写入测试数据
{
	  //memset(print_data, 0, 48 * 6);//清空all
    for (uint32_t cleardata = 0; cleardata < 48 * 5; ++cleardata)
    {
        print_data[cleardata] = 0x55;
    }
}

void testSTB()
{
    uint8_t print_data[48 * 6];
    uint32_t print_len;
    printf("开始打印打印头选通引脚测试\n顺序: 1  2  3  4  5  6");
    print_len = 48 * 5;
    setDebugData(print_data);
    start_printing_by_onestb(0, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(1, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(2, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(3, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(4, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(5, print_data, print_len);
    printf("测试完成");
}

