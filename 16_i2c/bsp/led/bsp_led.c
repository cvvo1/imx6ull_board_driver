#include "bsp_led.h"
#include "cc.h"


/*初始化LED*/
void led_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03,0);  /*复用为GPIO1_IO03*/

    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03,0x10b0); /*设置电气属性*/ 

    /* 2、、配置GPIO1_IO03的IO属性	
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
	 *bit [13]: 0 kepper功能
	 *bit [12]: 1 pull/keeper使能
	 *bit [11]: 0 关闭开路输出
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 110 R0/6驱动能力
	 *bit [0]: 0 低转换率
	*/                     


    /*GPIO初始化*/
    GPIO1->GDIR=0x8;   /*设置方向寄存器为输出模式*/
    GPIO1->DR|= (1<<3);    /*默认关闭LED灯*/
}


/*打开LED灯，开灯之前bit3为1*/
void led_on(void)
{
    GPIO1->DR &= ~(1<<3);   /*1左移三位后取反，再与原数据与上，等于对原数据第三位取反（bit3置0）*/
}

/*关闭LED灯，关灯之前bit3为0*/
void led_off(void)
{
    GPIO1->DR|= (1<<3);    /*1左移三位后，再与原数据或上，等于对原数据第三位取反（bit3置1）*/
}

void led_switch(int led,int status)
{
    switch (led)
    {
        case LED0:
            if(status == ON)
            {
                GPIO1->DR &= ~(1<<3);
            }
            else if (status == OFF)
            {
                GPIO1->DR|= (1<<3);
            }
            break;
    }
}

