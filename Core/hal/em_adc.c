#include "em_adc.h"
#include "adc.h"
#include "math.h"

#define ADC_DataNum 10

uint32_t ADC1_Value[2];

//ADC初始化-->获取ADC电压值保存-->数据滤波(去max min 求avg)-->将电压转换为电量百分比

void adc_Init(void)
{
	HAL_ADC_Start(&hadc1);//开启ADC转换
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1,1000);//轮询等待ADC是否转换完成
}

/**
  * @brief 求平均值
  * @param  
  * @retval 
  */
uint32_t adc_get_avg(uint32_t *ADC_Value,uint8_t Number)
{
	uint8_t i=0;
	uint32_t ADC_Avg_Value=0;
	uint32_t Max_Value = ADC_Value[0];
	uint32_t Min_Value = ADC_Value[0];
	
	for(i=0;i<Number;i++)
	{
		ADC_Avg_Value+=ADC_Value[i];
		if(ADC_Value[i]>Max_Value) Max_Value = ADC_Value[i];
		if(ADC_Value[i]<Min_Value) Min_Value = ADC_Value[i];
	}
	ADC_Avg_Value = ADC_Avg_Value-(Max_Value+Min_Value);
	ADC_Avg_Value = ADC_Avg_Value/(Number-2);
	return ADC_Avg_Value;
}

/**
  * @brief 获取ADC通道返回值
  * @param  
  * @retval 
  */
float get_adc_volts(void)
{
	uint8_t i;
	uint32_t ADC1_Channel_0[ADC_DataNum];
	uint32_t ADC1_Channel_1[ADC_DataNum];
	
	for(i=0;i<ADC_DataNum;i++)
	{
		HAL_ADC_Start(&hadc1);//ADC_开启
		if(HAL_ADC_PollForConversion(&hadc1,100)==HAL_OK)//轮询
		{
			ADC1_Channel_0[i]=HAL_ADC_GetValue(&hadc1);//获取通道0
		}
		HAL_ADC_Start(&hadc1);//ADC_开启
		if(HAL_ADC_PollForConversion(&hadc1,100)==HAL_OK)//轮询判断
		{
			ADC1_Channel_1[i]=HAL_ADC_GetValue(&hadc1);//通道1
		}
		HAL_Delay(100);
		HAL_ADC_Stop(&hadc1);
	}
	ADC1_Value[0]=adc_get_avg(ADC1_Channel_0,ADC_DataNum);
	ADC1_Value[1]=adc_get_avg(ADC1_Channel_1,ADC_DataNum);
		
	printf(" ADC channel0 end value = ->%1.3fV \r\n", ADC1_Value[0] * 3.3f / 4096);
  printf(" ADC channel1 end value = ->%1.3fV \r\n", ADC1_Value[1] * 3.3f / 4096);
	
	HAL_ADC_Stop(&hadc1);
	
	return ADC1_Value[0]*3.3f/4096;
}

/**
  * @brief 线性映射大小
  * @param  
  * @retval 
  */
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    const long dividend = out_max - out_min;
    const long divisor = in_max - in_min;
    const long delta = x - in_min;
    if(divisor == 0){
        return -1; //AVR returns -1, SAM returns 0
    }
    return (delta * dividend + (divisor / 2)) / divisor + out_min;
}


//公式法计算
float get_temp_calculate(float Rt)
{
	float temp = 0.0f;
	float Bx = 3950;
	float R7 = 30000;
	float T7 = 25+273.15;
	float Kt = 273.15;

	temp = 1/(log(Rt/R7)/Bx +1/T7)-Kt+0.5;
	return temp;
}

//获取温度
float get_adc_temperture(void)
{
	float temp = 0.0f;
	float Rt = 0;
	float vol = 0;
	vol = (float)ADC1_Value[1]*3.3f/4096;
	//printf("ADC temperatrue analog value = %f\r\n",vol);
	
	Rt=(10000*vol)/(3.3f-vol);
	//printf("ADC temperatrue Rt = %f\r\n",Rt);
	
	temp = get_temp_calculate(Rt);
	//printf("ADC temperatrue temp = %f\r\n",temp);
	
	return temp;
}
