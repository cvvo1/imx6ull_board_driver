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
#include "bsp_adc.h"


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
	uint8_t i;
	uint8_t LED_state = OFF;
	uint32_t adcvalue;
	int32_t integ; 	/* 整数部分 */
	int32_t fract;	/* 小数部分 */

	imx6ul_hardfpu_enable();	/* 使能I.MX6U的硬件浮点 			*/
	int_Init();         /*中断初始化*/
	imx6u_clkinit();    /*初始化系统时钟*/
	delay_Init();
	clk_enable();		/* 使能所有的时钟*/
	led_init();        /* 初始化led */
	beep_Init();     /* 初始化蜂鸣器 */
	uart_Init();
	lcd_Init();
	


	CRlcd_dev.forecolor = LCD_RED;
	lcd_show_string(50, 10, 400, 24, 24, (char*)"ALPHA-IMX6U ADC TEST");  
	lcd_show_string(50, 40, 200, 16, 16, (char*)"ATOM@ALIENTEK");  
	lcd_show_string(50, 60, 200, 16, 16, (char*)"2024/4/21");   
	lcd_show_string(50, 90, 400, 16, 16, (char*)"ADC Ori Value:0000");  
	lcd_show_string(50, 110, 400, 16, 16,(char*)"ADC Val Value:0.00 V");  
	CRlcd_dev.forecolor = LCD_BLUE;

	while(adc1ch1_Init())		/* ADC显示 */
	{
		lcd_show_string(30, 130, 200, 16, 16, (char*)"AP3216C calibration Failed!");
		delay_ms(500);
		lcd_show_string(30, 130, 200, 16, 16, (char*)"Please calibration!        ");
		delay_ms(500);
	}

	lcd_show_string(50, 130, 200, 16, 16, (char*)"ADC1CH1 Ready!");  

	while(1)			/* 死循环 */
	{
		adcvalue=getadc_value();
		lcd_showxnum(162, 90, adcvalue, 4, 16, 0);	/* ADC原始数据值 */

		adcvalue = getadc_volt();
		integ = adcvalue / 1000;
		fract = adcvalue % 1000;

		lcd_showxnum(162, 110, integ, 1, 16, 0);	/* 显示电压值的整数部分，3.1111的话，这里就是显示3 */
		lcd_showxnum(178, 110, fract, 3, 16, 0X80);	/* 显示电压值小数部分（前面转换为了整形显示），这里显示的就是111. */
		printf("ADC volt = %d.%dV\r\n", integ, fract);

		delay_ms(50);
		i++;
		if(i==10)
		{
			i = 0;
			LED_state = !LED_state;
			led_switch(LED0,LED_state);
		}
	}
	return 0;
}

