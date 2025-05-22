#include "em_hal.h"
#include "em_gpio.h"
#include "em_adc.h"
#include "em_device.h"

#define EPISON 1e-7 //判断温度值是否有效
bool need_report = false;

void Hal_Init(void)
{
	digitalWrite(LED_GPIO_Port,LED_Pin,1);//设置LED熄灭
}

/**
  * @brief LED运行逻辑
  * @param  类型
  * @retval 无
  */
void Run_Led(led_type_e type)
{
	switch(type)
	{
		case LED_CONNECT:
			digitalWrite(LED_GPIO_Port,LED_Pin,0);
			break;
		case LED_DISCONNECT:
			digitalWrite(LED_GPIO_Port,LED_Pin,1);
			break;
		case LED_WARN:
			digitalWrite(LED_GPIO_Port,LED_Pin,0);
			vTaskDelay(200);
			digitalWrite(LED_GPIO_Port,LED_Pin,1);
			vTaskDelay(200);
			digitalWrite(LED_GPIO_Port,LED_Pin,0);
			vTaskDelay(200);
			digitalWrite(LED_GPIO_Port,LED_Pin,1);
			vTaskDelay(200);
			break;
		case LED_PRINTER_START:
			digitalWrite(LED_GPIO_Port,LED_Pin,0);
			vTaskDelay(200);
			digitalWrite(LED_GPIO_Port,LED_Pin,1);
			vTaskDelay(200);
			break;
		case LED_BLE_INIT:
			digitalWrite(LED_GPIO_Port,LED_Pin,0);
			vTaskDelay(10);
			digitalWrite(LED_GPIO_Port,LED_Pin,1);
			vTaskDelay(10);
			break;
		default:
			break;
	}
}

/**
  * @brief 打印机缺纸检测
  * @param  
  * @retval 
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin&GPIO_PIN_8)
	{
		printf("lose paper\r\n");
		need_report = true; //需要上报
		set_paper_state(PAPER_STATUS_LACK);     //设置缺纸状态
	}
}

//读取缺纸状态
void Read_paper_state(void)
{
	if(digitalRead(VHINT_GPIO_Port,VHINT_Pin)==LOW)//0
	{
		set_paper_state(PAPER_STATUS_NORMAL);
	}else
	{
		set_paper_state(PAPER_STATUS_LACK);
	}
	if(get_device_state()->papre_state==PAPER_STATUS_LACK){
		need_report = true;//缺纸需要上报
		//printf("lack paper");
	}
}

/**
  * @brief 判断当前是否缺纸
  * @param  
  * @retval 
  */
bool read_if_need_add_paper(void)
{
	if(need_report)
	{
		need_report = false;	//这样写为了保证即时回到false,系统会即使捕捉状态
		return true;
	}else
	{
		return false;
	}
}


/**
  * @brief 读取当前电量
  * @param  
  * @retval 
  */
void Read_battery(void)
{
	get_device_state()->battery = map(get_adc_volts()*2000,3300,4200,0,100);
	if(get_device_state()->battery>100){
		get_device_state()->battery = 100;
	}
	printf("battery = %d%%\r\n",get_device_state()->battery);
}

/**
  * @brief 读取当前温度
  * @param  
  * @retval 
  */
void Read_temperatrue(void)
{
	float temperture = get_adc_temperture();
	if(temperture>EPISON){
		get_device_state()->temperture = temperture;
	}else{
		get_device_state()->temperture = 0;
	}
	printf("temperture = %d\r\n",get_device_state()->temperture);
}

void read_hal_state(void)
{
	Read_battery();
	Read_temperatrue();
	Read_paper_state();
}
