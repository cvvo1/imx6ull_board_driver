#ifndef _BSP_RTC_H
#define _BSP_RTC_H
#include "imx6ul.h"

extern uint16_t MyRTC_Time[];

/* 相关宏定义 */	
#define SECONDS_IN_A_DAY 		(86400) /* 一天86400秒	 		*/
#define SECONDS_IN_A_HOUR 		(3600)	/* 一个小时3600秒 		*/
#define SECONDS_IN_A_MINUTE 	(60)	/* 一分钟60秒  		 	*/
#define DAYS_IN_A_YEAR 			(365)	/* 一年365天 			*/
#define YEAR_RANGE_START 		(1970)	/* 开始年份1970年 		*/
#define YEAR_RANGE_END 			(2099)	/* 结束年份2099年 		*/

/* 时间日期结构体 */	
struct rtc_datetime
{
    uint16_t year;  /* 范围为:1970 ~ 2099 		*/
    uint8_t month;  /* 范围为:1 ~ 12				*/
    uint8_t day;    /* 范围为:1 ~ 31 (不同的月，天数不同).*/
    uint8_t hour;   /* 范围为:0 ~ 23 			*/
    uint8_t minute; /* 范围为:0 ~ 59				*/
    uint8_t second; /* 范围为:0 ~ 59				*/
};



void rtc_Init(void);
void rtc_enable(void);
void rtc_disable(void);
uint32_t rtc_coverdate_to_seconds(struct rtc_datetime *datetime);
uint32_t rtc_getseconds(void);
void rtc_setdatetime(struct rtc_datetime *datetime);
void rtc_getdatetime(struct rtc_datetime *datetime);

#endif // !_BSP_RTC_H