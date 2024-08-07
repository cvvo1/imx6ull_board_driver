/*
PLL1设置
①、要设置ARM内核主频为528MHz，设置CACRR寄存器的ARM_PODF位为2分频，然后设置PLL1=1056MHz即可。
    ACRR的bit3~0为ARM_PODF位，可设置0~7，分别对应1~8分频。应该设置CACRR寄存器的ARM_PODF=1。
②、设置PLL1=1056MHz。PLL1=pll1_sw_clk。pll1_sw_clk有两路可以选择，分别为pll1_main_clk，和step_clk，
    通过CCSR寄存器的pll1_sw_clk_sel位(bit2)来选择。为0的时候选择pll1_main_clk，为1的时候选额step_clk。
③、在修改PLL1的时候，也就是设置系统时钟的时候需要给6ULL一个临时的时钟，也就是step_clk。在修改PLL1的时候需要将pll1_sw_clk切换到step_clk上。
③、设置step_clk。Step_clk也有两路来源，由CCSR的step_sel位(bit8)来设置，为0的时候设置step_clk为osc=24MHz。为1的时候不重要，不用。
④、时钟切换成功以后就可以修改PLL1的值。
⑤、通过CCM_ANALOG_PLL_ARM寄存器的DIV_SELECT位(bit6~0)来设置PLL1的频率，公式为：
    Output = fref*DIV_SEL/2  1056=24*DIV_SEL/2=>DIV_SEL=88。设置CCM_ANALOG_PLL_ARM寄存器的DIV_SELECT位=88即可。PLL1=1056MHz
    还要设置CCM_ANALOG_PLL_ARM寄存器的ENABLE位(bit13)为1，也就是使能输出。
⑥、在切换回PLL1之前，设置CACRR寄存器的ARM_PODF=1！！切记。


PLL2固定为528 MHz，PLL3固定为480 MHz
1、初始化PLL2_PFD0~PLL_PFD3
    寄存器CCM_ANALOG_PFD_528用于设置4路PFD的时钟。比如PFD0=528*18/PFD0_FRAC。设置PFD0_FRAC位即可，规定12～35范围。
    比如PLL2_PFD0=352 MHz=528*18/PFD0_FRAC，因此FPD0_FRAC=27。
2、初始化PLL3_PFD0~PLL_PFD3
    寄存器CCM_ANALOG_PFD_480用于设置4路PFD的时钟

其他外设时钟源配置
1、AHB_CLK_ROOT、PERCLK_CLK_ROOT以及IPG_CLK_ROOT（后两个需要用到AHB_CLK_ROOT）
  （1）初始化AHB_CLK_ROOT=132 MHz
    设置CBCMR寄存器的PRE_PERIPH_CLK_SEL位19\18为01，设置CBCDR寄存器的PERIPH_CLK_SEL位25为0，设置输入为PLL2_PFD2(396 Hz)。
    设置CBCDR寄存器的AHB_PODF位为2，也就是3分频，因此396/3=132MHz。
  （2）初始化PERCLK_CLK_ROOT=66 MHz
    设置CBCDR寄存器IPG_PODF位9/8为01，也就是2分频。
  （3）初始化IPG_CLK_ROOT=66 MHz
    设置CSCMR1寄存器的PERCLK_CLK_SEL位6为0，表示PERCLK的时钟源为IPG。

*/
#include "bsp_clk.h"
/*使能IMX6U所有外设时钟*/
void clk_enable(void)
{
    CCM->CCGR0=0xFFFFFFFF;
    CCM->CCGR1=0xFFFFFFFF;
    CCM->CCGR2=0xFFFFFFFF;
    CCM->CCGR3=0xFFFFFFFF;
    CCM->CCGR4=0xFFFFFFFF;
    CCM->CCGR5=0xFFFFFFFF;
    CCM->CCGR6=0xFFFFFFFF;
}

/*初始化时钟，初始化imx6u的主频为528Hz*/
void imx6u_clkinit(void)
{
    uint32_t reg=0;
    /*判断pll1_sw_clk_sel(bit位2)，PLL1时钟来源，是否已切换*/
    if(((CCM->CCSR>>2)&0x1)==0)     /*获取bit2值，==0为当前时钟使用pll1_main_clk，需切换时钟*/
    {
        CCM->CCSR &= ~(1<<8);       /*bit8置0，设置tep_clk为osc=24MHz*/
        CCM->CCSR |= (1<<2);        /*bit2置1，设置当前时钟使用step_clk=24 MHz，进行时钟切换*/
    }

    /*设置PLL1=1056 MHz，设置CCM_ANALOG_PLL_ARM寄存器的DIV_SELECT位=88*/
    CCM_ANALOG->PLL_ARM = ((88<<0)&0x7f);
    CCM_ANALOG->PLL_ARM |=(1<<13);    /*bit13置1(ENABLE)，使能时钟输出*/

    /*设置二分频，设置CACRR寄存器的ARM_PODF=1*/
    CCM->CACRR = 1;
    CCM->CCSR &= ~(1<<2);      /*bit2置0，设置当前时钟使用pll1_main_clk=1056 MHz*/

    /*设置PLL2的4路PFD*/  
    reg=CCM_ANALOG->PFD_528;    /*读取CCM_ANALOG_PFD_528寄存器值*/
    reg &= ~(0x3f3f3f3f);       /*保留其他无关位，需要修改位，置0（原0x3f3f3f3f）*/
    reg |=(27<<0);              /*PLL2_PFD0  352MHz，PFD0=528*18/PFD0_FRAC，FPD0_FRAC=27*/
    reg |=(16<<8);              /*PLL2_PFD1  594MHz，PFD1=528*18/PFD1_FRAC，FPD1_FRAC=16*/
    reg |=(24<<16);             /*PLL2_PFD2  396MHz，PFD2=528*18/PFD2_FRAC，FPD2_FRAC=24*/
    reg |=(32<<24);             /*PLL2_PFD3  297MHz，PFD3=528*18/PFD3_FRAC，FPD3_FRAC=32*/
    CCM_ANALOG->PFD_528=reg;    /*设置PLL2_PFD0~3*/

    /*设置PLL3的4路PFD*/  
    reg=0;                      /*清零*/
    reg=CCM_ANALOG->PFD_480;    /*读取CCM_ANALOG_PFD_480寄存器值*/
    reg &= ~(0x3f3f3f3f);       /*保留其他无关位，需要修改位，置0（原0x3f3f3f3f）*/
    reg |=(12<<0);              /*PLL3_PFD0  720MHz，PFD0=480*18/PFD0_FRAC，FPD0_FRAC=12*/
    reg |=(16<<8);              /*PLL3_PFD1  540MHz，PFD1=480*18/PFD1_FRAC，FPD1_FRAC=16*/
    reg |=(17<<16);             /*PLL3_PFD2  508.2MHz，PFD2=480*18/PFD2_FRAC，FPD2_FRAC=17*/
    reg |=(19<<24);             /*PLL3_PFD3  454.7MHz，PFD3=480*18/PFD3_FRAC，FPD3_FRAC=19*/
    CCM_ANALOG->PFD_480=reg;    /*设置PLL3_PFD0~3*/

    /*设置HB_CLK_ROOT=132 MHz*/
    CCM->CBCMR &=~(3<<18);       /*先对寄存器18、19位置0,二进制11-十进制3*/
    CCM->CBCMR |=(1<<18);        /*设置CBCMR寄存器的PRE_PERIPH_CLK_SEL位为01*/
    CCM->CBCDR &=~(1<<25);       /*设置CBCDR寄存器的PERIPH_CLK_SEL位25为0*/
    while (CCM->CDHIPR & (1 << 5));  /*设置ERIPH_CLK_SEL位变化后，需要查询CDHIPR寄存器是否busy*/

    reg=0;
    reg=CCM->CBCDR;
    reg&=~(7<<10);       /*先对寄存器12-10位置0,二进制111-十进制7*/
    reg|=(2<<10);        /*设置CBCDR寄存器的AHB_PODF位12-10为2(010)，也就是3分频，因此396/3=132MHz*/
    CCM->CBCDR=reg;
    while (CCM->CDHIPR & (1 << 1));    /*设置AHB_PODF位分频器变化后，需要查询CDHIPR寄存器是否busy*/

    /*设置IPGCLK_ROOT=66 MHz*/
    CCM->CBCDR&=~(3<<8);  /*CBCDR寄存器IPG_PODF位9/8置0*/
    CCM->CBCDR|=(1<<8);   /*设置CBCDR寄存器IPG_PODF位9/8为1(01)，二分频，132/2=66 MHz*/
    
    /*设置PERCLK_CLK_ROOT=66 MHz*/
    CCM->CSCMR1&=~(1<<6);     /*设置CSCMR1寄存器的PERCLK_CLK_SEL位6为0，表示PERCLK的时钟源为IPG，默认不分频*/
    CCM->CSCMR1&=~(0x3f<<0);  /*PERCLK_POOF位[0-5]为0，不分频，二进制111111-十六进制3f*/
}


