#ifndef __EM_TIMER_H__
#define __EM_TIMER_H__

#include <em_config.h>

void Timer_Init(void);
bool get_state_timeout(void);
void clean_state_timeout(void);
void open_printer_timeout_timer(void);
void close_printer_timeout_timer(void);
bool get_printer_timeout_status(void);

#endif
