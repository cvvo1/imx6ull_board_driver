#include "bsp_icm20608.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "bsp_delay.h"
#include "stdio.h"
/* ICM20608简介
 *  1、6轴MEMS传感器，包括3轴加速度和3轴加速度
 *  2、ICM-20608内部有一个512字节的FIFO，加速度和加速度计都是16位的ADC，
 *  3、加速度(角速度)的量程范围可以编程设置，可选择±250，±500，±1000 和±2000°/s，加速度的量程范围也可以编程设置，可选择±2g，±4g，±8g和±16g。
 *  4、WHO_AM_I寄存器，地址为0x75，默认为0xaf
 *  5、0X3B-0X48数据寄存器
 * 
 * 
 * 引脚复用 
 *  ECSPI3_SCLK——UART2_RXD；
 *  ECSPI3_MOSI——UART2_CTS；
 *  ECSPI3_SS0——UART2_TXD；
 *  ECSPI3_MISO——UART2_RTS
 * 
 * 
 * */
struct icm20608_dev_struc icm20608_dev;

uint8_t icm20608_Init(void)
{
    uint8_t IDvalue=0;
    /*1、IO复用*/
    IOMUXC_SetPinMux(IOMUXC_UART2_RX_DATA_ECSPI3_SCLK,0);  
    IOMUXC_SetPinMux(IOMUXC_UART2_CTS_B_ECSPI3_MOSI,0); 
    IOMUXC_SetPinMux(IOMUXC_UART2_RTS_B_ECSPI3_MISO ,0); 

    /* 配置SPI   SCLK MISO MOSI IO属性	
	 *bit 16: 0 HYS关闭
	 *bit [15:14]: 00 默认100K下拉
	 *bit [13]: 0 keeper功能
	 *bit [12]: 1 pull/keeper使能 
	 *bit [11]: 0 关闭开路输出
 	 *bit [7:6]: 10 速度100Mhz
 	 *bit [5:3]: 110 驱动能力为R0/6
	 *bit [0]: 1 高转换率
 	 */
    
    IOMUXC_SetPinConfig(IOMUXC_UART2_RX_DATA_ECSPI3_SCLK,0x10b1);   /*设置电气属性*/
    IOMUXC_SetPinConfig(IOMUXC_UART2_CTS_B_ECSPI3_MOSI,0x10b1);   /*设置电气属性*/
    IOMUXC_SetPinConfig(IOMUXC_UART2_RTS_B_ECSPI3_MISO,0x10b1);   /*设置电气属性*/

    /*需要通过控制片选SS0接口复用为GPIO口，软件控制SS0为低电平为icm20608开始信号*/
    IOMUXC_SetPinMux(IOMUXC_UART2_TX_DATA_GPIO1_IO20,0); 
    IOMUXC_SetPinConfig(IOMUXC_UART2_TX_DATA_GPIO1_IO20,0x10b1);   /*设置电气属性*/
    gpio_pin_config_t SS0_config;
    SS0_config.direction=KGPIO_DigitalOutput;
    SS0_config.outputLogic=1;                   /*默认低电平*/
    gpio_Init(GPIO1,20,&SS0_config);

    /*2、初始化SPI*/
    spi0_Init(ECSPI3);

    /*3、初始化icm20608*/
    icm20608_send_reg(ICM20_PWR_MGMT_1,0x80); /*复位icm20609，复位后芯片默认处于睡眠模式*/
    delay_ms(50);
    icm20608_send_reg(ICM20_PWR_MGMT_1,0x01);  /*关闭icm20609睡眠模式且自动选择时钟*/
    delay_ms(50);

    IDvalue=icm20608_receive_reg(ICM20_WHO_AM_I);
    if((IDvalue!=ICM20608G_ID)&&(IDvalue!=ICM20608D_ID))
    {
        return 1;
    }


    icm20608_send_reg(ICM20_SMPLRT_DIV,0x00);   /*设置输出速率，为不分频，为采样率1 kHz*/
    icm20608_send_reg(ICM20_CONFIG,0x05);       /*加速度低通滤波，3-dB BW为10 Hz，BW为频带带宽（终止频率-起始频率）*/
    icm20608_send_reg(ICM20_GYRO_CONFIG,0x00);   /*加速度量程为±250dps*/
    icm20608_send_reg(ICM20_ACCEL_CONFIG,0x00);   /*加速度计量程设置±2g*/
    icm20608_send_reg(ICM20_ACCEL_CONFIG2,0x05);   /*加速度计低通滤波设置，3-dB BW为10.2 Hz，BW为频带带宽（终止频率-起始频率）*/
    icm20608_send_reg(ICM20_LP_MODE_CFG,0x00);     /*关闭加速度低功耗模式*/
    icm20608_send_reg(ICM20_FIFO_EN,0x00);       /*关闭FIFO   */  
    icm20608_send_reg(ICM20_PWR_MGMT_2,0x00);      /*打开加速度计和加速度所有轴 */

    return 0;
}

/*操作CS，为0为开始信号，为1为终止信号*/
void icm20608_CSN(enum icm20608_CS key)
{
    if (key==icm20608_start)
    {
        gpio_pinwrite(GPIO1,20,0);    
    }else if(key==icm20608_stop)
    {
        gpio_pinwrite(GPIO1,20,1);
    }
}
/*
 * @description  : 写ICM20608指定寄存器
 * @param - addr  : 要读取的寄存器地址
 * @param - value: 要写入的值
 * @return		 : 无
*/
void icm20608_send_reg(uint8_t addr,const uint8_t data)
{   
    icm20608_CSN(0);
    /*ICM20608在使用SPI接口的时候寄存器地址只有低7位有效,寄存器地址最高位是读/写标志位读的时候要为1，写的时候要为0*/
    addr&=~(0x80);
    spich0_transfer_byte(ECSPI3,addr);
    spich0_transfer_byte(ECSPI3,data);
    icm20608_CSN(1);
}

/*
 * @description	: 读取ICM20608寄存器值
 * @param - addr: 要读取的寄存器地址
 * @return 		: 读取到的寄存器值
*/
uint8_t icm20608_receive_reg(uint8_t addr)
{
    uint8_t temp;
    icm20608_CSN(0);
    /*ICM20608在使用SPI接口的时候寄存器地址只有低7位有效,寄存器地址最高位是读/写标志位读的时候要为1，写的时候要为0*/
    addr|=(0x80);
    spich0_transfer_byte(ECSPI3,addr);
    temp=spich0_transfer_byte(ECSPI3,0xff);
    icm20608_CSN(1);
    return temp;
}

/*
 * @description	: 读取ICM20608连续多个寄存器
 * @param - addr: 要读取的寄存器地址
 * @param -array: 数组地址
 *  @param -array:数组长度
 * @return 		: 读取到的寄存器值
*/
void icm20608_receive_array(uint8_t addr,uint8_t *array,uint8_t len)
{   
    uint8_t i=0;
    addr|=(0x80);
    icm20608_CSN(0);
    spich0_transfer_byte(ECSPI3,addr);
    for(i=0;i<len;i++)
    {
        array[i]=spich0_transfer_byte(ECSPI3,0xff);
    }
    icm20608_CSN(1);
}

/*
 * @description : 获取加速度的分辨率
 * @param		: 无
 * @return		: 获取到的分辨率
*/
float icm20608_gyro_scale(void)    /*?*/
{
    uint8_t data;
    float gyro_scale;
    data=(icm20608_receive_reg(ICM20_GYRO_CONFIG)>>0x3)&0x3;   /*获取加速度量程，0：±250dps； 1：±500dps； 2：±1000dps；3：±2000dps*/
    switch (data)
    {
    case 0:   /*±250dps*/
        gyro_scale=65535/(250.0*2);
        break;
    case 1:   /*±500dps*/
        gyro_scale=65535/(500.0*2);
        break;
    case 2:   /*±1000dps*/
        gyro_scale=65535/(1000.0*2);
        break;
    case 3:   /*±2000dps*/
        gyro_scale=65535/(2000.0*2);
        break;
    }
    return gyro_scale;
}

/*
 * @description : 获取加速度的分辨率
 * @param		: 无
 * @return		: 获取到的分辨率
*/
float icm20608_accel_scale(void)
{
    uint8_t data;
    float accel_scale;
    data=(icm20608_receive_reg(ICM20_ACCEL_CONFIG)>>0x3)&0x3;   /*获取加速度量程，0：±2g； 1：±4g； 2：±8g； 3：±16g*/
    switch (data)
    {
    case 0:   /*±2g*/
        accel_scale=65535/(2.0*2);
        break;
    case 1:   /*±4g*/
        accel_scale=65535/(4.0*2);
        break;
    case 2:   /*±8g*/
        accel_scale=65535/(8.0*2);
        break;
    case 3:   /*±16g*/
        accel_scale=65535/(16.0*2);
        break;
    }
    return accel_scale;
}




/*
 * @description : 读取ICM20608的加速度、陀螺仪和温度原始值
 * @param 		: 无
 * @return		: 无
*/
void icm20608_data(void)
{
    float gyro_scale=0;
    float accel_scale=0;
    uint8_t data[14];
    gyro_scale=icm20608_gyro_scale();
    accel_scale=icm20608_accel_scale();

    icm20608_receive_array(ICM20_ACCEL_XOUT_H,data,14);

    /*先获取16位无符号整数再通过(int16_t)转化为有符号整数*/
    icm20608_dev.accel_x_adc=(int16_t)((data[0]<<8)|data[1]);
    icm20608_dev.accel_y_adc=(int16_t)((data[2]<<8)|data[3]);
    icm20608_dev.accel_z_adc=(int16_t)((data[4]<<8)|data[5]);
    icm20608_dev.temp_adc   =(int16_t)((data[6]<<8)|data[7]);
    icm20608_dev.gyro_x_adc =(int16_t)((data[8]<<8)|data[9]);
    icm20608_dev.gyro_y_adc =(int16_t)((data[10]<<8)|data[11]);
    icm20608_dev.gyro_z_adc =(int16_t)((data[12]<<8)|data[13]);

    /*扩大100倍，获得小数后两位*/
    icm20608_dev.accel_x_act=((float)(icm20608_dev.accel_x_adc)/accel_scale)*100;
    icm20608_dev.accel_y_act=((float)(icm20608_dev.accel_y_adc)/accel_scale)*100;
    icm20608_dev.accel_z_act=((float)(icm20608_dev.accel_z_adc)/accel_scale)*100;
    icm20608_dev.temp_act = (((float)(icm20608_dev.temp_adc)-25)/326.8+25)*100;
    icm20608_dev.gyro_x_act=((float)(icm20608_dev.gyro_x_adc)/gyro_scale)*100;
    icm20608_dev.gyro_y_act=((float)(icm20608_dev.gyro_y_adc)/gyro_scale)*100;
    icm20608_dev.gyro_z_act=((float)(icm20608_dev.gyro_z_adc)/gyro_scale)*100;
}


