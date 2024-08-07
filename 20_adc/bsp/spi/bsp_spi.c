#include "bsp_spi.h"

/* SPI协议
 * (1)、CS/SS，Slave Select/Chip Select，这个是片选信号线，用于选择需要进行通信的从设备。I2C主机是通过发送从机设备地址来选择需要进行通信的从机设备的， 
 *    SPI主机不需要发送从机设备，直接将相应的从机设备片选信号拉低即可
 * (2)、SCK，Serial Clock串行时钟，和I2C的SCL一样，为SPI通信提供时钟
 * (3)、MOSI/SDO，Master Out Slave In/Serial Data Output，简称主出从入信号线，这根数据线只能用于主机向从机发送数据，也就是主机输出，从机输入
 * (4)、MISO/SDI，Master In Slave Out/Serial Data Input，简称主入从出信号线，这根数据线只能用户从机向主机发送数据，也就是主机输入，从机输出
 * 
 * SPI时序
 * 1、起始条件：SS从高电平切换到低电平
 * 2、终止条件：SS从低电平切换到高电平
 * 3、SPI 有四种工作模式，通过串行时钟极性(CPOL)和相位(CPHA)的搭配来得到四种工作模式：
 * （1）、 CPOL=0，串行时钟空闲状态为低电平。
 * （2）、 CPOL=1，串行时钟空闲状态为高电平，此时可以通过配置时钟相位(CPHA)来选择具体的传输协议。
 * （3）、 CPHA=0，SCK第一个边沿移入数据(采样数据)，第二个边沿移出数据(输出数据)
 * （4）、 CPHA=1，SCK第一个边沿移出数据(输出数据)，第二个边沿移入数据(采样数据)
 * 
 * ALPHA开发板上通过ECSPI3接口连接了一个6轴传感器，引脚如下，片选为SS0：
 *      ECSPI3_SCLK : UART2_RX
 *      ECSPI3_MOSI：UART2_CTS
 *      ECSPI3_SS0：UART2_TXD
 *      ECSPI3_MISO: UART2_RTS
 * 
 * 1、初始化相应IO
 *  引脚复用和电气属性设置
 * 
 * 2、初始化SPI
 *  ECSPIx_CONREG寄存器：控制寄存器
 *      BURST_LENGTH(bit31:20)： 突发长度，设置SPI的突发传输数据长度，在一次SPI发送中最大可以发送2^12 bit数据，可以设置 0X000~0XFFF
 *          我们一般设置突发长度为一个字节，也就是 8bit，BURST_LENGTH=7
 *      CHANNEL_SELECT(bit19:18)： SPI通道选择，一个ECSPI有四个硬件片选信号，使用软件片选。可设置为 0~3，分别对应通道 0~3
 *          I.MX6U-ALPHA开发板上的ICM-20608的片选信号接的是ECSPI3_SS0，也就是 ECSPI3 的通道 0，所以本章实验设置为0
 *      DRCTL(bit17:16)：SPI的SPI_RDY信号控制位，用于设置SPI_RDY信号，为0的话不关心SPI_RDY信号；为1的话SPI_RDY信号为边沿触发；为2的话SPI_DRY是电平触发
 *      PRE_DIVIDER(bit15:12)：SPI预分频，可设置0~15，分别对应1~16分频。
 *      POST_DIVIDER(bit11:8)：SPI分频值，ECSPI时钟频率的第二步分频设置，分频值为2^POST_DIVIDER。
 *      CHANNEL_MODE(bit7:4)： SPI通道主/从模式设置，[3:0]分别对应SPI通道3~0，为0的话就是设置为从模式，如果为1的话就是主模式
 *      SMC(bit3)：开始模式控制，此位只能在主模式下起作用，为0的话通过XCH位来开启SPI突发访问，为1的话只要向TXFIFO写入数据就开启SPI突发访问。
 *      XCH(bit2)：此位只在主模式下起作用，当SMC为0的话此位用来控制SPI突发访问的开启。 ？
 *      HT(bit1)：HT模式使能位，I.MX6ULL 不支持。
 *      EN(bit0)：SPI使能位，为0的话关闭SPI，为1的话使能SPI
 *  ECSPIx_CONFIGREG寄存器：配置寄存器
 *      HT_LENGTH(bit28:24)： HT模式下的消息长度设置，I.MX6ULL 不支持。
 *      SCLK_CTL(bit23:20)：设置SCLK信号线空闲状态电平，[3:0]分别对应通道3~0为0的话SCLK空闲状态为低电平，为1的话SCLK空闲状态为高电平。
 *      DATA_CTL(bit19:16)：设置DATA信号线空闲状态电平，[3:0]分别对应通道3~0为0的话DATA空闲状态为高电平，为1的话DATA空闲状态为低电平。 
 *      SS_POL(bit15:12)：设置SPI片选信号极性设置，[3:0]分别对应通道 3~0为0的话片选信号低电平有效，为1的话片选信号高电平有效。
 *      SS_CTL(bit11:8):SPI-SS波形选择。在主模式中,当SMC（启动模式控制）位被清除时，该字段控制芯片选择（SS）信号的输出波形。如果设置了SMC位，则忽略SS_CTL。
 *      SCLK_POL(bit7:4)：SPI时钟信号极性设置(CPOL)，[3:0]分别对应通道3~0为0的话SCLK高电平有效(空闲的时候为低电平)，为1的话SCLK低电平有效(空闲的时候为高电平)。
 *      SCLK_PHA(bit3:0)：SPI时钟相位设置(CPHA)，[3:0]分别对应通道3~0，为0的话串行时钟的第一个跳变沿(上升沿或下降沿)采集数据，为1的话串行时钟的第二个跳变沿(上升沿或下降沿)采集数据。
 *  ECSPIx_PERIODREG寄存器：采样周期寄存器
 *      CSD_CTL(bit21:16)：片选信号延时控制位，用于设置片选信号和第一个SPI时钟信号之间的时间间隔，范围为0~63。
 *      CSRC(bit15)：SPI时钟源选择，为0的话选择SPI CLK为SPI的时钟源，为1的话选择32.768KHz的晶振为SPI时钟源，一般选择SPI CLK作为SPI时钟源
 *      SAMPLE_PERIO(bit14:0)：采样周期寄存器，可设置为 0~0X7FFF分别对应 0~32767个周期。
 *  时钟源设置
 *      设置pll3_sw_clk时钟源为PLL3(480 MHz)，CCM_CCSR寄存器PLL3_SW_CLK_SEL(bit0)为0
 *      设置ECSPI根时钟源为pll3_60m(8分频-60 MHz)，CCM_CSCDR2寄存器ECSPI_CLK_SEL(bit18)为0
 *      设置ECSPI时钟分频值，CCM_CSCDR2寄存器ECSPI_CLK_PODF(bit24:19)，分频值为2^ECSPI_CLK_PODF，设置为0。最终时钟为60 MHz
 *  ECSPIx_STATREG寄存器：状态寄存器
 *      TC(bit7)：传输完成标志位，为0表示正在传输，为1表示传输完成。
 *      RO(bit6)：RXFIFO溢出标志位，为0表示RXFIFO无溢出，为1表示RXFIFO溢出。
 *      RF(bit5)：RXFIFO空标志位，为0表示RXFIFO不为空，为1表示RXFIFO为空。
 *      RDR(bit4)：RXFIFO数据请求标志位，此位为0表示RXFIFO里面的数据不大于RX_THRESHOLD，此位为1的话表示RXFIFO里面的数据大于RX_THRESHOLD。
 *      RR(bit3)：RXFIFO 就绪标志位，为0的话RXFIFO没有数据，为1的话表示RXFIFO中至少有一个字的数据。
 *      TF(bit2)：TXFIFO满标志位，为0的话表示TXFIFO不为满，为1的话表示TXFIFO为满。
 *      TDR(bit1)：TXFIFO 数据请求标志位，为0表示TXFIFO中的数据大于TX_THRESHOLD，为1表示TXFIFO中的数据不大于TX_THRESHOLD。
 *      TE(bit0)：TXFIFO空标志位，为0表示TXFIFO中至少有一个字的数据，为1表示 TXFIFO为空。
 *  ECSPIx_TXDATA和ECSPIx_RXDATA，如果要发送数据就向寄存器ECSPIx_TXDATA写入数据，读取及存取ECSPIx_RXDATA里面的数据就可以得到刚刚接收到的数据。
 * 3、初始化ICM20608
 *  SPI最大频率为8 MHz
*/

void spi0_Init(ECSPI_Type *base)
{
    base->CONREG =0;  /*先清零*/
    /*突发长度为8bit；SPI通道0；不关心SPI_RDY信号；SPI0主模式；当向TXFIFO写入数据以后立即开启SPI突发；使能SPI0*/
    base->CONREG|=(7<<20)|(1<<4)|(1<<3)|(1<<0);
    base->CONFIGREG=0;  /*先清零*/
    /*SPI0的SCLK空闲为低电平；数据空闲为高电平；SPI片选信号低电平有效；设置了SMC忽略SPI-SS波形选择；模式一 CPOL=0 CPHA=0*/
    
    base->PERIODREG=0;  /*先清零*/
    /*片选延时为0；选择SPI CLK为SPI的时钟源；读取数据的时候每次之间间隔0.1 ms,(1/(7.5MHz)*750)=0.1ms*/
    base->PERIODREG=0x2ee;  

    /*时钟源选择*/
    CCM->CCSR&=~(1<<0);
    CCM->CSCDR2&=~((1<<18)|(1<<19));

    /*ECSPI_CLK为60 MHz，设置SPI频率为7.5MHz*/
    base->CONREG&=~((0xf<<12)|(0xf<<8));   /*清除之前分频器设置*/
    base->CONREG|=(0x7<<12);    /*预分频为(7+1),后分频为0,为7.5 MHz*/
}

/*
 * @description		: SPI通道0发送/接收一个字节的数据
 * @param - base	: 要使用的SPI
 * @param - txdata	: 要发送的数据
 * @return 			: 无
*/
uint8_t spich0_transfer_byte(ECSPI_Type *base,uint8_t data)
{
    uint32_t Rxdata=0;   /*传输数据必须为32位字，高位舍弃*/
    uint32_t Txdata=data;

    /* 选择通道0 */
	base->CONREG &= ~(3 << 18);
	base->CONREG |= (0 << 18);

    /*等待TXFIFO空标志位，为1表示为空，需要输入数据*/
    while(((base->STATREG>>0)&0x1)!=1);    
    base->TXDATA=Txdata;

    /*等待RXFIFO就绪标志位,为1至少有一个字的数据需要读取*/
    while(((base->STATREG>>3)&0x1)!=1);     
    Rxdata=base->RXDATA;
    return  (uint8_t)Rxdata;
}