#ifndef _BSP_BACKLIGHT_H
#define _BSP_BACKLIGHT_H
#include "imx6ul.h"

/* 背光PWM结构体 */
struct backlight_dev_struc
{	
	unsigned char pwm_duty;		/* 占空比	*/
};

void backlight_Init(void);
void PWM1_irqhandle(unsigned int giccIar, void *param);
void PWM_softRestart();
void PWM_enable();
void PWM_setSample(uint32_t value);
void PWM_setPeriod(uint32_t value);
void PWM_setDuty(uint8_t value);


#endif // !_BSP_BACKLIGHT_H
