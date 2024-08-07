#include "bsp_epittimer.h"
#include "bsp_int.h"
#include "bsp_led.h"


/* EPIT是一个32位的一个向下计数器
 * EPIT配置步骤如下
 * 1、设置EPIT1的时钟源。设置寄存器 EPIT1_CR寄存器的CLKSRC(bit25:24)位为01，选择EPIT1的时钟源为ipg_clk=66MHz。
 * 2、设置分频值。设置寄存器EPIT1_CR寄存器的PRESCALAR(bit15:4)位，12位的分频器，0~4095分别代表1~4096分频。
 * 3、设置工作模式。设置寄存器EPIT1_CR的RLD(bit3)位，有两种工作模式：Set-add-forget(置1)、free-runing(置0)
 * 4、设置计数器的初始值来源。设置寄存器EPIT1_CR的ENMOD(bit1)位，置1为设置计数器的初始值为记载寄存器的值。
 * 5、使能比较中断。需要设置寄存器 EPIT1_CR 的 OCIEN(bit2)位。
 * 6、设置加载值和比较值。EPIT1_LR 中的加载值和寄存器 EPIT1_CMPR 中的比较值，通过这两个寄存器就可以决定定时器的中断周期。
 * 7、EPIT1中断设置和中断服务函数编写。使能 GIC 中对应的 EPIT1 中断，注册中断服务函数，如果需要的话还可以设置中断优先级，最后编写中断服务函数。
 * 8、使能 EPIT1 定时器。通过寄存器 EPIT1_CR 的 EN(bit0)位来设置。
 * 
 * EPIT_SR寄存器，只有bit0有效，表示中断状态，写1清零。
 * 当OCIF位为1的时候表示中断发生，为0的时候表示中断未发生。我们处理完定时器中断以后一定要清除中断标志位。
 * 
 * EPIT溢出时间计算:
 *  Tout=((frac+1)*value)/Tclk;    Tout-EPIT溢出时间;frac-分频值;value-加载值;Tclk-时钟源频率-66MHz
 *  实现500ms中断周期时，采用1000分频(frac=1000)，加载值为33000(value)
*/

/*
 * @description		: 初始化EPIT定时器.
 *					  EPIT定时器是32位向下计数器,时钟源使用ipg=66Mhz		 
 * @param - frac	: 分频值，范围为0~4095，分别对应1~4096分频。
 * @param - value	: 倒计数值。
 * @return 			: 无
 */

void epittimer_Init(uint32_t frac,uint32_t value)
{
    if(frac > 0XFFF)
    {
        frac = 0XFFF;
    }
    EPIT1->CR = 0;	/* 先清零CR寄存器 */
    
    EPIT1->CR |=(0x1<<24);  /*设置EPIT1的时钟源*/
    
    EPIT1->CR |=(frac<<4); /*设置分频值*/

    EPIT1->CR |=(0x1<<3);    /*设置工作模式*/

    EPIT1->CR |=(0x1<<1);   /*设置计数器的初始值来源*/
    
    EPIT1->CR |=(0x1<<2);  /*使能比较中断*/
    
    EPIT1->LR=value;  /*设置加载值*/
       
    EPIT1->CMPR=0;   /*设置比较计数器，默认写0 */

    GIC_EnableIRQ(EPIT1_IRQn);       /*使能GIC对应中断，GPIO1-IO18对应的中断ID为67+32=99*/
   
    /* 注册中断服务函数 */
    system_register_irqhandler(EPIT1_IRQn,(system_irq_handler_t)epit1_irqhandler,NULL);  

    EPIT1->CR |=(0x1<<0);   /* 使能EPIT1 */

}


/*
 * @description			: EPIT中断处理函数
 * @param				: 无
 * @return 				: 无
 */
void epit1_irqhandler(void)
{
    static unsigned char state=0;

    if((EPIT1->SR)&0x1)   /*判断EPIT中断是否发生*/
    {
        state=!state;     /*状态取反*/
        led_switch(LED0,state);     /*切换蜂鸣器状态*/
    }

    /*清除EPIT中断标志位*/
    EPIT1->SR |=(0x1<<0);
    

}

