#include "imx6ul.h"
#include "bsp_delay.h"
#include "bsp_clk.h"
#include "bsp_beep.h"
#include "bsp_led.h"
#include "bsp_int.h"
#include "bsp_uart.h"
#include "stdio.h"
#include "bsp_lcd.h"
#include "bsp_gpio.h"
#include "bsp_lcdapi.h"
#include "bsp_rtc.h"
#include "bsp_key.h"
#include "bsp_icm20608.h"
#include "bsp_gt9147.h"
#include "bsp_backlight.h"
#include "bsp_ft5xx6.h"


/*
 * @description	: 使能I.MX6U的硬件NEON和FPU
 * @param 		: 无
 * @return 		: 无
 */
 void imx6ul_hardfpu_enable(void)
{
	uint32_t cpacr;
	uint32_t fpexc;

	/* 使能NEON和FPU */
	cpacr = __get_CPACR();
	cpacr = (cpacr & ~(CPACR_ASEDIS_Msk | CPACR_D32DIS_Msk))
		   |  (3UL << CPACR_cp10_Pos) | (3UL << CPACR_cp11_Pos);
	__set_CPACR(cpacr);
	fpexc = __get_FPEXC();
	fpexc |= 0x40000000UL;	
	__set_FPEXC(fpexc);
}

int main(void)
{
	uint8_t LED_state = OFF;
	uint8_t KEY_state =0;
	unsigned char duty = 0;
	uint8_t i=0;

	imx6ul_hardfpu_enable();	/* 使能I.MX6U的硬件浮点 			*/
	int_Init();         /*中断初始化*/
	imx6u_clkinit();    /*初始化系统时钟*/
	delay_Init();
	clk_enable();		/* 使能所有的时钟*/
	led_init();        /* 初始化led */
	beep_Init();     /* 初始化蜂鸣器 */
	uart_Init();
	lcd_Init();
	backlight_Init();
	key_init();

	CRlcd_dev.forecolor = LCD_RED;
	lcd_show_string(50, 10, 400, 24, 24, (char*)"ZERO-IMX6U BACKLIGHT PWM TEST");  
	lcd_show_string(50, 40, 200, 16, 16, (char*)"ATOM@ALIENTEK");  
	lcd_show_string(50, 60, 200, 16, 16, (char*)"2024/4/19");   
	lcd_show_string(50, 90, 400, 16, 16, (char*)"PWM Duty:   %");  
	CRlcd_dev.forecolor = LCD_BLUE;

	/* 设置默认占空比 10% */
	if(CRlcd_dev.id == ATKVGA)
		duty=100;	//VGA只能在满输出时才能亮屏
	else
		duty = 10;

	lcd_shownum(50 + 72, 90, duty, 3, 16);
	PWM_setDuty(duty);	

	while(1)			/* 死循环 */
	{
		KEY_state = Key_GetNum();
		if(KEY_state == KEY0_VALUE)
		{
			duty += 10;				/* 占空比加10% */
			if(duty > 100)			/* 如果占空比超过100%，重新从10%开始 */
				duty = 10;
			lcd_shownum(50 + 72, 90, duty, 3, 16);
			PWM_setDuty(duty);		/* 设置占空比 */
		}
		
		delay_ms(10);
		i++;
		if(i==50)
		{
			i = 0;
			LED_state = !LED_state;
			led_switch(LED0,LED_state);
		}
	}
	return 0;
}

