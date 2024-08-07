#include "bsp_i2c.h"
#include "stdio.h"

/* I2C简介
 * （1）一条是SCL(串行时钟线)，另外一条是SDA(串行数据线)，这两条数据线需要接上拉电阻4.7k(弱上拉)，总线空闲的时候SCL和SDA处于高电平。
 * （2）I2C总线支持多从机，通过从机地址来区分访问哪个从机
 * 
 * 6ULL I2C接口
 *  ALPHA开发板上一个IIC接口的器件AP3216C(环境光传感器)，I2C1_SCL: 使用的是UART4_TXD这个IO，复用位ALT2。I2C1_SDA: 使用的是UART4_RXD这个IO，复用为ALT2。
 *  6UL的I2C频率标准模式100kbit/S，快速模式400Kbit/S
 *  时钟源选择perclk_clk_root=ipg_clk_root=66MHz
 * 
 * I2C时序，SCL和SDA默认高电平
 *  1、起始位：在SCL为高电平的时候，SDA出现下降沿就表示为起始位
 *  2、停止位：在SCL为高电平的时候，SDA出现上升沿就表示为停止位
 *  3、数据传输：在SCL高电平期间，SDA上的数据稳定，因此SDA上的数据变化只能在SCL低电平期间发生
 *  4、应答信号：当I2C主机发送完8位数据后一个时钟信号就是给应答信号使用的，等待I2C从机应答。从机通过将SDA拉低来表示发出应答信号，表示通信成功，否则表示通信失败。
 *  5、I2C写时序：1)、开始信号。
 *              2)、发送I2C设备地址，每个I2C器件都有一个设备地址其中高7位是设备地址，最后1位是读写位
 *              3)、I2C器件地址后面跟着一个读写位，为0表示写操作，为1表示读操作
 *              4)、从机发送的ACK应答信号
 *              5)、重新发送开始信号
 *              6)、发送要写入数据的寄存器地址+应答位
 *              7)、从机发送的ACK应答信号
 *              8)、发送要写入寄存器的数据
 *              9)、从机发送的ACK应答信号
 *              10)、停止信号
 *  6、I2C读时序：1)、主机发送起始信号
 *              2)、主机发送要读取的I2C从设备地址
 *              3)、读写控制位，因为是向I2C从设备发送数据，因此是写信号
 *              4)、从机发送的ACK应答信号
 *              5)、重新发送START信号
 *              6)、主机发送要读取的寄存器地址+写位
 *              7)、从机发送的ACK应答信号
 *              8)、重新发送START信号
 *              9)、重新发送要读取的I2C从设备地址
 *              10)、读写控制位，这里是读信号，表示接下来是从I2C从设备里面读取数据
 *              11)、从机发送的ACK应答信号
 *              12)、从I2C器件里面读取到的数据。
 *              13)、主机发出NO ACK 信号，表示读取完成，不需要从机再发送ACK信号了。
 *              14)、主机发出STOP信号，停止I2C 通信。
 * 
 * 1、初始化相应的IO。初始化I2C1相应的IO，设置其复用功能，如果要使用AP3216C中断功能的话，还需要设置AP3216C的中断IO。
 * 2、初始化I2C1。初始化I2C1接口，设置波特率。
 *  I2Cx_IADR寄存器：从机地址
 *      I2Cx_IADR保存I2C作为从机寻址时响应的地址。从地址不是在地址传输期间在总线上发送的地址
 *  I2Cx_IFDR寄存器：分频器寄存器，IPG_CLK_ROOT=66 MHz
 *      IC(bit5:0)位，设置特定的I2C波特率，需要设置I2C的波特率为400KHz，则IC=0x0d或0x30(160分频)
 *  I2Cx_I2CR寄存器：控制寄存器
 *      IEN(bit7)：I2C使能位，为1的时候使能 I2C，为0的时候关闭I2C
 *      IIEN(bit6)：I2C中断使能位，为1的时候使能I2C中断，为0的时候关闭I2C中断
 *      MSTA(bit5)：主从模式选择位，设置IIC工作在主模式还是从模式，为1的时候工作在主模式，为0的时候工作在从模式
 *      MTX(bit4)：传输方向选择位，用来设置是进行发送还是接收，为0的时候是接收，为1的时候是发送
 *      TXAK(bit3)：传输应答位使能(读取时)，为0的话发送ACK信号，为1的话发送NO ACK信号
 *      RSTA(bit2)：重复开始信号，读取时始终为0，为1的话产生一个重新开始信号
 *  I2Cx_I2SR寄存器：状态寄存器
 *      ICF(bit7)：数据传输状态位，为0的时候表示数据正在传输，为1的时候表示数据传输完成
 *      IAAS(bit6)：当为1的时候表示I2C地址，也就是I2Cx_IADR寄存器中的地址是从设备地址。写入I2C_I2CR将清除此位。
 *      IBB(bit5)：I2C总线忙标志位，当为0的时候表示I2C总线空闲，为1的时候表示I2C总线忙
 *      IAL(bit4)：仲裁丢失位，为1的时候表示发生仲裁丢失
 *      SRW(bit2)：从机读写状态位，当I2C作为从机的时候使用，此位用来表明主机发送给从机的是读还是写命令。为0的时候表示主机要向从机写数据，为1的时候表示主机要从从机读取数据
 *      IIF(bit1)：I2C中断挂起标志位，当为1的时候表示有中断挂起，此位需要软件清零
 *      RXAK(bit0)：应答信号标志位(发送时)，为0的时候表示接收到ACK应答信号，为1的话表示检测到NO ACK信号
 *  I2Cx_I2DR寄存器：
 *      保存最后一个接收到的数据字节或下一个要传输的数据字节。软件写入下一个要传输的数据字节或读取接收到的数据字节。
 * 3、初始化AP3216C。初始化AP3216C，读取AP3216C的数据。
 *  参考bsp_ap3215c.c
 * 
*/

/*
 * @description		: 初始化I2C，波特率400KHZ
 * @param - base 	: 要初始化的IIC设置
 * @return 			: 无
*/
void i2c_Init(I2C_Type *base)
{
    base->I2CR &=~(1<<7);    /* 要访问I2C的寄存器，首先需要先关闭I2C */
    base->IFDR =(0X30<<0);   /*I2C的时钟源来源于IPG_CLK_ROOT=66Mhz，设置160分频*/
    base->I2CR |=(1<<7);    /*开启I2C*/
}

/*
 * @description			: 发送开始信号+从设备地址
 * @param - base 		: 要使用的I2C
 * @param - addrss		: 设备地址
 * @param - direction	: 方向
 * @return 				: 0 正常 其他值 出错
 */
uint8_t i2c_master_start(I2C_Type *base,uint8_t address,enum i2c_direction direction)
{
    if((base->I2SR>>0x5)&0x1)    /*判断是否忙状态*/
    {
        return I2C_STATUS_BUSY;    /*返回忙状态*/
    }
    base->I2CR|=(1<<5)|(1<<4);    /*设计为主机模式和发送(写)*/
    base->I2DR =((uint32_t)address<<1)|(direction);    /*7位地址+读写控制位*/

    // base->I2DR=((unsigned int)address<<1)|((direction == kI2C_Read)?1:0);    /*A?B:C——若A成立则返回B，不成立则返回C*/
    return I2C_STATUS_OK;    /*发送正常*/
}

/*
 * @description		: 停止信号，读取时(接收数据模式)，在读取最后一个字节之前，必须产生停止信号。
 * @param - base	: 要使用的IIC
 * @param			: 无
 * @return 			: 状态结果
*/
uint8_t i2c_master_stop(I2C_Type *base)
{
    uint16_t timeout=0xffff;    /*设置超时时间*/
    base->I2CR &=~((1 << 5)|(1 << 4)|(1 << 3));  /*设置为从模式、接收模式、发送ACK信号*/
    /*等待忙结束*/
    while((base->I2SR>>0x5)&0x1)
    {
        timeout--;
        if(timeout==0)  /*超时跳出*/
        {
            return I2C_STATUS_TIMEOUT;
        }
    }
    return I2C_STATUS_OK;
}

/*
 * @description			: 发送重新开始信号
 * @param - base 		: 要使用的IIC
 * @param - addrss		: 设备地址
 * @param - direction	: 方向
 * @return 				: 0 正常 其他值 出错
*/

uint8_t i2c_master_repeated_start(I2C_Type *base,uint8_t address,enum i2c_direction direction)
{
    if((((base->I2CR>>0x5)&0x1)==0))    /*判断工作在从机模式下*/
    {
        return I2C_STATUS_BUSY;    /*返回忙状态*/
    }
    base->I2CR|=(1<<4)|(1<<2);    /*设计为主机模式、发送(写)和重新开始信号*/
    base->I2DR =((uint32_t)address<<1)|(direction);    /*7位地址+读写控制位*/

    // base->I2DR=((unsigned int)address<<1)|((direction == kI2C_Read)?1:0);    /*A?B:C——若A成立则返回B，不成立则返回C*/
    return I2C_STATUS_OK;    /*发送正常*/
}

/*
 * @description		: 传输完成后(检查IIF中断标志为1)，检查并清除错误，
 * @param - base 	: 要使用的IIC
 * @param - status	: 状态
 * @return 			: 状态结果
*/
uint8_t i2c_check_clear_error(I2C_Type *base,uint32_t state)
{
    /*先检查是否为总裁丢失错误*/
    if((state>>0x4)&0x1)     /*发生总裁丢失*/
    {
        base->I2SR&=~(1<<4);   /*先清除标志位*/
        base->I2CR &=~(1<<7);    /* 要访问I2C的寄存器，首先需要先关闭I2C */
        base->I2CR |=(1<<7);    /*开启I2C*/
        return I2C_STATUS_ARBITRATIONLOST;
    }
    else if((state>>0)&0x1)    /* 没有接收到从机的应答信号，检测到NO ACK信号 */
    {
        return I2C_STATUS_NAK;
    }
    return I2C_STATUS_OK;
}

/*
 * @description		: 发送数据，不需要开始
 * @param - base 	: 要使用的IIC
 * @param - buf		: 要发送的数据———const修饰普通变量实际上就是定义了一个常量，const修饰的类型为TYPE的变量value是不可变的
 * @param - size	: 要发送的数据大小
 * @param - flags	: 标志
 * @return 			: 无
*/
void i2c_master_send(I2C_Type *base,const uint8_t *buf,uint32_t size)
{
    while(!((base->I2SR>>0x7)&0x1));    /*等待传输完成*/

    base->I2SR &=~(1<<1);    /*I2C中断挂起标志位，此位需要软件清零*/
    base->I2CR |=(1<<4);    /*设置传输方向为发送*/
    while(size--)
    {
        base->I2DR = *buf++;    /*指针后移并取值*/
        while(!((base->I2SR>>0x1)&0x1));   /*一个字节通信完成后，会设置I2C中断挂起标志位为1，即传输完成*/
        base->I2SR &=~(1<<1);    /*必须清除标志位*/
        if(i2c_check_clear_error(base,base->I2SR))   /*检查ack应答位*/
        {
            break;
        }
    }
    base->I2SR &= ~(1 << 1);
	i2c_master_stop(base); 	/* 发送停止信号 */
}


/*
 * @description		: 接收数据
 * @param - base 	: 要使用的IIC
 * @param - buf		: 读取到数据
 * @param - size	: 要读取的数据大小
 * @return 			: 无
*/

void i2c_master_receive(I2C_Type *base,uint8_t *buf,uint32_t size)
{
    uint8_t temp=0;
    temp++; 	/* 防止编译报错 */
    
    while(!((base->I2SR>>0x7)&0x1));    /*等待传输完成*/
    base->I2SR &=~(1<<1);    /*I2C中断挂起标志位，此位需要软件清零*/
    base->I2CR &=~((1<<4)|(1<<3));   /* 接收数据，设置为接收模式，且发送ack应答*/

    if(size==1)  /*如果只接收一个字节数据的话发送NACK信号 */
    {
        base->I2CR |=(1<<3); 
    }
    temp=base->I2DR;           /*需要假读，因为在读取最后一个字节之前，已经发送no ack停止信号*/
    /*数据传输位(I2C_I2SR[ICF])通过在接收模式下读取I2C_I2DR或在发送模式下写入该寄存器而被清除。*/

    while(size--)    /*当size不为0时循环，先执行循环判断，再执行--*/
    {
        while(!((base->I2SR>>0x1)&0x1));    /*一个字节通信完成后，会设置I2C中断挂起标志位为1，即传输完成*/
        base->I2SR &=~(1<<1);    /*必须清除标志位*/
        if(size==0)
        {
            i2c_master_stop(base);
        }
        if(size==1)
        {
            base->I2CR |= (1 << 3);     /*需要提前一个字节发送no ack*/
        }
        *buf++ = base->I2DR;
    }
}

/*
 * @description	: I2C数据传输，包括读和写
 * @param - base: 要使用的IIC
 * @param - xfer: 传输结构体
 * @return 		: 传输结果,0 成功，其他值 失败;
*/

uint8_t i2c_master_transfer(I2C_Type *base,struct i2c_transfer *xfer)
{
    uint8_t temp;    /*返回I2C状态标志*/
    enum i2c_direction direction = xfer->direction;   /*传输方向中间变量*/

	base->I2SR &=~((1<<1)|(1<<4));			/*清除标志位，I2C中断挂起标志位和仲裁丢失位*/
	while(!((base->I2SR >> 7) & 0X1));   /*等待传输完成*/

    /* 如果是读的话，要先发送寄存器地址，所以要先将方向改为发送 */
    if ((xfer->subaddressSize > 0) && (xfer->direction == kI2C_Receive))
    {
        direction = kI2C_Send;
    }

    temp=i2c_master_start(base,xfer->slaveAddress,direction); /*发送开始信号，包括从机地址和读写位*/
    if(temp)    /*如果传输失败，则返回I2C状态*/
    {
        return temp;
    }
    
    while(!((base->I2SR>>0x1)&0x1));    /*一个字节通信完成后，会设置I2C中断挂起标志位为1，即等待传输完成*/
    temp = i2c_check_clear_error(base, base->I2SR);	/* 检查是否出现传输错误 */
    if(temp)
    {
      	i2c_master_stop(base); 						/* 发送出错，发送停止信号 */
        return temp;
    }

    /* 发送两种模式下寄存器地址，如果一个字节则直接发送，如果多个字节则按一个字节多次发送*/
    if(xfer->subaddressSize)    /*存在寄存器地址则发送*/
    {
        do    /*do-while为后循环，先进入循环体，再执行循环语句后再对判断while()中表达式*/
        {
			base->I2SR &=~(1<<1);       /*必须清除IIF中断标志位*/
            xfer->subaddressSize--;				/* 地址长度减一 */
			
            base->I2DR =  ((xfer->subaddress) >> (8 * xfer->subaddressSize)); //向I2DR寄存器写入子地址，先发送高位地址
  
			while(!((base->I2SR>>0x1)&0x1));    /*一个字节通信完成后，会设置I2C中断挂起标志位为1，即传输完成*/

            /* 传输完成后需要检查是否有错误发生 */
            temp = i2c_check_clear_error(base, base->I2SR);
            if(temp)
            {
             	i2c_master_stop(base); 				/* 发送停止信号 */
             	return temp;
            }  
        } while ((xfer->subaddressSize > 0) && (temp == I2C_STATUS_OK));

    /*接收数据需要重复开始信号和从机地址*/
        if(xfer->direction==kI2C_Receive)    
        {
            base->I2SR &=~(1<<1);    /*必须清除IIF中断标志位*/
            temp=i2c_master_repeated_start(base,xfer->slaveAddress,xfer->direction);
            if(temp)    /*如果传输失败，则返回I2C状态*/
            {
                return temp;
            }
            while(!((base->I2SR>>0x1)&0x1));    /*一个字节通信完成后，会设置I2C中断挂起标志位为1，即传输完成*/
            temp = i2c_check_clear_error(base, base->I2SR);	/* 检查是否出现传输错误 */
            if(temp)
            {
                i2c_master_stop(base);
                return I2C_STATUS_ADDRNAK;  /*返回从机地址no ack错误*/
            }
        }
    }
    /*发送数据*/
    if((xfer->direction==kI2C_Send)&&(xfer->dataSize>0))
    {
        i2c_master_send(base,xfer->data,xfer->dataSize);
    }
    
    /*接收数据*/
    if((xfer->direction==kI2C_Receive)&&(xfer->dataSize>0))
    {
        i2c_master_receive(base,xfer->data,xfer->dataSize);
    }
    return I2C_STATUS_OK;
}

