#include "main.h"
#include "bsp_delay.h"
#include "bsp_clk.h"
#include "bsp_beep.h"
#include "bsp_led.h"

/*
IMX6ULL IO初始化：
	①、使能时钟，CCGR0~CCGR6这7个寄存器控制着6ULL所有外设时钟的使能，设置CCGR0~CCGR6这7个寄存器全部为0XFFFFFFFF，相当于使能所有外设时钟。
	②、IO复用，将寄存器IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03的bit3~0设置为0101=5，这样GPIO1_IO03就复用为GPIO。
	③、寄存器IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03是设置GPIO1_IO03的电气属性。包括压摆率、速度、驱动能力、开漏、上下拉等，为0x10b0。
	④、配置GPIO功能，设置输入输出。设置GPIO1_GDIR寄存器bit3为1，也就是设置为输出模式。设置GPIO1_DR寄存器的bit3，为1表示输出高电平，为0表示输出低电平。
寄存器各对应地址：
*/

int main(void)
{
	clk_enable();		/* 使能所有的时钟		 	*/
	led_init();        /* 初始化led 			*/
	beep_Init();			


	while(1)			/* 死循环 				*/
	{	
		delay(1000);		        /* 延时大约500ms 		*/
        beep_switch(ON);		/* 打开LED		 	*/
		led_on();
		
		delay(1000);		        /* 延时大约500ms 		*/
		beep_switch(OFF);		
		led_off();   /* 关闭LED   			*/
	}

	return 0;
}

