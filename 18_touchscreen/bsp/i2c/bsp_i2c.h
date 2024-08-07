#ifndef _BSP_I2C_H
#define _BSP_I2C_H
#include "imx6ul.h"

/*定义I2C状态相关宏*/
#define I2C_STATUS_OK				(0)     
#define I2C_STATUS_BUSY				(1)     /*忙状态*/
#define I2C_STATUS_IDLE				(2)     /*空闲状态*/
#define I2C_STATUS_NAK				(3)     /*没接收应答*/
#define I2C_STATUS_ARBITRATIONLOST	(4)     /*仲裁丢失*/
#define I2C_STATUS_TIMEOUT			(5)     /*超时错误*/
#define I2C_STATUS_ADDRNAK			(6)    /*从机地址no ack错误*/
#define I2C_STATUS_FAIL			    (6)    /*I2C失败*/

/*I2C方向枚举类型*/
enum i2c_direction
{
    kI2C_Send = 0x0, 		/* 主机向从机写数据（发送） */
    kI2C_Receive = 0x1,  		/* 主机从从机读数据（接收）*/
} ;

/*主机传输结构体*/
struct i2c_transfer
{
    uint8_t slaveAddress;      	    /* 7位从机地址 */
    enum i2c_direction direction; 	/* 传输方向 */
    uint32_t subaddress;       		/* 寄存器地址*/
    uint32_t subaddressSize;    	/* 寄存器地址长度 */
    uint8_t *volatile data;    	    /* 数据缓冲区 */
    volatile uint32_t dataSize;  	/* 数据缓冲区长度 */
};

void i2c_Init(I2C_Type *base);
uint8_t i2c_master_start(I2C_Type *base,uint8_t address,enum i2c_direction direction);
uint8_t i2c_master_stop(I2C_Type *base);
uint8_t i2c_master_repeated_start(I2C_Type *base,uint8_t address,enum i2c_direction direction);
uint8_t i2c_check_clear_error(I2C_Type *base,uint32_t state);
void i2c_master_send(I2C_Type *base,const uint8_t *buf,uint32_t size);
void i2c_master_receive(I2C_Type *base,uint8_t *buf,uint32_t size);
uint8_t i2c_master_transfer(I2C_Type *base,struct i2c_transfer *xfer);   /*i2c只使用这个函数*/


#endif // !_BSP_I2C_H