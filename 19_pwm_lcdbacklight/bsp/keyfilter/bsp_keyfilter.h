#ifndef _BSP_KEYFILTER_H
#define _BSP_KEYFILTER_H

#include "imx6ul.h"
void keyfilter_Init(void);
void filtertimer_Init(uint32_t frac,uint32_t value);
void keyfilter_stop(void);
void keyfilter_start(uint32_t value);
void keyfilter_irqhandler(void);
void gpio1_18_epit_irqhandle(void);

#endif // !__BSP_KEYFILTER_H