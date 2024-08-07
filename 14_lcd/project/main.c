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



/*
IMX6ULL IO初始化：
	①、使能时钟，CCGR0~CCGR6这7个寄存器控制着6ULL所有外设时钟的使能，设置CCGR0~CCGR6这7个寄存器全部为0XFFFFFFFF，相当于使能所有外设时钟。
	②、IO复用，将寄存器IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03的bit3~0设置为0101=5，这样GPIO1_IO03就复用为GPIO。
	③、寄存器IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03是设置GPIO1_IO03的电气属性。包括压摆率、速度、驱动能力、开漏、上下拉等，为0x10b0。
	④、配置GPIO功能，设置输入输出。设置GPIO1_GDIR寄存器bit3为1，也就是设置为输出模式。设置GPIO1_DR寄存器的bit3，为1表示输出高电平，为0表示输出低电平。
寄存器各对应地址：
*/

/* 背景颜色索引 */
unsigned int backcolor[10] = {
	LCD_BLUE, 		LCD_GREEN, 		LCD_RED, 	LCD_CYAN, 	LCD_YELLOW, 
	LCD_LIGHTBLUE, 	LCD_DARKBLUE, 	LCD_WHITE, 	LCD_BLACK, 	LCD_ORANGE

}; 


int main(void)
{
	uint8_t LED_state = OFF;
	uint8_t index=0;

	int_Init();         /*中断初始化*/
	imx6u_clkinit();    /*初始化系统时钟*/
	delay_Init();
	clk_enable();		/* 使能所有的时钟*/
	led_init();        /* 初始化led */
	beep_Init();     /* 初始化蜂鸣器 */
	uart_Init();
	lcd_Init();

	CRlcd_dev.backcolor=LCD_WHITE;


	while(1)			/* 死循环 */
	{
		lcd_clear(backcolor[index]);
		lcd_show_string(10, 40, 260, 32, 32,(char*)"ALPHA IMX6U"); 	  /*默认使用前景色*/
		lcd_show_string(10, 80, 240, 24, 24,(char*)"RGBLCD TEST");
		lcd_show_string(10, 110, 240, 16, 16,(char*)"ATOM@ALIENTEK");      					 
		lcd_show_string(10, 130, 240, 12, 12,(char*)"2024/4/11");
		index++;
		if(index == 10)
		{
			index = 0; 
		}
		// LED_state=!LED_state;
		// led_switch(LED0,LED_state);
		// delay_ms(10);
	}

	return 0;
}

