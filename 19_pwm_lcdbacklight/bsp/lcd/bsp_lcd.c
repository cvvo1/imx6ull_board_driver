#include "bsp_lcd.h"
#include "bsp_gpio.h"
#include "bsp_delay.h"
#include "stdio.h"

/* 使用6ULL LDCIF控制器DOTCLK接口
 * 1、初始化LCD所使用的IO。首先肯定是初始化LCD所示使用的IO，将其复用为eLCDIF接口IO。
 * 2、设置LCD的像素时钟查阅所使用的LCD屏幕数据手册，或者自己计算出的时钟像素，然后设置CCM相应的寄存器，开启LCDIF1_CLK_ROOT。
 *  （1）先设置PLL5(Video PLL)为lcd的时钟源，PLL5 output frequency = Fref*(DIV_SELECT+NUM/DENOM)
 *      不考虑小数分频，即PLL_VIDEO_NUM和PLL_VIDEO_NUM寄存器都置0
 *      CCM_ANALOG_PLL_VIDEOn寄存器：
 *          POWERDOWN(bit12)，为1时关闭PLL电源，设置为0
 *          ENABLE(bit13)，使能PLL_VIDEO输出，设置为1
 *          BYPASS(bit16)，为1时绕过PLL，设置为0
 *          POST_DIV_SELECT(bit20:19)，设置PLL5预分频，1分频(10)，设置为0x2
 *          DIV_SELECT(bit6:0)，值范围为27～54，根据需要选择
 *      CCM_ANALOG_MISC2n寄存器：
 *          VIDEO_DIV(bit31:30)，设置PLL5后分频，1分频(00)，设置为0
 *  （2）设置后续时钟选择
 *      CCM_CSCDR2寄存器：
 *          LCDIF1_PRE_CLK_SEL(bit17:15)，lcdif1根时钟预复用器的选择器，设置时钟源为PLL5(010)，设置为2
 *          LCDIF1_PRED(bit14:12)，lcdif1时钟的预分频置，000-1分频～111-8分频
 *      CCM_CBCMR寄存器：
 *          LCDIF1_PODF(bit25:23)，LCDIF1时钟的后分频器，000-1分频～111-8分频
 *      CCM_CSCDR2寄存器：
 *          LCDIF1_CLK_SEL(bit11:9)，LCDIF1根时钟多路复用器的选择器，设置LCD时钟源为LCDIF_PRE时钟，设置为000
 *     
 * 3、配置eLCDIF接口设置LCDIF的寄存器CTRL、CTRL1、TRANSFER_COUNT、VDCTRL0~4、CUR_BUF和NEXT_BUF。根据LCD的数据手册设置相应的参数。
 *  LCDIF_CTRL寄存器：
 *      RUN(bit0)必须置1，eLCDIF将开始在SoC和显示器之间传输数据。此位必须保持设置，直到操作完成。
 *      DATA_FORMAT_24_BIT(bit1)只有当WORD_LENGTH为3的时候此位才有效，为0的时候表示全部的24位数据都有效。为1的话实际输入的数据有效位只有18位，
 *       虽然输入的是24位数据，但是每个颜色通道的高2位数据会被丢弃掉，设置为0。
 *      MASTER(Bit5)设置LCDIF接口工作在主机模式下，要置1。
 *      WORD_LENGTH(Bit9:8)设置输入像素格式位24bit，写0x3。
 *      LCD_DATABUS_WIDTH(Bit11：10)，设置数据传输宽度位24bit，写0x3。
 *      CSC_DATA_SWIZZLE(Bit13：12)设置数据交换，我们不需要交换，置位0。
 *      INPUT_DATA_SWIZZLE(Bit15:14)设置输入数据交换，不交换，置位0。
 *      DOTCLK_MODE(Bit17)LCDIF工作在DOTCLK模式下，置1。
 *      VSYNC_MODE(bit18)：此位为1的话LCDIF工作在VSYNC接口模式。
 *      BYPASS_COUNT(Bit19)工作在DOTCL模式,必须置1。
 *      CLKGATE(bit30)：如果此位为1的话时钟就不会进入到LCDIF,正常运行模式下，此位必须，置为0。
 *      SFTRST(Bit31)此位设置为1时，强制块级别重置，复位功能。能使eLCDIF正常工作，必须置0，
 *  LCDIF_CTRL1寄存器：
 *      BYTE_PACKING_FORMAT(bit19:16)，此位用来决定在32位的数据中哪些字节的数据有效，默认值为0XF，也就是所有的字节有效，当为 0 的话表示所有的字节都无效。
 *       如果显示的数据是24位(ARGB 格式，但是A通道不传输)的话就，设置此位为0X7。
 *  LCDIF_TRANSFER_COUNT寄存器：
 *      H_COUNT(bit15：0)是LCD一行的像素数，1024。V_COUNT(Bit31:16)是LCD一共有多少行，即垂直像素，600行。
 *  LCDIF_VDCTRL0寄存器：
 *      VSYNC_PULSE_WIDTH(bit17:0)置VSYNC信号宽度，即信号持续时间，为VSPW参数
 *      VSYNC_PULSE_WIDTH_UNIT(Bit20)设置VSYNC信号脉冲宽度单位，和VSYNC_PERIOD_UNUT一样，如果使用DOTCLK模式的话，设置为1
 *      VSYNC_PERIOD_UNIT(Bit21)VSYNC信号周期单位，为0的话VSYNC周期单位为像素时钟。为1的话VSYNC周期单位是水平行，如果使用DOTCLK模式的话，设置为1
 *      ENABLE_POL(Bit24)设置ENABLE信号极性，为0的时候是低电平有效，为1是高电平，设置为1
 *      DOTCLK_POL(Bit25)设置CLK信号极性，为0的话下降沿锁存数据，上升沿捕获数据，为1的话相反，设置为0
 *      HSYNC_POL(bit26)设置HSYNC信号极性，低电平有效，为0的话HSYNC低电平有效，为1的话HSYNC高电平有效，设置0
 *      VSYNC_POL(bit27)设置VSYNC信号极性，低电平有效，为0的话VSYNC低电平有效，为1的话VSYNC高电平有效，设置为0
 *      ENABLE_PRESENT(Bit28)EBABLE数据线使能位，即DE数据线，设置1
 *      VSYNC_OEB(Bit29)VSYNC信号方向控制位，为0的话VSYNC是输出，为1的话VSYNC是输入，设置为0
 *  LCDIF_VDCTRL1寄存器：
 *      为VSYNC和DOTCLK模式控制寄存器1，设置VSYNC总周期，就是：VSPW+VBP+屏幕高度+VFP
 *  LCDIF_VDCTRL2寄存器：
 *      HSYNC_PULSE_WIDTH(bit31~18)高18位，设置HSYNC信号宽度，即信号持续时间，也就是HSPW
 *      HSYNC_PERIOD(bit31~18)低18位，设置HSYNC总周期，就是：屏幕宽度+HSPW+HBP+HFP
 *  LCDIF_VDCTRL3寄存器：
 *      HORIZONTAL_WAIT_CNT(bit27:16)：此位用于DOTCLK模式，用于设置HSYNC信号产生到有效数据产生之间的时间，也就是HSPW+HBP
 *      VERTICAL_WAIR_CNT(bit15:0)：和HORIZONTAL_WAIT_CNT一样，只是此位用于VSYNC信号，也就是VSPW+VBP
 *  LCDIF_VDCTRL4寄存器：
 *      SYNC_SIGNALS_ON(bit18)：同步信号使能位，使能VSYNC、HSYNC、DOTCLK这些信号，设置为1
 *      DOTCLK_H_VALID_DATA_CNT(bit15:0)：设置 LCD 的宽度，也就是水平像素数量。
 *  LCDIF_CUR_BUF寄存器和LCDIF_NEXT_BUF寄存器，这两个寄存器分别为当前帧和下一帧缓冲区，也就是LCD显存。一般这两个寄存器保存同一个地址，即划分给LCD的显存首地址。
 * 
 * 4、编写API 数驱动LCD屏幕的目的就是显示内容，所以需要编写一些基本的API函数，比如画点、画线、画圆函数，字符串显示函数等。
 * 
*/


/* 液晶屏参数结构体 */
struct CRlcd_typedef CRlcd_dev;

/*lcd初始化*/
void lcd_Init(void)
{
    uint16_t lcdid=0;
    /*读取屏幕ID*/
    lcdid=lcd_read_panelid();
    CRlcd_dev.id=lcdid;
    printf("LCD ID=%#X\r\n", lcdid);    /*以十进制打印*/
    
    lcdgpio_Init();   /*io初始化*/
    lcd_reset();      /*复位LCD*/
    delay_ms(10);     /*延时10ms*/
    lcd_noreset();    /*结束复位*/

    /* CRLCD参数结构体初始化 */
    if(lcdid==ATK4342)
    {
        CRlcd_dev.height=272;
        CRlcd_dev.width=480;
        CRlcd_dev.vspw=1;
        CRlcd_dev.vbpd=8;
        CRlcd_dev.vfpd=8;
        CRlcd_dev.hspw=1;
        CRlcd_dev.hbpd=40;
        CRlcd_dev.hfpd=5;
        lcd_clkInit(25,8,8);   /*时钟初始化，9.37MHz */
    }else if(lcdid==ATK4384)
    {
        CRlcd_dev.height=480;
        CRlcd_dev.width=800;
        CRlcd_dev.vspw=3;
        CRlcd_dev.vbpd=32;
        CRlcd_dev.vfpd=13;
        CRlcd_dev.hspw=48;
        CRlcd_dev.hbpd=88;
        CRlcd_dev.hfpd=40;
        lcd_clkInit(42,4,8);   /*时钟初始化，31.5MHz */
    }else if(lcdid==ATK7084)
    {
        CRlcd_dev.height=480;
        CRlcd_dev.width=800;
        CRlcd_dev.vspw=1;
        CRlcd_dev.vbpd=23;
        CRlcd_dev.vfpd=22;
        CRlcd_dev.hspw=1;
        CRlcd_dev.hbpd=46;
        CRlcd_dev.hfpd=210;
        lcd_clkInit(30,3,7);   /*时钟初始化，34.2MHz */
    }else if(lcdid==ATK7016)
    {
        CRlcd_dev.height=600;
        CRlcd_dev.width=1024;
        CRlcd_dev.vspw=3;
        CRlcd_dev.vbpd=20;
        CRlcd_dev.vfpd=12;
        CRlcd_dev.hspw=20;
        CRlcd_dev.hbpd=140;
        CRlcd_dev.hfpd=160;
        lcd_clkInit(32,3,5);   /*时钟初始化，51.2MHz */
    }else if(lcdid==ATK1018)
    {
	    CRlcd_dev.height=800;	
	    CRlcd_dev.width=280;
	    CRlcd_dev.vspw=3;
	    CRlcd_dev.vbpd=10;
	    CRlcd_dev.vfpd=10;
	    CRlcd_dev.hspw=10;
	    CRlcd_dev.hbpd=80;
	    CRlcd_dev.hfpd=70;
		lcd_clkInit(35,3,5);	/* 初始化LCD时钟 56MHz */
    }else if(lcdid==ATKVGA)
    {
	    CRlcd_dev.height=768;	
	    CRlcd_dev.width=1366;
	    CRlcd_dev.vspw=3;
	    CRlcd_dev.vbpd=24;
	    CRlcd_dev.vfpd=3;
	    CRlcd_dev.hspw=143;
	    CRlcd_dev.hbpd=213;
	    CRlcd_dev.hfpd=70;
		lcd_clkInit(32,3,3);	/* 初始化LCD时钟 85MHz */
    }
    CRlcd_dev.pixsize=4;    /* ARGB8888模式，每个像素4字节 */
    CRlcd_dev.framebuffer=LCD_FRAMEBUF_ADDR;	
	CRlcd_dev.backcolor = LCD_WHITE;	    /* 背景色为白色 */
	CRlcd_dev.forecolor = LCD_BLACK;	    /* 前景色为黑色 */

    /*初始化ELCDIF_CTRL寄存器*/
    LCDIF->CTRL&=~((1<<1)|(3<<12)|(3<<14)|(1<<31));    /*需要置0的位*/
    LCDIF->CTRL|=(1<<5)|(3<<8)|(3<<10)|(1<<17)|(1<<19);       /*需要置1的位，先不使能LCDIF*/

    /*初始化LCDIF_CTRL1寄存器*/
    LCDIF->CTRL1 =0 ;   /*先清零*/
    LCDIF->CTRL1 |=(0x7<<16);

    /*初始化LCDIF_TRANSFER_COUNT寄存器*/
    LCDIF->TRANSFER_COUNT=0;   /*先清零*/
    LCDIF->TRANSFER_COUNT=(CRlcd_dev.height<<16)|(CRlcd_dev.width<<0);

    /*初始化LCDIF_VDCTRL0寄存器*/
    LCDIF->VDCTRL0=0;   /*先清零*/
    if(lcdid ==ATKVGA)   /*VGA需要特殊处理*/
    {
        LCDIF->VDCTRL0=(0<<29)|(1<<28)|(0<<27)|(0<<26)|(1<<25)|(0<<24)|(1<<21)|(1<<20)|(CRlcd_dev.vspw<<0);
    }else
    {
        LCDIF->VDCTRL0&=~((1<<25)|(1<<26)|(1<<27)|(1<<29));    /*需要置0的位*/ 
        LCDIF->VDCTRL0|=(CRlcd_dev.vspw<<0)|(1<<20)|(1<<21)|(1<<24)|(1<<28);    /*需要置1的位*/ 
    }

    /*初始化LCDIF_VDCTRL1寄存器*/
    LCDIF->VDCTRL1=0;   /*先清零*/
    LCDIF->VDCTRL1=	CRlcd_dev.vspw+CRlcd_dev.height+CRlcd_dev.vbpd+CRlcd_dev.vfpd;  /*VSYNC周期*/

    /*初始化LCDIF_VDCTRL2寄存器*/
    LCDIF->VDCTRL2=0;  /*先清零*/
    LCDIF->VDCTRL2=(CRlcd_dev.hspw<<18)|(CRlcd_dev.hspw+CRlcd_dev.width+CRlcd_dev.hbpd+CRlcd_dev.hfpd);

    /*初始化LCDIF_VDCTRL3寄存器*/
    LCDIF->VDCTRL3=0;   /*先清零*/
    LCDIF->VDCTRL3=((CRlcd_dev.hspw+CRlcd_dev.hbpd)<<16)|(CRlcd_dev.vspw+CRlcd_dev.vbpd);


    /*初始化LCDIF_VDCTRL4寄存器*/
    LCDIF->VDCTRL4=0;   /*先清零*/
    LCDIF->VDCTRL4|=(1<<18)|(CRlcd_dev.width);

    /*初始化LCDIF_CUR_BUF寄存器和LCDIF_NEXT_BUF寄存器*/
    LCDIF->CUR_BUF=(uint32_t)CRlcd_dev.framebuffer;
    LCDIF->NEXT_BUF=(uint32_t)CRlcd_dev.framebuffer;

    lcd_enable();    /* 使能LCD*/
    delay_ms(10);
    lcd_clear(LCD_WHITE);     /*清屏*/

}

/*
 * 读取屏幕ID，
 * 描述：LCD_DATA23=R7(M0) LCD_DATA15=G7(M1) LCD_DATA07=B7(M2);
 * 		M2:M1:M0
 *		0 :0 :0	//4.3寸480*272 RGB屏,ID=0X4342
 *		0 :0 :1	//7寸800*480 RGB屏,ID=0X7084
 *	 	0 :1 :0	//7寸1024*600 RGB屏,ID=0X7016
 *  	1 :0 :1	//10.1寸1280*800,RGB屏,ID=0X1018
 *		1 :0 :0	//4.3寸800*480 RGB屏,ID=0X4384
 * @param 		: 无
 * @return 		: 屏幕ID
*/
uint16_t lcd_read_panelid(void)
{
    uint8_t lcdid=0;
    /*需要读取屏幕ID，首先需要配置屏幕ID信号线，即LCD VSYNC拉高，则三个模拟开关SGM3157打开*/
    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0);
	IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_GPIO3_IO03, 0X10B0);   /*输出的默认电气属性，例如led灯*/

    /* 打开模拟开关 */
	gpio_pin_config_t lcdio_config;
	lcdio_config.direction = KGPIO_DigitalOutput; 
	lcdio_config.outputLogic = 1;
	gpio_Init(GPIO3, 3, &lcdio_config);

    /* 读取ID值，设置B7 G7 R7为输入 */
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_GPIO3_IO12, 0);   /*IO复用*/
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_GPIO3_IO20, 0);   /*IO复用*/
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_GPIO3_IO28, 0);   /*IO复用*/
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_GPIO3_IO12,0xf080);   /*输入的默认电气属性，例如按键key*/
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_GPIO3_IO20,0xf080);   /*输入的默认电气属性，例如按键key*/
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_GPIO3_IO28,0xf080);   /*输入的默认电气属性，例如按键key*/

    gpio_pin_config_t idio_config;
    idio_config.direction=KGPIO_DigitalInput;   /*设置屏幕ID对应的GPIO为输入*/
    gpio_Init(GPIO3,12,&idio_config);
	gpio_Init(GPIO3,20,&idio_config);
	gpio_Init(GPIO3,28,&idio_config);     /*初始化IO*/

    lcdid=(uint8_t)gpio_pinread(GPIO3,28);            /*读取M0，强制类型转换*/
    lcdid|=(uint8_t)gpio_pinread(GPIO3,20)<<1;        /*读取M1，左移1位*/
    lcdid|=(uint8_t)gpio_pinread(GPIO3,12)<<2;        /*读取M2，左移2位*/

    if (lcdid==0)
    {
        return ATK4342;
    }
    else if (lcdid==1)
    {
        return ATK7084;
    }
    else if (lcdid==2)
    {
        return ATK7016;
    }
    else if (lcdid==4)
    {
        return ATK4384;
    }  
    else if (lcdid==5)
    {
        return ATK1018;
    }
    else if (lcdid==7)
    {
        return ATKVGA;
    }
    else return 0;
}

/*
 * IO引脚: 	LCD_DATA00 -> LCD_B0
 *			LCD_DATA01 -> LCD_B1
 *			LCD_DATA02 -> LCD_B2
 *			LCD_DATA03 -> LCD_B3
 *			LCD_DATA04 -> LCD_B4
 *			LCD_DATA05 -> LCD_B5
 *			LCD_DATA06 -> LCD_B6
 *			LCD_DATA07 -> LCD_B7
 *
 *			LCD_DATA08 -> LCD_G0
 *			LCD_DATA09 -> LCD_G1
 *			LCD_DATA010 -> LCD_G2
 *			LCD_DATA011 -> LCD_G3
 *			LCD_DATA012 -> LCD_G4
 *			LCD_DATA012 -> LCD_G4
 *			LCD_DATA013 -> LCD_G5
 *			LCD_DATA014 -> LCD_G6
 *			LCD_DATA015 -> LCD_G7
 *
 *			LCD_DATA016 -> LCD_R0
 *			LCD_DATA017 -> LCD_R1
 *			LCD_DATA018 -> LCD_R2 
 *			LCD_DATA019 -> LCD_R3
 *			LCD_DATA020 -> LCD_R4
 *			LCD_DATA021 -> LCD_R5
 *			LCD_DATA022 -> LCD_R6
 *			LCD_DATA023 -> LCD_R7
 *
 *			LCD_CLK -> LCD_CLK
 *			LCD_VSYNC -> LCD_VSYNC
 *			LCD_HSYNC -> LCD_HSYNC
 *			LCD_DE -> LCD_DE
 *			LCD_BL -> GPIO1_IO08   背光
*/
/*
 * @description	: LCD GPIO初始化
 * @param 		: 无
 * @return 		: 无
*/
void lcdgpio_Init(void)
{
    /*IO复用*/
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA00_LCDIF_DATA00,0);  
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA01_LCDIF_DATA01,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA02_LCDIF_DATA02,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA03_LCDIF_DATA03,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA04_LCDIF_DATA04,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA05_LCDIF_DATA05,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA06_LCDIF_DATA06,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA07_LCDIF_DATA07,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA08_LCDIF_DATA08,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA09_LCDIF_DATA09,0); 
    IOMUXC_SetPinMux(IOMUXC_LCD_DATA10_LCDIF_DATA10,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA11_LCDIF_DATA11,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA12_LCDIF_DATA12,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA13_LCDIF_DATA13,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA14_LCDIF_DATA14,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA15_LCDIF_DATA15,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA16_LCDIF_DATA16,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA17_LCDIF_DATA17,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA18_LCDIF_DATA18,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA19_LCDIF_DATA19,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA20_LCDIF_DATA20,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA21_LCDIF_DATA21,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA22_LCDIF_DATA22,0);
	IOMUXC_SetPinMux(IOMUXC_LCD_DATA23_LCDIF_DATA23,0);

    IOMUXC_SetPinMux(IOMUXC_LCD_CLK_LCDIF_CLK,0);
    IOMUXC_SetPinMux(IOMUXC_LCD_ENABLE_LCDIF_ENABLE,0);
    IOMUXC_SetPinMux(IOMUXC_LCD_VSYNC_LCDIF_VSYNC,0);
    IOMUXC_SetPinMux(IOMUXC_LCD_HSYNC_LCDIF_HSYNC,0);


    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO08_GPIO1_IO08,0);   /*背光引脚*/
	
    /* 配置LCD IO电气属性	
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 0 默认22K上拉
	 *bit [13]: 0 pull功能
	 *bit [12]: 0 pull/keeper使能 
	 *bit [11]: 0 关闭开路输出
	 *bit [7:6]: 10 速度100Mhz
	 *bit [5:3]: 111 驱动能力为R0/7
	 *bit [0]: 1 高转换率
	*/
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA00_LCDIF_DATA00,0xB9);  
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA01_LCDIF_DATA01,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA02_LCDIF_DATA02,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA03_LCDIF_DATA03,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA04_LCDIF_DATA04,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA05_LCDIF_DATA05,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA06_LCDIF_DATA06,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA07_LCDIF_DATA07,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA08_LCDIF_DATA08,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA09_LCDIF_DATA09,0xB9); 
    IOMUXC_SetPinConfig(IOMUXC_LCD_DATA10_LCDIF_DATA10,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA11_LCDIF_DATA11,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA12_LCDIF_DATA12,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA13_LCDIF_DATA13,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA14_LCDIF_DATA14,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA15_LCDIF_DATA15,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA16_LCDIF_DATA16,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA17_LCDIF_DATA17,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA18_LCDIF_DATA18,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA19_LCDIF_DATA19,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA20_LCDIF_DATA20,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA21_LCDIF_DATA21,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA22_LCDIF_DATA22,0xB9);
	IOMUXC_SetPinConfig(IOMUXC_LCD_DATA23_LCDIF_DATA23,0xB9);

    IOMUXC_SetPinConfig(IOMUXC_LCD_CLK_LCDIF_CLK,0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_ENABLE_LCDIF_ENABLE,0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_VSYNC_LCDIF_VSYNC,0xB9);
    IOMUXC_SetPinConfig(IOMUXC_LCD_HSYNC_LCDIF_HSYNC,0xB9);


    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO08_GPIO1_IO08,0xB9);   /*背光引脚*/

    gpio_pin_config_t BLgpio_config;
    BLgpio_config.direction=KGPIO_DigitalOutput;
    BLgpio_config.outputLogic=1;
    gpio_Init(GPIO1,8,&BLgpio_config);   /*设置GPIO1_IO08背光引脚为输出，默认打开背光*/
    gpio_pinwrite(GPIO1,8,1);  


}

/*
 * @description		: LCD时钟初始化, LCD时钟计算公式如下：
 *                	  LCD CLK = 24 * loopDiv / prediv / div
 * @param -	loopDiv	: PLL5 DIV_SELECT值，值范围为27～54
 * @param -	prediv  : LCDIF11根时钟预分频值
 * @param -	div		: LCDIF11根时钟后分频值
 * @return 			: 无
*/
void lcd_clkInit(uint8_t loopDiv,uint8_t prediv,uint8_t div)
{

    /*先设置PLL5(Video PLL)为lcd的时钟源*/
    CCM_ANALOG->PLL_VIDEO_NUM=0;        /*关闭小数分频器*/
    CCM_ANALOG->PLL_VIDEO_DENOM=0;

    CCM_ANALOG->PLL_VIDEO &=~((1<<12)|(1<<13)|(1<<16)|(3<<19)|(0x7f<<0));       
    CCM_ANALOG->PLL_VIDEO |=(1<<13)|(2<<19)|(loopDiv<<0);   /*使能PLL_VIDEO输出，设置PLL5预分频，设置DIV_SELECT*/

    CCM_ANALOG->MISC2 &=~(3<<30);   
    CCM_ANALOG->MISC2 |=(0<<30);   /*设置PLL5后分频*/

    /*设置后续时钟选择*/
    CCM->CSCDR2 &= ~(7 << 15);  	
	CCM->CSCDR2 |= (2 << 15);			/* 设置LCDIF_PRE_CLK使用PLL5 */
    /* 设置LCDIF_PRE分频 */

	CCM->CSCDR2 &= ~(7<<12);		
	CCM->CSCDR2 |= (prediv-1) << 12;	/* 设置分频  */

    CCM->CBCMR &=~(7<<23);
    CCM->CBCMR |=(div-1)<<23;       /*lcdif1时钟的后分频值*/

    CCM->CSCDR2 &=~(7<<9);
    CCM->CSCDR2 |=(0<<9);		/* LCDIF_PRE时钟源选择LCDIF_PRE时钟 */


}

/*
 * @description	: 复位ELCDIF接口
 * @param 		: 无
 * @return 		: 无
*/
void lcd_reset(void)
{   
    LCDIF->CTRL &=~(1<<30);   /* 此位必须设置为零才能正常工作。当设置为1时，它关闭块的时钟，且无法设置寄存器值。 */
    LCDIF->CTRL |=(1<<31);   /* 强制复位 */ 

    // LCDIF->CTRL  = 1<<31; /* 强制复位 */
}

/*
 * @description	: 结束复位ELCDIF接口
 * @param 		: 无
 * @return 		: 无
*/
void lcd_noreset(void)
{
    LCDIF->CTRL &=~(1<<31);   /* 取消强制复位 */

}

/*
 * @description	: 使能ELCDIF接口
 * @param 		: 无
 * @return 		: 无
*/
void lcd_enable(void)
{
    LCDIF->CTRL &=~(1<<0);   
    LCDIF->CTRL |=(1<<0);     /* 使能ELCDIF */
}

/*
 * @description		: 画点函数 
 * @param - x		: x轴坐标——ATK7016:范围(0,600)
 * @param - y		: y轴坐标——ATK7016:范围(0,1024)
 * @param - color	: 颜色值
 * @return 			: 无
 * 
 * inline(内联函数)是一种“用于实现的关键字”，其实在内部的工作就是在for循环的内部任何函数的地方都换成了表达式，避免了频繁调用函数对栈内存重复开辟所带来的消耗
 * inline只适合函数体内代码简单的函数数使用，不能包含复杂的结构控制语句例如while、switch，并且内联函数本身不能是直接递归函数
*/
inline void lcd_drawpoint(uint16_t x,uint16_t y,uint32_t color)
{
    /*由x和y坐标算出当前位置显存地址+偏移*/
    *(uint32_t *)((uint32_t)CRlcd_dev.framebuffer+CRlcd_dev.pixsize*(CRlcd_dev.width*y+x)) =color;             
}

/*
 * @description		: 读取指定点的颜色值
 * @param - x		: x轴坐标
 * @param - y		: y轴坐标
 * @return 			: 读取到的指定点的颜色值
*/
inline uint32_t lcd_readpoint(uint16_t x,uint16_t y)
{
    uint32_t reg;
    reg=*(uint32_t *)((uint32_t)CRlcd_dev.framebuffer+CRlcd_dev.pixsize*(CRlcd_dev.width*y+x));
    return reg;
}


/*
 * @description		: 清屏
 * @param - color	: 颜色值
 * @return 			: 无
*/
void lcd_clear(uint32_t color)
{
    uint32_t NUM;
    uint32_t i=0;
    uint32_t *startaddr=(uint32_t *)CRlcd_dev.framebuffer;   /*指向帧缓存首地址，其中(uint32_t *)——类型强制转换为地址*/
    NUM=(uint32_t)CRlcd_dev.width*CRlcd_dev.height;          /*4字节颜色缓冲区总长度*/
    for(i=0;i<NUM;i++)
    {
        startaddr[i]=color;      /*向数组中赋值*/
    }

}

/*
 * @description		: 以指定的颜色填充一块矩形
 * @param - x0		: 矩形起始点坐标X轴
 * @param - y0		: 矩形起始点坐标Y轴
 * @param - x1		: 矩形终止点坐标X轴
 * @param - y1		: 矩形终止点坐标Y轴
 * @param - color	: 要填充的颜色
 * @return 			: 无
*/
void lcd_fill(uint16_t x_begin,uint16_t y_begin,uint16_t x_last,uint16_t y_last,uint32_t color)
{
    uint16_t x,y;
    if(x_begin<0)
    {
        x_begin=0;
    }
    if(y_begin<0)
    {
        y_begin=0;
    }
    if(x_last>=CRlcd_dev.width)
    {
        x_begin=CRlcd_dev.width-1;
    }
    if(x_last>=CRlcd_dev.width)
    {
        x_begin=CRlcd_dev.width-1;
    }

    for(x=x_begin;x<=x_last;x++)
    {
        for(y=y_begin;y<=y_last;y++)
        {
            lcd_drawpoint(x,y,color);
        }
    }
}