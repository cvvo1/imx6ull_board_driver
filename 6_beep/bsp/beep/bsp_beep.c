/*
  1、将SNVS_TAMPER1复用为GPIO
  2、设置电气属性
  3、
  4、
*/

#include "bsp_beep.h"
#include "cc.h"

/*BEEP初始化*/
void beep_Init(void)
{
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01,0);  /*复用为GPIO5_IO01*/
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01,0x10b0);   /*设置电气属性*/

    GPIO5->GDIR |=(1<<1);   /*设置GPIO5_IO01为输出*/
    GPIO5->DR |=(1<<1);     /*设置GPIO5_IO01输出高电平，关闭蜂鸣器*/
}

void beep_switch(int status)
{
    if(status == ON)
        GPIO5->DR &= ~(1<<1);   /*打开蜂鸣器，bit1位置0*/
    else if(status == OFF)
        GPIO5->DR |=(1<<1);   /*关闭蜂鸣器，bit1位置1*/
    
}