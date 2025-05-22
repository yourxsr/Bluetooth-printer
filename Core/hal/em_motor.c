#include "em_motor.h"
#include "em_gpio.h"
#include "timers.h"

// AP IN1
// AM IN2
// BP IN3
// BM IN4

uint8_t motor_pos = 0;

TimerHandle_t Motor_TimerHandle = NULL;//保证开机第一次启用

uint8_t motor_table[8][4] =
    {
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {1, 0, 1, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 1, 0, 0}};

void motor_Init(void)
{
	//关闭打印机加热驱动
	//digitalWrite(VH_EN_GPIO_Port,VH_EN_Pin,LOW);
	
	digitalWrite(MOTOR_IN1_GPIO_Port,MOTOR_IN1_Pin,LOW);
	digitalWrite(MOTOR_IN2_GPIO_Port,MOTOR_IN2_Pin,LOW);
	digitalWrite(MOTOR_IN3_GPIO_Port,MOTOR_IN3_Pin,LOW);
	digitalWrite(MOTOR_IN4_GPIO_Port,MOTOR_IN4_Pin,LOW);
}

void read_motor_timer_callbackfun(TimerHandle_t xTimer)
{
	digitalWrite(MOTOR_IN1_GPIO_Port,MOTOR_IN1_Pin, motor_table[motor_pos][0]);
	digitalWrite(MOTOR_IN2_GPIO_Port,MOTOR_IN2_Pin, motor_table[motor_pos][1]);
	digitalWrite(MOTOR_IN3_GPIO_Port,MOTOR_IN3_Pin, motor_table[motor_pos][2]);
	digitalWrite(MOTOR_IN4_GPIO_Port,MOTOR_IN4_Pin, motor_table[motor_pos][3]);
	motor_pos++;
	if (motor_pos >= 8)
	{
			motor_pos = 0;
	}
}

void motor_start(void)
{
	//自动加载，循环
	if(Motor_TimerHandle==NULL)
	{
		Motor_TimerHandle = xTimerCreate("MotorTimer",2,pdTRUE,NULL,read_motor_timer_callbackfun);
		
	}
	xTimerStart(Motor_TimerHandle,100);//开启定时器
}

/**
  * @brief 前进一步
  * @param  
  * @retval 
  */
void motor_run(void)
{
		digitalWrite(MOTOR_IN1_GPIO_Port,MOTOR_IN1_Pin, motor_table[motor_pos][0]);
    digitalWrite(MOTOR_IN2_GPIO_Port,MOTOR_IN2_Pin, motor_table[motor_pos][1]);
    digitalWrite(MOTOR_IN3_GPIO_Port,MOTOR_IN3_Pin, motor_table[motor_pos][2]);
    digitalWrite(MOTOR_IN4_GPIO_Port,MOTOR_IN4_Pin, motor_table[motor_pos][3]);
    motor_pos++;
    if (motor_pos >= 8)
    {
        motor_pos = 0;
    }
}

/**
  * @brief 前进指定步数
  * @param  
  * @retval 
  */
void motor_run_step(uint32_t steps)
{
	while(steps)
	{
		digitalWrite(MOTOR_IN1_GPIO_Port,MOTOR_IN1_Pin, motor_table[motor_pos][0]);
		digitalWrite(MOTOR_IN2_GPIO_Port,MOTOR_IN2_Pin, motor_table[motor_pos][1]);
		digitalWrite(MOTOR_IN3_GPIO_Port,MOTOR_IN3_Pin, motor_table[motor_pos][2]);
		digitalWrite(MOTOR_IN4_GPIO_Port,MOTOR_IN4_Pin, motor_table[motor_pos][3]);
		motor_pos++;
		if (motor_pos >= 8)
		{
			motor_pos = 0;
		}
		//电机运行每行之间要等待一会
		us_delay(MOTOR_WATI_TIME);
		steps--;
	}
}

void motor_stop(void)
{
	//关闭定时器
	//把四个通道引脚置0
	digitalWrite(MOTOR_IN1_GPIO_Port,MOTOR_IN1_Pin,LOW);
	digitalWrite(MOTOR_IN2_GPIO_Port,MOTOR_IN2_Pin,LOW);
	digitalWrite(MOTOR_IN3_GPIO_Port,MOTOR_IN3_Pin,LOW);
	digitalWrite(MOTOR_IN4_GPIO_Port,MOTOR_IN4_Pin,LOW);
	
	if(Motor_TimerHandle!=NULL)
	{
//		xTimerDelete(Motor_TimerHandle,0xffff);//删除定时器
//		Motor_TimerHandle = NULL;
		xTimerStop(Motor_TimerHandle,0xffff);
	}
}
