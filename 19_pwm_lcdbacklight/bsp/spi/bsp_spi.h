#ifndef _BSP_SPI_H
#define _BSP_SPI_H
#include "imx6ul.h"

void spi0_Init(ECSPI_Type *base);
uint8_t spich0_transfer_byte(ECSPI_Type *base,uint8_t data);

#endif // !_BSP_SPI_H
