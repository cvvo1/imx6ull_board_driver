#include "bsp_backlight.h"
#include "bsp_int.h"

/* PWM简介
 *  1、6ULL的PWM是16位向上计数器，
 *  2、有4个16位的FIFO。
 *  3、一个12位的分频器
 * 
 * PWM频率：PWM计数器从0X0000(16bit)开始计数，当计数器的值等于PWMPR(Period Register 16位周期计数器)+1的时候定时器就会重新开始下一个周期的运行，
 *      因此PWMPR寄存器控制着PWM频率，PWM频率：PWMO(Hz)=PCLK(Hz)/(period+2)——PCLK为分频后时钟源频率
 *      PWM_PWMPR中的零值将导致输出信号的两个时钟周期的周期。将0xFFFF写入该寄存器将获得与写入0xFFFE相同的结果。
 * 
 * PWM占空比：FIFO保存着采样值，当我们向PWMSAR(Sample Register)16位采样寄存器(类似捕获比较寄存器值CCR)写采样值的时候会写到FIFO里面，每当读取一次PWMSAR寄存器
 *      (FIFO可以随时写入，但只能在启用PWM时读取)，或者每产生一个PWM信号，FIFO的数据都会减一。直到FIFO为空，那么就无法再产生PWM信号。
 *      FIFO为空的时候会产生中断，在中断中向FIFO写入采样数据，即向PWMSAR写数据。
 * 	PWM占空比：Duty=PWMSAR/(PWMPR+2)
 *  PWM分辨率：Reso=1/(PWMPR+2)
 * 
 * 引脚复用
 * BLT_PWM——GPIO1_io8
 * 
 * LCD背光实验步骤
 * 1、配置引脚GPIO1_IO8。配置GPIO1_IO08的复用功能，将其复用为PWM1_OUT信号线。
 * 2、初始化PWM1。初始化PWM1，配置所需的 PWM 信号的频率和默认占空比。
 *  PWM1_PWMCR寄存器：控制寄存器
 *      FWM(bit27:26)： FIFO水位线，用来设置FIFO空余位置为多少的时候表示FIFO为空。
 *                      设置为0(00)的时候表示FIFO空余位置大于等于1的时候FIFO为空；
 *                      设置为1(01)的时候表示FIFO空余位置大于等于2的时候FIFO为空；
 *                      设置为2(10)的时候表示FIFO空余位置大于等于3的时候FIFO为空；
 *                      设置为3(11)的时候表示FIFO空余位置大于等于4的时候FIFO为空。
 *      STOPEN(bit25)：设置停止模式下PWM是否工作，此位不受软件重置的影响。它通过硬件重置来清除，为0的话表示停止模式下关闭PWM，为1的话表示在停止模式下PWM继续工作
 *      DOZEN(bit24)：设置休眠模式下PWM是否工作，此位不受软件重置的影响。它通过硬件重置来清除，为0的话表示休眠模式下关闭PWM，为1的话表示在休眠模式下PWM继续工作
 *      WAITEN(bit23)：设置等待模式下PWM是否工作，此位不受软件重置的影响。它通过硬件重置来清除，为0的话表示等待模式下关闭PWM，为1的话表示在等待模式下PWM继续工作
 *      DBGEN(bit22)： 设置调试模式下PWM是否工作，此位不受软件重置的影响。它通过硬件重置来清除，为0的话表示调试模式下关闭PWM，为1的话表示在调试模式下PWM继续工作
 *      BCTR(bit21)：字节(Byte-8位)交换控制位，用来控制16位的数据进入FIFO的字节顺序。0字节排序保持不变，1字节排序颠倒。
 *      HCRT(bit20)：半字(Half-16位)交换控制位，用来决定从32位IP总线接口传输来的哪个半字数据写入采样寄存器的低16位中，0未进行半字交换，1来自写入数据总线的半字被交换
 *      POUTC(bit19:18)：PWM输出控制控制位，用来设置PWM输出模式，为0的时候表示PWM先输出高电平，当计数器值和采样值相等的话就输出低电平。
 *                      为1的时候相反，当为2或者3的时候PWM信号不输出。设置为0，比较时改为低电平，这样采样值越大高电平时间就越长，占空比就越大。
 *      CLKSRC(bit17:16)：PWM时钟源选择，为0的话关闭；为1的话选择ipg_clk(66 MHz)为时钟源；为2的话选择ipg_clk_highfreq为时钟源；为3的话选择ipg_clk_32k为时钟源。
 *      PRESCALER(bit15:4)：分频值，可设置为0~4095，对应着1~4096分频。
 *      SWR(bit3)：软件复位，向此位写1就复位PWM，此位是自清零的，当复位完成以后此位会自动清零。
 *      REPEAT(bit2:1)：重复采样设置，此位用来设置FIFO中的每个数据能用几次。可设置 0~3，分别表示FIFO中的每个数据能用1~4 次，设置为0，即FIFO中的每个数据只能用一次。
 *      EN(bit0)：PWM使能位，为1的时候使能PWM，为0的时候关闭PWM
 *  PWM1_PWMIR寄存器：中断控制寄存器
 *      CIE(bit2)：比较中断使能位，为1的时候使能比较中断，为0的时候关闭比较中断。
 *      RIE(bit1)：翻转中断使能位，当计数器值等于采样值并回滚到0X0000的时候就会产生此中断，为1的时候使能翻转中断，为0的时候关闭翻转中断。
 *      FIE(bit0)：FIFO空中断，为1的时候使能，为0的时候关闭。
 *  PWM1_PWMSR寄存器：状态寄存器
 *      FWE(bit6)：FIFO写错误事件，为1的时候表示发生了FIFO写错误
 *      CMP(bit5)：FIFO比较事件发生标志位，为1的时候表示发生FIFO比较事件
 *      ROV(bit4)：翻转事件标志位，为1的话表示翻转事件发生
 *      FE(bit3)： FIFO 空标志位，此位表示与控制寄存器中FWM字段设置的水位相比的FIFO数据电平，为1的时候表示FIFO位空
 *      FIFOAV(bit2:1)：此位记录FIFO中的有效数据个数，有效值为 0~4(4x16 data FIFO)，分别表示FIFO中有0~4个有效数据
 * 
 * 3、设置中断。因为FIFO中的采样值每个周期都会少一个，所以需要不断的向FIFO中写入采样值，防止其为空。
 *      我们可以使能 FIFO空中断，这样当FIFO为空的时候就会触发相应的中断，然后在中断处理函数中向FIFO写入采样值。
 * 4、使能PWM1。配置好PWM1以后就可以开启了
 * 
*/ 

struct backlight_dev_struc backlight_dev;

void backlight_Init(void)
{
    uint8_t i=0;
    /*1、IO初始化*/
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO08_PWM1_OUT,0); 
	/* 配置PWM IO属性	
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 10 100K上拉
	 *bit [13]: 1 pull功能
	 *bit [12]: 1 pull/keeper使能
	 *bit [11]: 0 关闭开路输出
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 010 驱动能力为R0/2
	 *bit [0]: 1 高转换率
	 */
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO08_PWM1_OUT,0xB091);   /*设置电气属性*/

    /*2、初始化PWM1*/
    PWM_softRestart();/*软件复位PWM1*/

    /*FIFO空余位置大于等于2的时候FIFO为空；选择ipg_clk(66 MHz)为时钟源；为66分频，PWM时钟源为1 MHz*/
    PWM1->PWMCR|=(1<<26)|(1<<16)|(65<<4);

    /*设置PWM周期*/
    PWM_setPeriod(1000);      /*PWM频率为1MHz/1000=1kHz*/
    
    /* 设置占空比，默认50%占空比   ,写四次是因为有4个FIFO */
	backlight_dev.pwm_duty = 50;
	for(i = 0; i < 4; i++)
	{
		PWM_setDuty(backlight_dev.pwm_duty);	
	}
	
    /*使能FIFO空中断*/
    PWM1->PWMIR|=(1<<0);
  
    /* 注册中断服务函数 */
    system_register_irqhandler(PWM1_IRQn,(system_irq_handler_t)PWM1_irqhandle,NULL);  

    GIC_EnableIRQ(PWM1_IRQn);       /*使能GIC对应中断，GPIO1-IO18对应的中断ID为67+32=99*/

    PWM_enable();

}

void PWM1_irqhandle(unsigned int giccIar, void *param)
{
    if((PWM1->PWMSR>>0X3)&0X1)   /*判断一次为空中断，FIFO 空标志位，*/
    {
        PWM_setDuty(backlight_dev.pwm_duty);
        PWM1->PWMSR |=(1<<3);    /*清除中断标志位*/
    }
}


/*软件复位PWM*/
void PWM_softRestart()
{
    PWM1->PWMCR|=(1<<3);
    while((PWM1->PWMCR>>0x3)&0x1);   /*当复位完成以后此位会自动清零*/
}

/*
 * @description	: 使能PWM
 * @param		: 要使用的PWM
 * @return 		: 无
*/
void PWM_enable()
{
    PWM1->PWMCR|=(1<<0);
}

/*
 * @description		: 设置Sample寄存器，Sample数据会写入到FIFO中，
 * 					  所谓的Sample寄存器，就相当于比较寄存器，假如PWMCR中的POUTC
 *				  	  设置为00的时候。当PWM计数器中的计数值小于Sample的时候
 *					  就会输出高电平，当PWM计数器值大于Sample的时候输出底电平,
 *					  因此可以通过设置Sample寄存器来设置占空比
 * @param		: 要使用的PWM
 * @param -  value	: 寄存器值，范围0~0XFFFF
 * @return 			: 无
*/
void PWM_setSample(uint32_t value)
{
    PWM1->PWMSAR=(value&0xffff);    /*所有配置寄存器都可以通过32位访问总线接口进行访问*/
}

/*
 * @description		: 设置PWM周期，就是设置寄存器PWMPR，PWM周期公式如下
 *					  PWM_FRE = PWM_CLK / (PERIOD + 2)， 比如当前PWM_CLK=1MHz
 *					  要产生1KHz的PWM，那么实际PERIOD=1000-2 = 998
 * @param		: 要使用的PWM
 * @param -  value	: 周期值，范围0~0XFFFF
 * @return 			: 无
*/
void PWM_setPeriod(uint32_t value)
{
    uint32_t value_act=0;
    if(value<2)
    {
        value_act=2;
    }else
    {
        value_act=value-2;
    }
    PWM1->PWMPR=(value_act&0xffff);
}

/*
 * @description		: 设置PWM占空比
 * @param		: 要使用的PWM
 * @param -  value	: 占空比0~100，对应0%~100%
 * @return 			: 无
*/
void PWM_setDuty(uint8_t value)
{
    uint16_t period;
    uint16_t sample;
    backlight_dev.pwm_duty=value;
    period=PWM1->PWMPR+2;
    sample=period*backlight_dev.pwm_duty/100.0;
    PWM_setSample(sample);
}