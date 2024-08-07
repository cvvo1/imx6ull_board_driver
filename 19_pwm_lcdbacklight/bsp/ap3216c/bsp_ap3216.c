#include "bsp_ap3216.h"
#include "bsp_i2c.h"
#include "bsp_delay.h"
#include "stdio.h"


/* AP3216C简介
 * AP3216C是一个三合一的环境光传感器，ALS+PS+IRLED，ALS是环境光，PS是接近传感器，IR是红外LED灯。I2C接口，最高400Kbit/S的频率。
 * (1)ALS环境光16位输出。(2)PS接近传感器PS10bit输出。(3)IR红外LED灯传感器10bit输出
 * 同时打开 ALS、PS和IR则读取间隔最少要 112.5ms，
 * AP3216的设备地址为0X1E，
 * 
 * 
*/



uint8_t ap3216_Init(void)
{
    uint8_t data=0;
    /*1、IO初始化*/
    IOMUXC_SetPinMux(IOMUXC_UART4_RX_DATA_I2C1_SDA,1);  /*设置1为强制软件输入*/
    IOMUXC_SetPinMux(IOMUXC_UART4_TX_DATA_I2C1_SCL,1);  

    /* 
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 01 默认47K上拉，设置上下拉电阻
	 *bit [13]: 0 pull功能。当为0的时候使用状态保持器，当为1的时候使用上下拉。状态保持器在IO作为输入的时候才有用，就是当外部电路断电以后此IO口可以保持住以前的状态。
	 *bit [12]: 0 pull/keeper使能。这一位为0时禁止上下拉/状态保持器，为1时使能上下拉和状态保持器。 
	 *bit [11]: 1 开启开漏输出。这一位为0的时候禁止开漏输出，为1的时候就使能开漏输出功能。
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 110 驱动能力为R0/6，当IO用作输出的时候用来设置IO的驱动能力，可以简单理解为IO口上串联的电阻大小，电阻越小驱动能力越强，
	 *bit [0]: 1 高转换率，为1时IO电平跳变时间更快，对应波形更陡；为0时IO电平跳变时间要慢一些，波形也更平缓。
	*/

    IOMUXC_SetPinConfig(IOMUXC_UART4_RX_DATA_I2C1_SDA,0x48B1); /*设置电气属性*/ 
    IOMUXC_SetPinConfig(IOMUXC_UART4_TX_DATA_I2C1_SCL,0x48B1); /*设置电气属性*/  

    /*2、I2C初始化*/
    i2c_Init(I2C1);
    /*3、AP3216C初始化*/
    ap3216_sendonebyte(AP3216C_ADDR,AP3216C_SYSTEMCONG,0x04);  /*复位AP3216C，100-软复位*/
    delay_ms(50);
    ap3216_sendonebyte(AP3216C_ADDR,AP3216C_SYSTEMCONG,0x03);  /*011-使能 ALS+PS+IR*/
    data=ap3216_receiveonebyte(AP3216C_ADDR,AP3216C_SYSTEMCONG);    /*死循环在此*/
    printf("0X30data:%d\r\n",data);
    if(data==0x03)
    {
        return I2C_STATUS_OK;    /*AP3216C正常 */
    }else
    {
        return I2C_STATUS_FAIL;  /*AP3216C失败*/
    }
}

/*
 * @description	: 向AP3216C写入一个字节数据
 * @param - addr: 设备地址
 * @param - reg : 要写入的寄存器
 * @param - data: 要写入的数据
 * @return 		: 操作结果
*/
uint8_t ap3216_sendonebyte(uint8_t addr,uint8_t reg,uint8_t data)
{
    uint8_t status;
    uint8_t senddata=data;    /*因为发送数据需要数据地址，为了不修改原变量，采用一个中间变量*/
    struct i2c_transfer ap3216cXfer;
    ap3216cXfer.slaveAddress=addr;         /*设备地址*/
    ap3216cXfer.direction=kI2C_Send;       /*方向为发送*/
    ap3216cXfer.subaddress=reg;            /*寄存器地址*/
    ap3216cXfer.subaddressSize=1;          /*8位一个字节*/
    ap3216cXfer.data=&senddata;           /* 要写入的数据地址 */
    ap3216cXfer.dataSize=1;               /*数据一个字节*/              

    status=i2c_master_transfer(I2C1,&ap3216cXfer);
    return status;
}

/*
 * @description	: 从AP3216C读取一个字节的数据
 * @param - addr: 设备地址
 * @param - reg : 要读取的寄存器
 * @return 		: 读取到的数据。
*/
uint8_t ap3216_receiveonebyte(uint8_t addr,uint8_t reg)
{
    uint8_t temp=0;
    
    struct i2c_transfer ap3216cXfer;
    ap3216cXfer.slaveAddress=addr;         /*设备地址*/
    ap3216cXfer.direction=kI2C_Receive;       /*方向为发送*/
    ap3216cXfer.subaddress=reg;            /*寄存器地址*/
    ap3216cXfer.subaddressSize=1;          /*8位一个字节*/
    ap3216cXfer.data=&temp;           /* 要写入的数据地址 */
    ap3216cXfer.dataSize=1;               /*数据一个字节*/
    
    i2c_master_transfer(I2C1,&ap3216cXfer);

    return temp;    
}

/*
 * @description	: 读取AP3216C的数据，读取原始数据，包括ALS,PS和IR, 注意！
 *              : ALS是环境光，PS是接近传感器，IR是红外LED灯
 *				: 如果同时打开ALS,IR+PS的话两次数据读取的时间间隔要大于112.5ms
 * @param - ir	: ir数据
 * @param - ps 	: ps数据
 * @param - ps 	: als数据 
 * @return 		: 无。
*/
void ap3216_receivedata(uint16_t *ALS,uint16_t *PS,uint16_t *IR)
{
    uint8_t buf[6];
    uint8_t i=0;
    for(i=0;i<6;i++)
    {
        buf[i]=ap3216_receiveonebyte(AP3216C_ADDR,AP3216C_IRDATALOW+i);
    }
    /*IR数据*/ 
    if((buf[0]>>0x7)&0x1)   /*判断为无效*/
    {
        *IR=0;
    }else
    {
        *IR=((uint16_t)buf[1]<<2)|(buf[0]&0x03);
    }
    *ALS=((uint16_t)buf[3]<<8)|(buf[2]);

    if((buf[4]>>0x6)&0x1)   /*判断为无效*/
    {
        *PS=0;
    }else
    {
        *PS=(((uint16_t)buf[5]&0x3f)<<4)|(buf[4]&0x0f);
    }
    
}