#ifndef _BSP_AP3216_H
#define _BSP_AP3216_H

#include "imx6ul.h"

#define AP3216C_ADDR    	0X1E	/* AP3216C器件地址 */

/* AP3316C寄存器 */
#define AP3216C_SYSTEMCONG	0x00	/* 配置寄存器，bit(2:0)位，
                                    000：掉电模式(默认)，001：使能 ALS，010：使能 PS+IR，011：使能 ALS+PS+IR，100：软复位，101： ALS 单次模式，
                                    110： PS+IR 单次模式，111： ALS+PS+IR 单次模式。*/
#define AP3216C_INTSTATUS	0X01	/* 中断状态寄存器 */
#define AP3216C_INTCLEAR	0X02	/* 中断清除寄存器 */
#define AP3216C_IRDATALOW	0x0A	/* IR数据低字节，bit(7)位为0：IR&PS 数据有效，1:无效，bit(1:0)位为IR最低2位数据。*/
#define AP3216C_IRDATAHIGH	0x0B	/* IR数据高字节，bit(7:0) */
#define AP3216C_ALSDATALOW	0x0C	/* ALS数据低字节，bit(7:0)*/
#define AP3216C_ALSDATAHIGH	0X0D	/* ALS数据高字节，bit(7:0)*/
#define AP3216C_PSDATALOW	0X0E	/* PS数据低字节，bit(7)位为0:物体在远离，1：物体在接近，bit(6)位0：IR&PS数据有效，1：IR&PS数据无效，bit(3:0)，PS低4位数据*/
#define AP3216C_PSDATAHIGH	0X0F	/* PS数据高字节 bit(7)位为0:物体在远离，1：物体在接近，bit(6)位0：IR&PS数据有效，1：IR&PS数据无效，bit(5:0)，PS高6位数据**/

uint8_t ap3216_Init(void);
uint8_t ap3216_sendonebyte(uint8_t addr,uint8_t reg,uint8_t data);
uint8_t ap3216_receiveonebyte(uint8_t addr,uint8_t reg);
void ap3216_receivedata(uint16_t *ALS,uint16_t *PS,uint16_t *IR);

#endif // !_BSP_AP3216_H
