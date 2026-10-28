#ifndef __ADC_H
#define __ADC_H

#include "sys.h"

void adc_init(void);
u16  Get_adc(u8 ch);
u16  Get_adc_average(u8 ch, u8 times);

#endif
