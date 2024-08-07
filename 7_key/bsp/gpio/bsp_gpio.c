#include "bsp_gpio.h"

/*
 * GPIO初始化
 * @param - base	: 要初始化的GPIO组。
 * @param - pin		: 要初始化GPIO在组内的编号。
 * @param - config	: GPIO配置结构体。
 * @return 			: 无
*/
void gpio_Init(GPIO_Type *base, int pin, gpio_pin_config_t *config)
{
    if(config->directon == KGPIO_DigitalInput)       /*为输入，pin位置0*/
    {
        base->GDIR &= ~(1 << pin);
    }
    else    /*为输出，pin位置置1*/
    {
        base->GDIR |= (1 << pin);
        /*设置默认输出电平*/
    }
}

/*控制GPIO高低电平*/
void gpio_pinwrite(GPIO_Type *base, int pin, int value)
{
    if(value == 0)        /*写入0*/
    {
        base->DR &= ~(1<<pin);
    }
    else
    {
        base->DR |= (1<<pin);
    }
}

/*读取指定IO电平*/
int gpio_pinread(GPIO_Type *base, int pin)
{
    int ret;
    ret=((base->DR >> pin) & 0x1);      
    return ret;

}