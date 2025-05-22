#include "em_button.h"
#include "em_gpio.h"
#include "em_keymotor.h"

//短按，长按中间，长按结束

#define button_slow_time 1000

bool isPress = false;	//是否按下
uint32_t clickTime = 0;//按下时间
bool isLong_Click_Run = false;

void Key_Init(void)
{
	//无 CUBEMX中已经写好
}

/**
  * @brief 获取当前时间戳
  * @param  
  * @retval 
  */
static uint32_t millis(){
	return HAL_GetTick();//获取当前Tick时间
}

void Key_run(void)
{
	//按键消抖
	if(isPress == false)
	{
		if(digitalRead(BTN_GPIO_Port,BTN_Pin)==LOW)
		{
			vTaskDelay(10);
			if(digitalRead(BTN_GPIO_Port,BTN_Pin)==LOW)
			{
				//获取当前时间
				clickTime = millis();
				isPress=true;
			}
		}
	}
	if(isPress==true)
	{
		//是否按下判断
		if(digitalRead(BTN_GPIO_Port,BTN_Pin)==HIGH)
		{
			//判断时长
			if((millis()-clickTime)>button_slow_time)
			{
				printf("click long free!\r\n");
				isLong_Click_Run=false;//长按结束
				click_long_free();
			}else
			{
				printf("click slow free!\r\n");
				click_slow();
			}
			isPress=false;//!!注意已经松手
		}else
		{
			if((millis()-clickTime)>button_slow_time)//未松手且长按中
			{
				if(isLong_Click_Run==false)//之前非长按
				{
					printf("click_long_ing\r\n");
					isLong_Click_Run=true;
					long_click_run();//走纸
				}
			}	
		}
	}
}
