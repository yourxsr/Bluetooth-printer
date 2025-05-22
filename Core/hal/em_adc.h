#ifndef __EM_ADC_H__
#define __EM_ADC_H__

#include "em_config.h"
void adc_Init(void);
float get_adc_volts(void);
long map(long x, long in_min, long in_max, long out_min, long out_max);
float get_adc_temperture(void);

#endif
