#include "bsp_uart.h"

/* 1、设置UART1的时钟源。设置UART的时钟源为pll3_80m，设置寄存器CCM_CSCDR1的UART_CLK_SEL(bit6)位为0，时钟源为80MHz，否则为24MHz，默认为0且一分频
 * 2、初始化UART1。初始化UART1所使用IO，设置UART1的寄存器 UART1_UCR1~UART1_UCR3，设置内容包括波特率，奇偶校验、停止位、数据位等等。
 *      设置UART的UCR1寄存器ADBR(bit14)为0，关闭自动波特率检测。UARTEN(bit0)：UART使能位，为0的时候关闭UART，为1的时候使能UART。
 *      设置UART的UCR2寄存器：
 *          IRTS(bit14)：为0的时候使用RTS引脚功能，为1的时候忽略RTS引脚。
 *          PREN(bit8)：奇偶校验使能位，为0的时候关闭奇偶校验，为1的时候使能奇偶校验。
 *          PROE(bit7)：奇偶校验模式选择位，开启奇偶校验以后此位如果为0的话就使用偶校验，此位为1的话就使能奇校验。
 *          STOP(bit6)：停止位数量，为0的话1位停止位，为1的话2位停止位。
 *          WS(bit5)：数据位长度，为0的时候选择7位数据位，为1的时候选择8位数据位。
 *          TXEN(bit2)：发送使能位，为0的时候关闭UART的发送功能，为1的时候打开UART的发送功能。
 *          RXEN(bit1)：接收使能位，为0的时候关闭UART的接收功能，为1的时候打开UART的接收功能。
 *          SRST(bit0)：软件复位，为0的是时候软件复位UART，为1的时候表示复位完成。复位完成以后此位会自动置1，表示复位完成。此位只能写0，写1会被忽略掉。
 *      设置UART的UCR3寄存器：RXDMUXSEL(bit2)，这个位应该始终为 1。
 *      设置UART_UFCR寄存器位FDIV(bit9:7)，用来设置参考时钟分频，与UARTx_UBMR和UARTx_UBIR这三者的配合即可得到我们想要的波特率。
 * 3、使能UART1。UART1初始化完成以后就可以使能UART1了，设置寄存器UART1_UCR1的UARTEN位(bit0)为1
 * 4、编写UART1数据收发函数。编写两个函数用于UART1的数据收发操作。
 *          发送一个字符，需要等待UART_USR2寄存器位TXDC(bit3)，为0传输未完成，为1传输完成
 *          接收一个字符，需要等待UART_USR2寄存器位RDR(bit0)，为0没有接收数据就绪，为1接收数据就绪
 *          
 * 
 * 波特率计算公式
 *      Boud Rate=Ref Freq/(16×(UBMR+1)/(UBIR+1))
 *  Ref Freq：经过分频以后进入UART的最终时钟频率(80MHz)；UBMR：寄存器UARTx_UBMR中的值；UBIR：寄存器UARTx_UBIR中的值。
 *  UBIR寄存器必须在UBMR寄存器之前更新
*/

/*初始化uart*/
void uart_Init(void)
{
    /*初始化UART1的IO*/
    uart1_io_Init();

    /*初始化UART1*/
    uart_disable(UART1);    /*关闭UART*/
    uart_softreset(UART1);  /*复位UART*/
    
    /*配置UART1，数据位、奇偶校验、停止位等*/
    UART1->UCR1 = 0;    /*先清除UCR1寄存器*/
    UART1->UCR1 &=~(1<<14);   /*关闭自动波特率检测*/

    UART1->UCR2 = 0;    /*清除UCR2寄存器*/
    UART1->UCR2 |=(1<<14)|(1<<5)|(1<<2)|(1<<1);   /*忽略RTS引脚、关闭奇偶校验、1位停止位、8位数据位、打开UART发送接收功能*/

    UART1->UCR3 |=(1<<2);   /*在该芯片中，UART用于MUXED模式，因此应始终设置此位*/
#if 0
    /*设置波特率为1152000*/
    UART1->UFCR &=~(0x7<<7);   /*先清零*/
    UART1->UFCR |=(0x5<<7);    /*设置为101，设置为1分频，时钟源为80MHz*/ 
    UART1->UBIR = 71;
    UART1->UBMR = 3124;
#endif

    uart_setbaudrate(UART1,115200,80000000);   /*设置波特率*/

    /*使能UART1*/
    uart_enable(UART1);
}

/*
 * @description 		: 波特率计算公式，
 *    			  	  	  可以用此函数计算出指定串口对应的UFCR，
 * 				          UBIR和UBMR这三个寄存器的值
 * @param - base		: 要计算的串口。
 * @param - baudrate	: 要使用的波特率。
 * @param - srcclock_hz	:串口时钟源频率，单位Hz
 * @return		: 无
 */
void uart_setbaudrate(UART_Type *base, unsigned int baudrate, unsigned int srcclock_hz)
{
    uint32_t numerator = 0u;		//分子
    uint32_t denominator = 0U;		//分母
    uint32_t divisor = 0U;
    uint32_t refFreqDiv = 0U;
    uint32_t divider = 1U;
    uint64_t baudDiff = 0U;
    uint64_t tempNumerator = 0U;
    uint32_t tempDenominator = 0u;

    /* get the approximately maximum divisor */
    numerator = srcclock_hz;
    denominator = baudrate << 4;
    divisor = 1;

    while (denominator != 0)
    {
        divisor = denominator;
        denominator = numerator % denominator;
        numerator = divisor;
    }

    numerator = srcclock_hz / divisor;
    denominator = (baudrate << 4) / divisor;

    /* numerator ranges from 1 ~ 7 * 64k */
    /* denominator ranges from 1 ~ 64k */
    if ((numerator > (UART_UBIR_INC_MASK * 7)) || (denominator > UART_UBIR_INC_MASK))
    {
        uint32_t m = (numerator - 1) / (UART_UBIR_INC_MASK * 7) + 1;
        uint32_t n = (denominator - 1) / UART_UBIR_INC_MASK + 1;
        uint32_t max = m > n ? m : n;
        numerator /= max;
        denominator /= max;
        if (0 == numerator)
        {
            numerator = 1;
        }
        if (0 == denominator)
        {
            denominator = 1;
        }
    }
    divider = (numerator - 1) / UART_UBIR_INC_MASK + 1;

    switch (divider)
    {
        case 1:
            refFreqDiv = 0x05;
            break;
        case 2:
            refFreqDiv = 0x04;
            break;
        case 3:
            refFreqDiv = 0x03;
            break;
        case 4:
            refFreqDiv = 0x02;
            break;
        case 5:
            refFreqDiv = 0x01;
            break;
        case 6:
            refFreqDiv = 0x00;
            break;
        case 7:
            refFreqDiv = 0x06;
            break;
        default:
            refFreqDiv = 0x05;
            break;
    }
    /* Compare the difference between baudRate_Bps and calculated baud rate.
     * Baud Rate = Ref Freq / (16 * (UBMR + 1)/(UBIR+1)).
     * baudDiff = (srcClock_Hz/divider)/( 16 * ((numerator / divider)/ denominator).
     */
    tempNumerator = srcclock_hz;
    tempDenominator = (numerator << 4);
    divisor = 1;
    /* get the approximately maximum divisor */
    while (tempDenominator != 0)
    {
        divisor = tempDenominator;
        tempDenominator = tempNumerator % tempDenominator;
        tempNumerator = divisor;
    }
    tempNumerator = srcclock_hz / divisor;
    tempDenominator = (numerator << 4) / divisor;
    baudDiff = (tempNumerator * denominator) / tempDenominator;
    baudDiff = (baudDiff >= baudrate) ? (baudDiff - baudrate) : (baudrate - baudDiff);

    if (baudDiff < (baudrate / 100) * 3)
    {
        base->UFCR &= ~UART_UFCR_RFDIV_MASK;
        base->UFCR |= UART_UFCR_RFDIV(refFreqDiv);
        base->UBIR = UART_UBIR_INC(denominator - 1); //要先写UBIR寄存器，然后再写UBMR寄存器，3592页 
        base->UBMR = UART_UBMR_MOD(numerator / divider - 1);
    }
}

/*
 * @description : 关闭指定的UART
 * @param - base: 要关闭的UART
 * @return		: 无
*/
void uart_disable(UART_Type *base)
{
    base->UCR1 &=~(1<<0);
}

/*
 * @description : 打开指定的UART
 * @param - base: 要打开的UART
 * @return		: 无
*/
void uart_enable(UART_Type *base)
{
    base->UCR1 |=(1<<0);
}

/*
 * @description : 复位指定的UART
 * @param - base: 要复位的UART
 * @return		: 无
*/
void uart_softreset(UART_Type *base)
{
    base->UCR2 &=~(1<<0);    /*设置UART_UCR2的SRST位(bit0)为0，进行软件复位*/
    while((base->UCR2 & 0x1) == 0); /* 等待复位完成*/
}

/*
 * @description : 初始化串口1所使用的IO引脚
 * @param		: 无
 * @return		: 无
*/
void uart1_io_Init(void)
{
    /* 1、初始化IO复用 
     * UART1_RXD -> UART1_TX_DATA
     * UART1_TXD -> UART1_RX_DATA
	*/
    IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX,0);  /*复用为GPIO1_IO18*/
    IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX,0);  /*复用为GPIO1_IO18*/

    /* 2、设置电气属性 */
    IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX,0x10b0);   /*设置电气属性*/
    IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX,0x10b0);   /*设置电气属性*/
    /*
     * bit 16:0 HYS关闭
     * bit [15:14]: 00 默认100K下拉
     * bit [13]: 0 keeper功能
     * bit [12]: 1 pull/keeper使能
     * bit [11]: 0 关闭开路输出
     * bit [7:6]: 10 速度100Mhz
     * bit [5:3]: 110 驱动能力R0/6
     * bit [0]: 0 低转换率
 	*/
}

/*
 * @description : 发送一个字符，一次性发送一个字节
 * @param - c	: 要发送的字符
 * @return		: 无
*/
void putc(uint8_t c)
{
    while (((UART1->USR2 >>3) & 0x01)!=1);    /*等待发送完成标志位为1*/
    UART1->UTXD = c;

}

/*
 * @description : 发送一个字符串
 * @param - str	: 要发送的字符串
 * @return		: 无
*/
void puts(int8_t *str)
{
    int8_t *p=str;
    while(*p)    /*为有效字符串才发送，0对应空字符位，为字符串结束标志位*/
    {
        putc(*p++);     /* 取数组当前位置的值*p,然后p指向下一位置的数据*/ 
    }
}

/*
 * @description : 接收一个字符
 * @param 		: 无
 * @return		: 接收到的字符
*/
uint8_t getc(void)
{
    while (((UART1->USR2 >>0) & 0x01)!=1);    /*等待接收数据就绪标志位为1*/
    return UART1->URXD;
}

/*
 * @description : 防止编译器报错
 * @param 		: 无
 * @return		: 无
 */
void raise(int sig_nr) 
{

}