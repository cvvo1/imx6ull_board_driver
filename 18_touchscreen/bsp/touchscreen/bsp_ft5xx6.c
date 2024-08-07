#include "bsp_ft5xx6.h"
#include "bsp_gpio.h"
#include "bsp_int.h"
#include "bsp_i2c.h"
#include "bsp_delay.h"
#include "stdio.h"

/* 多点触摸屏简介
 * 1、多点触摸，不需要按下
 * 2、电容触摸屏需要一个IC驱动控制的，一般是I2C接口，多点触摸屏驱动最终就是一个I2C外设驱动。
 * 3、4.3寸都是GT9147 IC，7寸2款，一个是FT5206、一个是FT5426。
 * 4、触摸屏IP:
 *      CT_INT：触摸中断线，连接到了GPIO1_IO09
 *      I2C2_SCL：连接到了UART5_TXD
 *      I2C2_SDA：连接到了UART5_RXD
 *      CT_RST：连接到了SNVS_TAMPER9
 * 5、电容触摸芯片输出的触摸点坐标信息为对应的屏幕像素点信息，因此不需要校准。电阻屏需要校准。
 * 
 * FT54x6/FT52x6电容触摸芯片
 * 
 * 通过中断方式读取触摸电个数和触摸点坐标数据
 * 
*/
struct ft5xx6_dev_struc ft5426_dev;

/*
 * @description	: 初始化触摸屏，其实就是初始化FT5426
 * @param		: 无
 * @return 		: 无
*/
void ft5xx6_Init(void)
{
    uint8_t version_reg[2];
    ft5426_dev.initfalg = FT5426_INIT_NOTFINISHED;
	int i; 
	for( i = 0; i < 5; i++ )    /*最多五个触摸点*/
	{	/* 避免编译器自动赋值 */
		ft5426_dev.x[i] = 0;
		ft5426_dev.y[i] = 0;
	}
	ft5426_dev.point_num = 0;

    /*1、I2C IO初始化，硬件控制*/
    IOMUXC_SetPinMux(IOMUXC_UART5_TX_DATA_I2C2_SCL,1);  
    IOMUXC_SetPinMux(IOMUXC_UART5_RX_DATA_I2C2_SDA,1); 
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
    IOMUXC_SetPinConfig(IOMUXC_UART5_TX_DATA_I2C2_SCL,0x48b1);   /*设置电气属性*/
    IOMUXC_SetPinConfig(IOMUXC_UART5_RX_DATA_I2C2_SDA,0x48b1);    /*设置电气属性*/

    /*2、配置中断IO和复位IO，复用为GPIO开启中断*/
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO09_GPIO1_IO09,0);
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09,0);

    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO09_GPIO1_IO09,0xf080);          /*类似按键key中断电气属性，输入*/
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER9_GPIO5_IO09,0xf080);   /*类似led电气属性，输出*/

    /*初始化中断IO*/
    gpio_pin_config_t CTintpin_config;
    CTintpin_config.direction=KGPIO_DigitalInput;
    CTintpin_config.interruptMode=kGPIO_IntRisingOrFallingEdge;   /*跳变沿触发中断*/
    gpio_Init(GPIO1,9,&CTintpin_config);    /*设置GPIO1_IO18为输入，且下降沿触发中断*/    
    GIC_EnableIRQ(GPIO1_Combined_0_15_IRQn);       /*使能GIC对应中断，GPIO1-IO18对应的中断ID为67+32=99*/
    /* 注册中断服务函数 */
    system_register_irqhandler(GPIO1_Combined_0_15_IRQn,(system_irq_handler_t)gpio1_io09_irqhandle,NULL);  
    gpio_enable_int(GPIO1,18);   /* 使能GPIO1_IO18的中断功能 */

    /*初始化复位IO，软件控制*/
    gpio_pin_config_t CTrespin_config;
    CTrespin_config.direction=KGPIO_DigitalOutput;
    CTrespin_config.outputLogic=1;
    gpio_Init(GPIO5,0,&CTrespin_config);

    /*5、初始化I2C*/
    i2c_Init(I2C2);

    /*6、初始化FT5426*/
    gpio_pinwrite(GPIO5,0,0);   /*底板上CT_INT和CT_RST接上拉电阻，默认高电平，复位GPIO置低电平,复位FT5426 */
    delay_ms(20);
    gpio_pinwrite(GPIO5,0,1);
    delay_ms(20);               /*停止复位*/

    ft5xx6_sendbyte(FT5426_ADDR,FT5426_DEVICE_MODE,0x00);    /*设置进入正常模式*/
    ft5xx6_sendbyte(FT5426_ADDR,FT5426_IDG_MODE,0x01);      /*设置中断模式为触发模式1*/

    ft5xx6_receive_array(FT5426_ADDR,FT5426_IDGLIB_VERSION1,version_reg,2);
    printf("Touch Firmware version:%#x",((uint16_t)version_reg[0]<<8)|version_reg[1]);

    ft5426_dev.initfalg = FT5426_INIT_FINISHED;	/* 标记FT5426初始化完成 */
	ft5426_dev.intflag = 0;
}

/*
 * @description			: GPIO1_IO9最终的中断处理函数
 * @param				: 无
 * @return 				: 无
*/
void gpio1_io09_irqhandle(unsigned int giccIar, void *param)
{
    if(ft5426_dev.initfalg == FT5426_INIT_FINISHED)
	{
		//ft5426_dev.intflag = 1;
		ft5xx6_touch_coordinate();
	}
    gpio_clearintflags(GPIO1,9);     /*清除中断标志位*/
}

/*
 * @description	: 向FT5429写入数据
 * @param - addr: 设备地址
 * @param - reg : 要写入的寄存器
 * @param - data: 要写入的数据
 * @return 		: 操作结果
*/
uint8_t ft5xx6_sendbyte(uint8_t addr,uint8_t reg,uint8_t data)
{
    uint8_t status=0;
    uint8_t senddata=data;    /*因为发送数据需要数据地址，为了不修改原变量，采用一个中间变量*/
    struct i2c_transfer ft5xx6Xer;
    ft5xx6Xer.slaveAddress=addr;
    ft5xx6Xer.direction=kI2C_Send;
    ft5xx6Xer.subaddress=reg;
    ft5xx6Xer.subaddressSize=1;
    ft5xx6Xer.data=&senddata;
    ft5xx6Xer.dataSize=1;
    status=i2c_master_transfer(I2C2,&ft5xx6Xer);
    return status;
}

/*
 * @description	: 从FT5426读取一个字节的数据
 * @param - addr: 设备地址
 * @param - reg : 要读取的寄存器
 * @return 		: 读取到的数据。
*/
uint8_t ft5xx6_receivebyte(uint8_t addr,uint8_t reg)
{
    uint8_t temp=0;
    struct i2c_transfer ft5xx6Xer;
    ft5xx6Xer.slaveAddress=addr;
    ft5xx6Xer.direction=kI2C_Receive;
    ft5xx6Xer.subaddress=reg;
    ft5xx6Xer.subaddressSize=1;
    ft5xx6Xer.data=&temp;
    ft5xx6Xer.dataSize=1;
    i2c_master_transfer(I2C2,&ft5xx6Xer);
    return temp;
}

/*
 * @description	: 从FT5429读取多个字节的数据
 * @param - addr: 设备地址
 * @param - reg : 要读取的开始寄存器地址
 * @param - buf : 读取到的数据缓冲区
 * @param - len : 要读取的数据长度
 * @return 		: 无
*/
void ft5xx6_receive_array(uint8_t addr,uint8_t reg,uint8_t *array,uint8_t length)
{
    struct i2c_transfer ft5xx6Xer;
    ft5xx6Xer.slaveAddress=addr;
    ft5xx6Xer.direction=kI2C_Receive;
    ft5xx6Xer.subaddress=reg;
    ft5xx6Xer.subaddressSize=1;
    ft5xx6Xer.data=array;
    ft5xx6Xer.dataSize=length;
    i2c_master_transfer(I2C2,&ft5xx6Xer);
}

/*
 * @description	: 读取当前触摸点个数
 * @param 		: 无
 * @return 		: 无
*/
void ft5xx6_touch_num(void)
{
    ft5426_dev.point_num=ft5xx6_receivebyte(FT5426_ADDR,FT5426_TD_STATUS);   /*记录有多少个触摸点*/
}

/*
 * @description	: 读取当前所有触摸点的坐标
 * @param 		: 无
 * @return 		: 无
*/
void ft5xx6_touch_coordinate(void)
{
    uint8_t i;
    uint8_t type = 0;
    uint8_t point_array[FT5426_XYCOORDREG_NUM];
    ft5xx6_touch_num();
    /*一个触摸点用6个寄存器存储坐标数据，后两个寄存器保留，共30个寄存器*/
    ft5xx6_receive_array(FT5426_ADDR,FT5426_TOUCH1_XH,point_array,FT5426_XYCOORDREG_NUM);

    for(i=0;i<ft5426_dev.point_num;i++)
    {
        uint8_t *one_point=&point_array[i*6];

        ft5426_dev.x[i]=(uint16_t)(((one_point[0]&0x0f)<<8) |(one_point[1]));
        ft5426_dev.y[i]=(uint16_t)(((one_point[2]&0x0f)<<8) |(one_point[3]));
    
        type = one_point[0] >> 6;	/* 获取触摸类型 */
        /* 以第一个触摸点为例，寄存器TOUCH1_YH(地址0X05),各位描述如下：
	    * bit7:4  Touch ID  触摸ID，表示是哪个触摸点
	     bit3:0  Y轴触摸点的11~8位。
	    */
	    //id = (one_point[2] >> 4) & 0x0f;
        if(type == FT5426_TOUCH_EVENT_DOWN || type == FT5426_TOUCH_EVENT_ON )/* 按下 	*/
	    {
		
	    } else  {	/* 释放 */	
			
	    }
    }
}