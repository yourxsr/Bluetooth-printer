#ifndef __EM_KEYMOTOR_H__
#define __EM_KEYMOTOR_H__

#include "em_config.h"

void click_slow(void);
void click_long_free(void);
void long_click_run(void);
bool get_printer_test(void);
void set_printer_test(bool state);

#endif
