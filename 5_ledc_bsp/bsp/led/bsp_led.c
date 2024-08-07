#include "bsp_led.h"
#include "cc.h"


/*初始化LED*/
void led_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03,0);  /*复用为GPIO1_IO03*/
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03,0x10b0); /*设置电气属性*/                       


    /*GPIO初始化*/
    GPIO1->GDIR=0x8;   /*设置方向寄存器为输出模式*/
    GPIO1->DR=0x0;     /*打开LED灯*/
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
    switch (status)
    {
    case LED0:
        if(status == ON)
            led_on();
        else if (status == OFF)
        {
            led_off();
        }
        break;
    }
}

