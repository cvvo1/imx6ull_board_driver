#ifndef _BSP_UART_H
#define _BSP_UART_H

#include "imx6ul.h"

void uart_Init(void);
void uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz);
void uart_disable(UART_Type *base);
void uart_enable(UART_Type *base);
void uart_softreset(UART_Type *base);
void uart1_io_Init(void);
void putc(uint8_t c);
void puts(int8_t *str);
uint8_t getc(void);

#endif // !_BSP_UART_H