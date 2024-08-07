#ifndef _BSP_DELAY_H
#define _BSP_DELAY_H

#include "imx6ul.h"

// void delay_short(volatile unsigned int n);
// void delay(volatile unsigned int n);
void delay_Init(void);
// void gpt1_irqhandler(void);
void delay_us(uint32_t usdelay);
void delay_ms(uint32_t msdelay);
void delay_s(uint32_t sdelay);

#endif // !_BSP_DELAY_H