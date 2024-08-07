#include "bsp_adc.h"
#include "bsp_delay.h"

/* ADC简介
 * ADC，Analog to Digital Converter 的缩写，中文名称模数转换器。它可以将外部的模拟信号转化成数字信号。
 * ADC 有几个比较重要的参数：
 *      测量范围：测量范围对于ADC来说就好比尺子的量程，ADC测量范围决定了你外接的设备其信号输出电压范围，不能超过ADC的测量范围
 *      分辨率：就是尺子上的能量出来的最小测量刻度，假如 ADC 的测量范围为0-5V，分辨率设置为12位，那么我们能测出来的最小电压就是5V除以2的12次方，
 *            也就是5/4096=0.00122V很明显，分辨率越高，采集到的信号越精确，所以分辨率是衡量 ADC 的一个重要指标。
 *      精度：是影响结果准确度的因素之一，经过计算我们ADC在12位分辨率下的最小测量值是 0.00122V但是我们ADC的精度最高只能到11位也就是0.00244V
 *      采样时间：当ADC在某时刻采集外部电压信号的时候，此时外部的信号应该保持不变，但实际上外部的信号是不停变化的。所以在ADC内部有一个保持电路，
 *             保持某一时刻的外部信号，这样 ADC 就可以稳定采集了，保持这个信号的时间就是采样时间。
 *      采样率：也就是在一秒的时间内采集多少次。很明显，采样率越高越好，当采样率不够的时候可能会丢失部分信息，所以ADC采样率是衡量ADC性能的另一个重要指标
 * 
 * I.MX6ULL 提供了两个12位ADC通道(ADC1和ADC2)和各10个输入接口
 *
 * 步骤
 * 1、初始化ADC1_CH1。初始化ADC1_CH1，配置ADC位数，时钟源，采样时间等。
 * ADC1和ADC2(ADC_5H)的寄存器有所不同，只使用ADC1
 *  ADCx_CFG寄存器：配置寄存器
 *      OVWREN (bit16)：数据覆盖现有(以前)未读数据上使能位，为1的时候使能覆盖功能，为0禁用覆盖，数据结果寄存器中的现有数据不会被后续转换的数据覆盖。
 *      AVGS(bit15:14)：硬件平均值选择确定平均多少ADC转换，当ADC_GC[AVGE]=1时，此功能被激活00：4个样本平均，01：8个样本平均，10：16个样本平均，11：32个样本平均
 *      ADTRG(bit13)：转换触发选择。为0的时候选择软件触发，为1的时候，不选择软件触发。
 *      REFSEL(bit12:11)：参考电压选择，为00时选择VREFH/VREFL这两个引脚上的电压为参考电压，正点原子ALPHA开发板上VREFH为3.3V， VREFL为0V。
 *      ADHSC(bit10)：高速转换使能位，当为0时为正常模式，为1时为高速模式。
 *      ADSTS(bit9:8)：设置ADC的采样周期(采样持续时间)，当选择长采样时间（ADLSMP=1）时，否则适用于短采样。如果不需要高转换率，则当启用连续转换时，也可以使用更长的采样时间来降低总体功耗。
 *                     00：如果ADLSMP=0b，则采样周期（ADC时钟）=2；如果ADLSMP=1b，则采样周期（ADC时钟）=12
                       01：如果ADLSMP=0b，则采样周期（ADC时钟）=4；如果ADLSMP=1b，则采样周期（ADC时钟）=16
                       10：如果ADLSMP=0b，则采样周期（ADC时钟）=6；如果ADLSMP=1b，则采样周期（ADC时钟）=20
                       11：如果ADLSMP=0b，则采样周期（ADC时钟）=8；如果ADLSMP=1b，则采样周期（ADC时钟）=24
 *      ADLPC(bit7)：低功率配置使ADC硬块进入低功率模式，0：ADC硬块未处于低功率模式，1：ADC硬块处于低功率模式
 *      ADIV(bit6:5)：时钟分频选择，为00：不分频，01：2分频，10：4分频，11：8分频。
 *      ADLSMP(bit4)：长采样周期使能位，当值为0时为短采样周期模式，为1时为长采样周期模式，搭配ADSTS位一起控制ADC的采样周期，
 *      MODE(bit3:2)：转换模式选择用于设置ADC分辨率模式。00：8位转换，01：10位转换，10：12位转换，11：保留
 *      ADICLK(bit1:0)：输入时钟源选择，00：IPG Clock(66 MHz)，01:IPGClock/2，10：保留，11：ADACK(40 MHz)。设置为11，也就是选择ADACK为ADC的时钟源。
 *  ADCx_GC寄存器：通用控制寄存器
 *      CAL(bit7)：入1时，硬件校准功能将会启动(将会中止任何当前转换)，校准过程中该位会一直保持1，校准完成后会清0，校准完成后需要检查一下ADC_GS[CALF]位，确认校准结果。
 *      ADCO(bit6)：连续转换使能位，只有在开启了硬件平均功能时有效(AVGE=1)，为0时只能转换一次或一组，当ADCO为1时可以连续转换或多组。
 *      AVGE(bit5)：硬件平均启用启用ADC的硬件平均功能。0禁用硬件平均功能，1启用硬件平均功能
 *      ACFE(bit4)：比较功能使能位。为0时关闭，为1时使能。
 *      ACFGT(bit3)：配置比较方法，如果为0的话就比较转换结果是否小于ADC_CV寄存器值，如果为1的话就比较装换结果是否大于或等于ADC_CV寄存器值。
 *      ACREN(bit2)：范围比较功能使能位。为0的话仅和ADC_CV里的CV1比较，为1的话和ADC_CV里的CV1、CV2比较。
 *      DMAEN(bit1)：DMA功能使能位，为0是关闭，为1是开启。
 *      ADACKEN(bit0)：启用ADC的异步时钟源和时钟源输出，而不管ADC的转换和输入时钟选择(DC_CFG[ADICLK])设置如何。
 *                     0：异步时钟输出被禁用；异步时钟仅在ADICLK选择并且转换处于活动状态时启用。
 *                     1：无论ADC的状态如何，都启用异步时钟和时钟输出 
 *  ADCx_GS寄存器：通用状态寄存器
 *      AWKST(bit2)：异步唤醒中断状态，为1时表示发生了异步唤醒中断。为0时没有发生异步中断。
 *      CALF(bit1)：校准失败标志位，为0的时候表示校准正常完成，为1的时候表示校准失败，写1清零
 *      ADACT(bit0)：转换活动标志，为0的时候表示转换没有进行，为1的时候表示正在进行转换。
 *  ADCx_HS寄存器：状态寄存器
 *      COCO0(bit0)：转换完成标志位，此位为只读位，当关闭比较功能和硬件平均以后每次转换完成此位就会被置1。
 *                  使能硬件平均以后，只有在设置的转换次数达到以后此位才置1。COCO0标志也将在校准和测试序列完成时设置
 *  ADCx_HC0寄存器：控制寄存器
 *      AIEN(bit7)：转换完成中断控制位，为1的时候打开转换完成中断，为0的时候关闭。
 *      ADCH(bit4:0)：转换通道选择，可以设置为00000~01111分别对应通道 0~15。11001 为内部通道，用于ADC自测。
 *  ADCx_R0寄存器：数据结果寄存器，bit11:0 这12位有效，
 * 
 * 2、校准ADC。ADC 在使用之前需要校准一次。
 * 3、使能ADC。配置好 ADC 以后就可以开启了。
 * 4、读取ADC值。ADC 正常工作以后就可以读取 ADC 值。
 */


/*
 * @description	: 初始化ADC1_CH1，使用GPIO1_IO01这个引脚。
 * @param		: 无
 * @return 		: 0 成功，其他值 错误代码
 */
uint8_t adc1ch1_Init(void)
{
    /*GPIO无效，引脚直接接入内部adc*/
    ADC1->CFG=0;   /*先清零*/
    /*高速模式；采样周期4/16；12位转换；选择ADACK为ADC的时钟源*/
    ADC1->CFG|=(1<<10)|(1<<8)|(2<<2)|(3 << 0);

    ADC1->GC=0;   /*先清零*/
    /*启用连续转换；启用硬件平均；使能ADACK*/  /*无法开启硬件平均，启用硬件平均没有除以平均数*/
    // ADC1->GC|=(1<<6)|(1<<5)|(1<<0);
    ADC1->GC |= (1<<0);

    /*校准ADC*/
    if(adc1ch1_calibration()!=kStatus_Success)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;

}

/*
 * @description	: 初始化ADC1校准
 * @param		: 无
 * @return 		: kStatus_Success 成功，kStatus_Fail 失败
*/
uint8_t adc1ch1_calibration(void)
{
    uint8_t ret=0;
    ADC1->GS|=(1<<1);     /* 清除CALF位，写1清零 */
    ADC1->GC|=(1<<7);      /* 使能校准功能 */

    /* 校准完成之前GC寄存器的CAL位会一直为1，直到校准完成此位自动清零 */
    while((ADC1->GC>>7)&0x1)
    {
        /*如果GS寄存器的CALF位为1的话表示校准失败*/
        if((ADC1->GS>>1)&0x1)
        {
            ret=kStatus_Fail;
            break;
        }
    }

    if((ADC1->HS&0x1)==0)
    {
        ret=kStatus_Fail;
    }

    /*如果GS寄存器的CALF位为1的话表示校准失败*/
    if((ADC1->GS>>1)&0x1)
    {
        ret=kStatus_Fail;
    }
    return ret;
}

/*
 * @description	: 获取ADC原始值
 * @param		: 无
 * @return 		: 获取到的ADC原始值
*/
uint32_t getadc_value(void)
{
    uint32_t temp=0;
    /* 配置ADC通道1 */
    ADC1->HC[0] = 0;            /* 关闭转换结束中断    */
    ADC1->HC[0] |= (1 << 0);    /* 选择转换通道1    */
    while((ADC1->HS&0x1)!=1);

    temp=ADC1->R[0];
 
    return temp;     /* 返回ADC值 */
}

/*
 * @description	        : 获取ADC平均值
 * @param		times   : 获取次数
 * @return 		        : times次转换结果平均值
*/
uint32_t getadc_avg(uint8_t times)
{
    uint32_t temp_val=0,temp=0;
    uint8_t i=0;
    for(i=0;i<times;i++)
    {
        temp_val+=getadc_value();
        delay_ms(5);
    }
    temp=temp_val/times;
    return temp;
}

/*
 * @description : 获取ADC对应的电压值
 * @param	    : 无
 * @return 	    : 获取到的电压值，单位为mV
*/
uint32_t getadc_volt(void)
{
    uint32_t adcvalue=0,temp=0;
    adcvalue=getadc_avg(10);
    temp = (float)adcvalue *(3300.0f/4095.0f);    	/* 获取计算后的带小数的实际电压值 */
    return  temp;
}