#include "main.h"
/*
IMX6ULL IO初始化：
	①、使能时钟，CCGR0~CCGR6这7个寄存器控制着6ULL所有外设时钟的使能，设置CCGR0~CCGR6这7个寄存器全部为0XFFFFFFFF，相当于使能所有外设时钟。
	②、IO复用，将寄存器IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO03的bit3~0设置为0101=5，这样GPIO1_IO03就复用为GPIO。
	③、寄存器IOMUXC_SW_PAD_CTL_PAD_GPIO1_IO03是设置GPIO1_IO03的电气属性。包括压摆率、速度、驱动能力、开漏、上下拉等，为0x10b0。
	④、配置GPIO功能，设置输入输出。设置GPIO1_GDIR寄存器bit3为1，也就是设置为输出模式。设置GPIO1_DR寄存器的bit3，为1表示输出高电平，为0表示输出低电平。
寄存器各对应地址：
*/
/*使能时钟*/
void clk_enable(void)
{
    CCM_CCGR0=0xFFFFFFFF;
    CCM_CCGR1=0xFFFFFFFF;
    CCM_CCGR2=0xFFFFFFFF;
    CCM_CCGR3=0xFFFFFFFF;
    CCM_CCGR4=0xFFFFFFFF;
    CCM_CCGR5=0xFFFFFFFF;
    CCM_CCGR6=0xFFFFFFFF;
}

/*初始化LED*/
void led_init(void)
{
    SW_MUX_GPIO1_IO03=0x5;      /*复用为GPIO1_IO03*/
    SW_PAD_GPIO1_IO03=0x10b0;   

    /*GPIO初始化*/
    GPIO1_GDIR=0x8;    /*设置方向寄存器为输出模式*/
    GPIO1_DR=0x0;      /*打开LED灯*/
}


/*打开LED灯，开灯之前bit3为1*/
void led_on(void)
{
    GPIO1_DR &= ~(1<<3);   /*1左移三位后取反，再与原数据与上，等于对原数据第三位取反（bit3置0）*/
}

/*关闭LED灯，关灯之前bit3为0*/
void led_off(void)
{
    GPIO1_DR |= (1<<3);    /*1左移三位后，再与原数据或上，等于对原数据第三位取反（bit3置1）*/
}

/*短延时*/
void delay_short(volatile unsigned int n)
{
	while(n--)
    {

    }
}

/*延时，主频396MHz,一次循环大概是1ms，n:延时毫秒数*/
void delay(volatile unsigned int n)
{
	while(n--)
	{
		delay_short(0x7ff);
	}
}

int main(void)
{
	clk_enable();		/* 使能所有的时钟		 	*/
	led_init();			/* 初始化led 			*/

	while(1)			/* 死循环 				*/
	{	
        delay(500);		/* 延时大约500ms 		*/
		led_off();		/* 关闭LED   			*/
		
        delay(500);		/* 延时大约500ms 		*/
		led_on();		/* 打开LED		 	*/
	}

}