#include "bsp_keyfilter.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_beep.h"

/* 定时器按键消抖
 * 当按键按下以后，进入到中断服务函数中，开始一个定时器，定时周期为10ms，只有最后一个抖动信号开启的定时器才能完成的执行完一个周期。
 * 当定时器产生周期中断以后就在中断服务函数里面做具体的处理，比如开关蜂鸣器。
 * 1、配置按键IO的中断。配置按键所使用的 IO，因为要使用到中断驱动按键，所以要配置 IO 的中断模式。
 * 2、初始化消抖用的定时器。定时器的定时周期为 10ms，也可根据实际情况调整定时周期。
 * 3、编写中断处理函数。需要编写两个中断处理函数：按键对应的 GPIO 中断处理函数和 EPIT1 定时器的中断处理函数。
 * 在按键的中断处理函数中主要用于开启 EPIT1 定时器， EPIT1 的中断处理函数才是重点，
 * 按键要做的具体任务都是在定时器 EPIT1 的中断处理函数中完成的，比如控制蜂鸣器打开或关闭。
 * 
*/
/*
 * @description		: 按键初始化
 * @param			: 无
 * @return 			: 无
 */
void keyfilter_Init(void)
{
    IOMUXC_SetPinMux(IOMUXC_UART1_CTS_B_GPIO1_IO18,0);  /*复用为GPIO1_IO18*/
    IOMUXC_SetPinConfig(IOMUXC_UART1_CTS_B_GPIO1_IO18,0xf080);   /*设置电气属性*/
    /*
     *bit 16:0 HYS关闭
	   *bit [15:14]: 11 设置22k上拉，默认高电平
     *bit [13]: 1 pull功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 000 关闭输出驱动
     *bit [0]: 0 低转换率
    */
    /*GPIO初始化*/

   gpio_pin_config_t keyfilter_config;
   keyfilter_config.directon=KGPIO_DigitalInput;
   keyfilter_config.interruptMode=kGPIO_IntFallingEdge;
   keyfilter_config.outputLogic=1;

   gpio_Init(GPIO1,18,&keyfilter_config);    /*设置GPIO1_IO18为输入，且下降沿触发中断*/    

   GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);       /*使能GIC对应中断，GPIO1-IO18对应的中断ID为67+32=99*/
   
   /* 注册中断服务函数 */
   system_register_irqhandler(GPIO1_Combined_16_31_IRQn,(system_irq_handler_t)gpio1_18_epit_irqhandle,NULL);  

   gpio_enable(GPIO1,18);   /* 使能GPIO1_IO18的中断功能 */

   filtertimer_Init(1,660000);    /*初始化定时器为，10ms*/
}

/*
 * @description		: 初始化用于消抖的定时器，默认关闭定时器
 * @param - value	: 定时器EPIT计数值
 * @return 			: 无
*/
void filtertimer_Init(uint32_t frac,uint32_t value)
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
    system_register_irqhandler(EPIT1_IRQn,(system_irq_handler_t)keyfilter_irqhandler,NULL);  

}

/*
 * @description		: 关闭定时器
 * @param 			: 无
 * @return 			: 无
*/
void keyfilter_stop(void)
{
    EPIT1->CR &=~(0x1<<0);   /* 禁止EPIT1 */
}


/*
 * @description		: 重启定时器
 * @param - value	: 定时器EPIT计数值
 * @return 			: 无
*/
void keyfilter_start(uint32_t value)
{
    EPIT1->CR &=~(0x1<<0);   /* 禁止EPIT1 */
    EPIT1->LR=value;  /*设置加载值*/
    EPIT1->CR |=(0x1<<0);   /* 使能EPIT1 */
}

/*
 * @description		: 定时器中断处理函数 
 * @param			: 无
 * @return 			: 无
*/
void keyfilter_irqhandler(void)
{
    static unsigned char state=0;

    if((EPIT1->SR)&0x1)   /*判断EPIT中断是否发生*/
    {
        keyfilter_stop();    /*关闭定时器*/
        if((gpio_pinread(GPIO1,18))==0)   /*判断按键是否按下了*/
        {
            state=!state;     /*状态取反*/
            beep_switch(state);     /*切换蜂鸣器状态*/
        }
    }
    /*清除EPIT中断标志位*/
    EPIT1->SR |=(0x1<<0);

}

/*
 * @description		: GPIO中断处理函数
 * @param			: 无
 * @return 			: 无
*/
void gpio1_18_epit_irqhandle(void)
{
    /*开启定时器*/
    keyfilter_start(660000);
    /*清除GPIO中断标志位*/
    gpio_clearintflags(GPIO1,18);
}