#ifndef  __BSP_LED_H
#define  __BSP_LED_H

#define LED0  0

#include "imx6ul.h"

void led_init(void);
void led_on(void);
void led_off(void);
void led_switch(int led,int status);

#endif 