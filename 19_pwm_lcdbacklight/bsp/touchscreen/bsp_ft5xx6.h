#ifndef _BSP_FT5XX6_H
#define _BSP_FT5XX6_H
#include "imx6ul.h"

#define FT5426_ADDR				0X38	/* FT5426设备地址 		*/

/*寄存器说明*/
#define FT5426_DEVICE_MODE		0X00 	/* 模式寄存器bit(6:4)：000：正常模式、001：系统信息模式、100：测试模式*/
#define FT5426_TD_STATUS		0X02	/* 触摸状态寄存器bit(3:0):记录有多少个触摸点，有效值为 1~5。当前触摸点1～5 			*/
#define FT5426_TOUCH1_XH		0X03	/* 触摸点坐标寄存器,开始记录着触摸屏的触摸点坐标信息
										   一个触摸点坐标信息用12bit表示，其中H寄存器的(bit3:0)这4个bit为高4位，L寄存器的(bit7:0)为低8位
										   XH寄存器，(bit7:6)表示事件，事件标志：00：按下、01：抬起、10：接触、11：保留
										   YH寄存器，(bit7:4)表示触摸ID，
										 * 一个触摸点用6个寄存器存储坐标数据，后两个寄存器保留，共30个寄存器*/
#define FT5426_IDGLIB_VERSION1	0XA1 	/* 固件版本寄存器，(bit7:0)高字节 */
#define FT5426_IDGLIB_VERSION2	0XA2 	/* 固件版本寄存器，(bit7:0)低字节 */
#define FT5426_IDG_MODE			0XA4	/* 中断模式(bit7:0),用于设置中断模式0：轮询模式、1：触发模式*/

#define FT5426_XYCOORDREG_NUM	30		/* 触摸点坐标寄存器数量 */
#define FT5426_INIT_FINISHED	1		/* 触摸屏初始化完成 			*/
#define FT5426_INIT_NOTFINISHED	0		/* 触摸屏初始化未完成 			*/

#define FT5426_TOUCH_EVENT_DOWN			0x00	/* 按下 		*/
#define FT5426_TOUCH_EVENT_UP			0x01	/* 释放 		*/
#define FT5426_TOUCH_EVENT_ON			0x02	/* 接触 		*/
#define FT5426_TOUCH_EVENT_RESERVED		0x03	/* 没有事件 */


/* FT5426触摸屏结构体 */
struct ft5xx6_dev_struc
{	
	uint8_t initfalg;		/* 触摸屏初始化状态 */
	uint8_t intflag;		/* 标记中断有没有发生 */
	uint8_t point_num;	/* 触摸点 		*/
	uint16_t x[5];		/* X轴坐标 	*/
	uint16_t y[5];		/* Y轴坐标 	*/
};

struct ft5xx6_dev_struc ft5426_dev;    /*声明为外部可调用*/

void ft5xx6_Init(void);
void gpio1_io09_irqhandle(unsigned int giccIar, void *param);
uint8_t ft5xx6_sendbyte(uint8_t addr,uint8_t reg,uint8_t data);
uint8_t ft5xx6_receivebyte(uint8_t addr,uint8_t reg);
void ft5xx6_receive_array(uint8_t addr,uint8_t reg,uint8_t *array,uint8_t length);
void ft5xx6_touch_num(void);
void ft5xx6_touch_coordinate(void);

#endif // !_BSP_FT5XX6_H