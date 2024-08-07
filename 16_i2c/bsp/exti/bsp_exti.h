#ifndef _BSP_EXTI_H
#define _BSP_EXTI_H
#include "imx6ul.h"

void exti_Init(void);
void gpio1_io18_irqhandle(unsigned int giccIar, void *param);


#endif // !__BSP_EXIT_H