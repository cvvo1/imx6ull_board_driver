/*
  1、设置UART1_CTS复用为GPIO1_IO18
  2、设置UART1_CTS的电气属性
  3、配置GPIO1_IO18为输入模式。
  4、读取按键值，GPIO1_IO18不按下时为高电平，按下时为低电平

*/

#include "bsp_key.h"
#include "cc.h"
#include "bsp_delay.h"
#include "bsp_gpio.h"

void key_init(void)
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
   key_config.directon=KGPIO_DigitalInput;

   
   gpio_Init(GPIO1,18,&key_config);   /*设置GPIO1_IO18为输入*/
}
/*读取按键值，返回值：0按下，1没按下*/
uint8_t read_key(void)
{
    int ret=0;
    ret=gpio_pinread(GPIO1,18);
    return ret;
}

uint8_t Key_GetNum(void)
{
  uint8_t KeyNum=0;
  if (read_key()== 0)
  {
    delay_ms(10);
    while(read_key()== 0);
    delay_ms(10);
    KeyNum=1;
  }
  return KeyNum;
}



