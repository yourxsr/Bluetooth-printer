#ifndef __EM_MOTOR_H__
#define __EM_MOTOR_H__

#include "em_config.h"

void motor_run_step(uint32_t steps);
void motor_start(void);
void motor_stop(void);
void motor_Init(void);
void motor_run(void);

#endif
