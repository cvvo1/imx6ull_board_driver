#ifndef _BSP_LCD_H
#define _BSP_LCD_H
#include "imx6ul.h"

/* ARGB颜色 */
#define LCD_BLUE		  0x000000FF
#define LCD_GREEN		  0x0000FF00
#define LCD_RED 		  0x00FF0000
#define LCD_CYAN		  0x0000FFFF
#define LCD_MAGENTA 	  0x00FF00FF
#define LCD_YELLOW		  0x00FFFF00
#define LCD_LIGHTBLUE	  0x008080FF
#define LCD_LIGHTGREEN	  0x0080FF80
#define LCD_LIGHTRED	  0x00FF8080
#define LCD_LIGHTCYAN	  0x0080FFFF
#define LCD_LIGHTMAGENTA  0x00FF80FF
#define LCD_LIGHTYELLOW   0x00FFFF80
#define LCD_DARKBLUE	  0x00000080
#define LCD_DARKGREEN	  0x00008000
#define LCD_DARKRED 	  0x00800000
#define LCD_DARKCYAN	  0x00008080
#define LCD_DARKMAGENTA   0x00800080
#define LCD_DARKYELLOW	  0x00808000
#define LCD_WHITE		  0x00FFFFFF
#define LCD_LIGHTGRAY	  0x00D3D3D3
#define LCD_GRAY		  0x00808080
#define LCD_DARKGRAY	  0x00404040
#define LCD_BLACK		  0x00000000
#define LCD_BROWN		  0x00A52A2A
#define LCD_ORANGE		  0x00FFA500
#define LCD_TRANSPARENT   0x00000000

/* 屏幕ID */
#define ATK4342		0X4342	/* 4.3寸480*272 	*/
#define ATK4384		0X4384	/* 4.3寸800*480 	*/
#define ATK7084		0X7084	/* 7寸800*480 		*/
#define ATK7016		0X7016	/* 7寸1024*600 		*/
#define ATK1018		0X1018	/* 10.1寸1280*800 	*/
#define ATKVGA		0xff00 /* VGA */

/* LCD显存地址 */
#define LCD_FRAMEBUF_ADDR	(0x89000000)

/* LCD控制参数结构体 */
struct CRlcd_typedef{
    uint16_t height;	/* LCD屏幕高度 */
	uint16_t width;		/* LCD屏幕宽度 */
	uint8_t pixsize;	/* LCD每个像素所占字节大小 */
	uint16_t vspw;      /*VSYNC信号宽度，也就是VSYNC信号持续时间*/
	uint16_t vbpd;      /*帧同步信号后肩，单位为1行的时间*/
	uint16_t vfpd;      /*帧同步信号前肩，单位为1行的时间*/
	uint16_t hspw;       /*HSYNC信号宽度，也就是HSYNC信号持续时间*/
	uint16_t hbpd;       /*行同步信号后肩，单位是CLK*/
	uint16_t hfpd;       /*行同步信号前肩，单位是CLK*/
	uint32_t framebuffer; 	/* LCD显存首地址   	  */
	uint32_t forecolor;		/* 前景色 */
	uint32_t backcolor;		/* 背景色 */
	uint32_t id;  			/*	屏幕ID */
};

extern struct CRlcd_typedef CRlcd_dev;   /*extern是C语言的一个关键字，用于声明全局变量和函数*/

void lcd_Init(void);
uint16_t lcd_read_panelid(void);
void lcdgpio_Init(void);
void lcd_clkInit(uint8_t loopDiv,uint8_t prediv,uint8_t div);
void lcd_reset(void);
void lcd_noreset(void);
void lcd_enable(void);
inline void lcd_drawpoint(uint16_t x,uint16_t y,uint32_t color);
inline uint32_t lcd_readpoint(uint16_t x,uint16_t y);
void lcd_clear(uint32_t color);
void lcd_fill(uint16_t x_begin,uint16_t y_begin,uint16_t x_last,uint16_t y_last,uint32_t color);

#endif // !_BSP_LCD_H