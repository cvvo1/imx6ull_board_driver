#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H
#include "imx6ul.h"

/*枚举描述GPIO中断触发类型*/
typedef enum _gpio_interrupt_mode
{
    kGPIO_NoIntmode = 0U, 				/* 无中断功能 */
    kGPIO_IntLowLevel = 1U, 			/* 低电平触发	*/
    kGPIO_IntHighLevel = 2U, 			/* 高电平触发 */
    kGPIO_IntRisingEdge = 3U, 			/* 上升沿触发	*/
    kGPIO_IntFallingEdge = 4U, 			/* 下降沿触发 */
    kGPIO_IntRisingOrFallingEdge = 5U, 	/* 上升沿和下降沿都触发 */
}gpio_interrupt_mode_t;


/*枚举类型和GPIO结构体*/
typedef enum _gpio_pin_direction
{
    KGPIO_DigitalInput = 0U,
    KGPIO_DigitalOutput = 1U,
}gpio_pin_direction_t;

typedef struct _gpio_pin_config
{
    gpio_pin_direction_t directon;     /* GPIO方向:输入还是输出*/
    uint8_t outputLogic;             /* 如果是输出的话，默认输出电平 */
    gpio_interrupt_mode_t interruptMode;	/* 中断方式 */

}gpio_pin_config_t;

void gpio_Init(GPIO_Type *base, unsigned int pin, gpio_pin_config_t *config);
void gpio_pinwrite(GPIO_Type *base, unsigned int pin, int value);
int gpio_pinread(GPIO_Type *base, unsigned int pin);
void gpio_intconfig(GPIO_Type* base, unsigned int pin, gpio_interrupt_mode_t pin_int_mode);
void gpio_enable(GPIO_Type *base, unsigned int pin);
void gpio_disable(GPIO_Type *base, unsigned int pin);
void gpio_clearintflags(GPIO_Type *base, unsigned int pin);

#endif // !__BSP_GPIO_H