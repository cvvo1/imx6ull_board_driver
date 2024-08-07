#include "bsp_delay.h"
#include "bsp_int.h"
#include "bsp_led.h"

/* 使用6U低GPT(General Purpose Timer)定时器实现高精度延时
 * GPT定时器是32位向上计数器。GPT定时器有捕获的功能。GPT定时器支持比较输出或中断功能。GPT定时器有一个12位的分频器。
 * GPT时钟源可以选择，这里我们使用ipg_clk=66M作为GPT的时钟源。
 * 
 * 实现高精度延时步骤
 * 1、设置GPT1定时器。设置GPT1_CR寄存器的SWR(bit15)位置1来复位寄存器GPT1。
 * 	  复位后设置寄存器GPT1_CR寄存器的CLKSRC(bit8:6)位(001)，选择GPT1的时钟源为 ipg_clk。
 * 2、设置定时器GPT1的工作模式。设置寄存器GPT1_CR寄存器的FRR(bit9)位为0，
 * 	  设置为Restart mode，比较事件发生后，计数器重置为0x00000000并继续计数。只有比较通道1才有此功能。Free-run模式：所有三个输出比较通道都适用
 * 3、设置GPT1的分频值。设置寄存器GPT1_PR寄存器的PRESCALAR(bit11:0)位，设置分频值。
 * 4、设置GPT1的比较值。如果要使用GPT1的输出比较中断，那么GPT1的输出比较寄存器GPT1_OCR 的值可以根据所需的中断时间来设置。
 *    本章例程不使用比较输出中断，所以将GPT1_OCR1设置为最大值，即：0XFFFFFFFF。
 * 5、使能GPT1定时器。设置GPT1_CR的EN(bit0)位为1来使能GPT1定时器。
 * 6、编写延时函数GPT1。针对us和ms延时分别编写两个延时函数。
 * 
*/

/*GPI1定时器初始化*/
void delay_Init(void)
{
	GPT1->CR=0;  /* 先清零CR寄存器，同时关闭GPT */	
	GPT1->CR|=(1<<15);    /*复位GPT1*/
	while((GPT1->CR>>15)&0x01);    /*软复位，需要等待复位完成，即bit15位为0*/

	GPT1->CR|=(1<<6);   /*选择时钟源为ipg_clk=66MHz*/
	GPT1->CR &=~(1<<9);    /*Free-Run mode工作模式*/
	GPT1->CR |=(1<<1);     /*GPT计数器初始值重置为0*/
	GPT1->PR=65;         /*设置为65，即65+1=66分频，因此GPT1时钟为66M/(65+1)=1MHz，记一个数为1us*/
	GPT1->OCR[0]=0xffffffff;   /*将Restart模式下的GPT1_OCR1设置为最大值*/
	GPT1->CR |=(1<<0);    /*使能GPT1定时器*/
	
}	

/*us延时*/
void delay_us(uint32_t usdelay)
{
	uint32_t oldcnt,newcnt;
	uint32_t tcntvalue=0;   /*保存走过的时间差*/
	oldcnt=GPT1->CNT;
	while(1)
	{
		newcnt=GPT1->CNT;
		if(newcnt!=oldcnt)
		{
			if(newcnt>oldcnt)   /*GPT1的CNT向上计数，表示没有溢出*/
			{
				tcntvalue += newcnt-oldcnt;   /*得到走过的时间*/
			}
			else
			{
				tcntvalue += 0xffffffff-newcnt+oldcnt;   /*得到走过的时间*/
			}
			oldcnt=newcnt;     /*更新oldcnt值*/
			if(tcntvalue>=usdelay)    /* 延时时间到了 */
			{
				break;               /* 跳出 */
			}
		}
	}
}

/*ms延时*/
void delay_ms(uint32_t msdelay)
{
	int i=0;
	for(i=0;i<msdelay;i++)
	{
		delay_us(1000);
	}
}


/*s延时*/
void delay_s(uint32_t sdelay)
{
	int i=0;
	for(i=0;i<sdelay;i++)
	{
		delay_ms(1000);
	}
}

#if 0   /*高精度延时不需要使用中断*/ 
	GPT1->OCR[0]=500000;   /*设置中断周期为500ms*/
	GPT1->IR|=(1<<0);      /*打开GPT1输出比较通道1中断*/

    GIC_EnableIRQ(GPT1_IRQn);       /*使能GIC对应中断，GPT1_IRQn对应的中断ID为55+32=87*/
   
    /* 注册中断服务函数 */
    system_register_irqhandler(GPT1_IRQn,(system_irq_handler_t)gpt1_irqhandler,NULL); 

	GPT1->CR |=(1<<0);    /*使能GPT1定时器*/
}


/*GPT1中断处理函数*/
void gpt1_irqhandler(void)
{
	static unsigned char state=0;
    if(GPT1->SR&(1<<0))   /*判断GPT1中断是否发生*/
    {
        state=!state;     /*状态取反*/
        led_switch(LED0,state);     /*切换蜂鸣器状态*/
    }
    /*清除GPT1中断标志位*/
    GPT1->SR |=(0x1<<0);
}
#endif

#if 0 
/*短延时*/
void delay_short(volatile unsigned int n)
{
	while(n--)
    {

    }
}

/*延时，
主频396MHz,设置位0x7ff,一次循环大概是1ms，n:延时毫秒数
主频528MHz,设置位0xaa9,一次循环大概是1ms，n:延时毫秒数
*/
void delay(volatile unsigned int n)
{
	while(n--)
	{
		delay_short(0xaa9);
	}
}
#endif