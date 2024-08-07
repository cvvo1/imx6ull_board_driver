#include "bsp_exti.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_delay.h"
#include "bsp_beep.h"

/*初始化外部中断，也就是GPIO1-IO18，按键中断*/
void exti_Init(void)
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

   gpio_pin_config_t key_config;
   key_config.direction=KGPIO_DigitalInput;
   key_config.interruptMode=kGPIO_IntFallingEdge;

   gpio_Init(GPIO1,18,&key_config);    /*设置GPIO1_IO18为输入，且下降沿触发中断*/    

   GIC_EnableIRQ(GPIO1_Combined_16_31_IRQn);       /*使能GIC对应中断，GPIO1-IO18对应的中断ID为67+32=99*/
   
   /* 注册中断服务函数 */
   system_register_irqhandler(GPIO1_Combined_16_31_IRQn,gpio1_io18_irqhandle,NULL);  

   gpio_enable(GPIO1,18);   /* 使能GPIO1_IO18的中断功能 */
}

/*
 * @description			: GPIO1_IO18最终的中断处理函数
 * @param				: 无
 * @return 				: 无
 * 当按下KEY以后触发GPIO中断，然后在中断服务函数里控制蜂鸣器开关
*/
void gpio1_io18_irqhandle(unsigned int giccIar, void *param)
{
    static unsigned char state=0;

    /*
	 *采用延时消抖，中断服务函数中禁止使用延时函数！因为中断服务需要
	 *快进快出！！这里为了演示所以采用了延时函数进行消抖，后面我们会讲解
	 *定时器中断消抖法！！！
 	*/
    delay_ms(10);
    if((gpio_pinread(GPIO1,18))==0)   /*延时，判断按键是否按下了*/
    {
        state=!state;     /*状态取反*/
        beep_switch(state);     /*切换蜂鸣器状态*/
    }

    /*清除GPIO中断标志位*/
    gpio_clearintflags(GPIO1,18);

}