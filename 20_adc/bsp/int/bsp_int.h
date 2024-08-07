#ifndef _BSP_INT_H
#define _BSP_INT_H

#include "imx6ul.h"

/*定义中断处理函数，giccIar——中断ID；param——传递给中断函数的参数*/
typedef void (*system_irq_handler_t) (unsigned int giccIar, void *param);  
           /*1.void(*system_irq_handler_t)()
                system_irq_handler_t是一个指向返回值为void ，参数为空的类型的函数指针
            2.void(*system_irq_handler_t)(unsigned int giccIar, void *param)
                system_irq_handler_t是一个指向返回值为void，参数为unsigned int类型的普通变量和void *类型的指针
            3. typedef void(*system_irq_handler_t)(unsigned int giccIar, void *param)
                system_irq_handler_t不再是一个函数指针了，它代表着一种函数指针类型，这种类型可以定义一个指向返回值为void，
                参数为unsigned int类型的普通变量和void *类型的指针*/

/*中断处理函数结构体*/
typedef struct _sys_irq_handle
{
    system_irq_handler_t irqHandler; /* 中断服务函数 */
    void *userParam;                 /* 中断服务函数参数 */
} sys_irq_handle_t;    /*sys_irq_handle_t为定义新类型名*/


void int_Init(void);
void system_irqtable_Init(void);
void system_register_irqhandler(IRQn_Type irq, system_irq_handler_t handler, void *userParam);
void system_irqhandler(unsigned int gicciar);
void default_irqhandler(unsigned int gicciar, void *userParam);
#endif // !__BSP_INT_H
