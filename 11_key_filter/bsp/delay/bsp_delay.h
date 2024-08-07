#ifndef _BSP_DELAY_H
#define _BSP_DELAY_H

#include "imx6ul.h"

void delay_short(volatile unsigned int n);
void delay(volatile unsigned int n);

#endif // !_BSP_DELAY_H