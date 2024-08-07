#include "bsp_gpio.h"

/*
 * 6ULL GPIO中断设置
 * 1、首先设置GPIO的中断触发方式，即GPIO_ICR1(低16位)或ICR2寄存器，触发方式有低电平、高电平、上升沿和下降沿，本例设置KEY0，UART1_CTS下降沿触发
 * 2、使能GPIO对应中断，设置GPIO_IMR寄存器
 * 3、处理完中断后，需要清除中断标志位，即对GPIO_ISR寄存器相应位置1
*/

/*
 * GPIO初始化
 * @param - base	: 要初始化的GPIO组。
 * @param - pin		: 要初始化GPIO在组内的编号。
 * @param - config	: GPIO配置结构体。
 * @return 			: 无
*/
void gpio_Init(GPIO_Type *base,uint8_t pin, gpio_pin_config_t *config)
{
    base->IMR &= ~(1U << pin);

    if(config->direction == KGPIO_DigitalInput)       /*为输入，pin位置0*/
    {
        base->GDIR &= ~(1 << pin);
    }
    else    /*为输出，pin位置置1*/
    {
        base->GDIR |= (1 << pin);
        gpio_pinwrite(base,pin,config->outputLogic);  /*设置默认输出电平*/
    }
    gpio_intconfig(base,pin,config->interruptMode);  /*中断功能配置*/
}

/*控制GPIO高低电平*/
void gpio_pinwrite(GPIO_Type *base,uint8_t pin, uint8_t value)
{
    if(value == 0U)        /*写入0*/
    {
        base->DR &= ~(1U<<pin);
    }
    else
    {
        base->DR |= (1U<<pin);
    }
}

/*读取指定IO电平*/
int gpio_pinread(GPIO_Type *base,uint8_t pin)
{
    return (((base->DR) >> pin) & 0x1);
}

/*
 * @description  			: 设置GPIO的中断配置功能
 * @param - base 			: 要配置的IO所在的GPIO组。
 * @param - pin  			: 要配置的GPIO脚号。
 * @param - pinInterruptMode: 中断模式，参考枚举类型gpio_interrupt_mode_t
 * @return		 			: 无
 */
void gpio_intconfig(GPIO_Type* base, uint8_t pin, gpio_interrupt_mode_t pin_int_mode)
{
    /*先判断是第十六位还是高十六位对应ICR1和ICR2*/
    volatile uint32_t *icr;    /*存放ICR1和ICR2地址*/
	uint32_t icrShift;
    icrShift=pin;

    base->EDGE_SEL &= ~(1U << pin);   /*先对EDGE_SEL寄存器置0，否则ICR无效*/
    
    if(pin<16)
    {
        icr=&(base->ICR1);
    }
    else
    {
        icr=&(base->ICR2);
        icrShift-=16;      /*转换为对应寄存器的位*/
    }

    switch (pin_int_mode)
    {
    case kGPIO_IntLowLevel:
        *icr &=~(0x3<<(2*icrShift));
        break;
    case kGPIO_IntHighLevel:
        *icr &=~(3U<<(2*icrShift));    /*先置0*/
        *icr |=(1U<<(2*icrShift));     /*后置1*/
        break;
    case kGPIO_IntRisingEdge:
        *icr &=~(3U<<(2*icrShift));
        *icr |=(2U<<(2*icrShift));
        break;
    case kGPIO_IntFallingEdge:
        *icr &=~(3U<<(2*icrShift));
        *icr |=(3U<<(2*icrShift));
        break;
    case kGPIO_IntRisingOrFallingEdge:
        base->EDGE_SEL |= (1U << pin);   /*EDGE_SEL置1,对应上升沿和下降沿有效*/
        break;
    default:
			break;
    }

    

}

/*
 * @description  			: 使能GPIO的中断功能
 * @param - base 			: 要使能的IO所在的GPIO组。
 * @param - pin  			: 要使能的GPIO在组内的编号。
 * @return		 			: 无
*/
void gpio_enable(GPIO_Type *base, uint8_t pin)
{
    base->IMR |=(1 << pin);
}


/*
 * @description  			: 禁止GPIO的中断功能
 * @param - base 			: 要禁止的IO所在的GPIO组。
 * @param - pin  			: 要禁止的GPIO在组内的编号。
 * @return		 			: 无
*/
void gpio_disable(GPIO_Type *base, uint8_t pin)
{
    base->IMR &=~(1 << pin);
}

/*
 * @description  			: 清除中断标志位(写1清除)
 * @param - base 			: 要清除的IO所在的GPIO组。
 * @param - pin  			: 要清除的GPIO掩码。
 * @return		 			: 无
 */
void gpio_clearintflags(GPIO_Type *base, uint8_t pin)
{
    base->ISR |=(1<<pin);
}