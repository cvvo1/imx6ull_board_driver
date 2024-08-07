#include "bsp_rtc.h"
#include "time.h"

/* SRTC需要外界提供一个32.768KHz的时钟，核心板上有此晶振
 * 1、初始化SNVS_SRTC，初始化 SNVS_LP中的SRTC。
 *  SNVS_HPCOMR寄存器：
 *      NPSWA_EN(bit31)，置1，表示所有的软件都可以访问SNVS所有寄存器
 *      SW_SV(Bit8)，软件安全违规，也是和安全有关的，我们置1，也可以不置1
 * 2、设置RTC时间。第一次使用RTC肯定要先设置时间。
 *  LPSRTCMR寄存器(bit14:0),作为SRTC计数器的高15位
 *  LPSRTCLR寄存器(bit31：15),作为SRTC计数器的低17位
 * 3、使能RTC。配置好RTC并设置好初始时间以后就可以开启RTC了。
 *  SNVS_LP Control寄存器：
 *      SRTC_ENV(bit0)，安全实时计数器已启用且有效设置后，SRTC即可运行，设置为1
 *  
 * 
*/

struct rtc_datetime rtcdate;
/*初始化RTC*/
void rtc_Init(void)
{
    SNVS->HPCOMR |=(1<<31)|(1<<8);

    // rtcdate.year = 2024U;
    // rtcdate.month = 4U;
    // rtcdate.day = 12U;
    // rtcdate.hour = 17U;
    // rtcdate.minute = 00;
    // rtcdate.second = 00;
    // rtc_setdatetime(&rtcdate);   //初始化时间和日期
    
    rtc_enable();

}

/*开启RTC*/
void rtc_enable(void)
{
    SNVS->LPCR |=(1<<0);
    while(!(SNVS->LPCR>>0)&0x01);
}

/*关闭RTC*/
void rtc_disable(void)
{
    SNVS->LPCR &=~(1<<0);
    while((SNVS->LPCR>>0)&0x1);
}

/*
 * @description	: 判断指定年份是否为闰年，闰年条件如下:
 * @param - year: 要判断的年份
 * @return 		: 1 是闰年，0 不是闰年
 */
uint8_t rtc_isleapyear(uint16_t year)
{	
	uint8_t value=0;
	
	if(year % 400 == 0)
		value = 1;
	else 
	{
		if((year % 4 == 0) && (year % 100 != 0))
			value = 1;
		else 
			value = 0;
	}
	return value;
}

/*
 * @description		: 将时间转换为秒数
 * @param - datetime: 要转换日期和时间。
 * @return 			: 转换后的秒数
*/
uint32_t rtc_coverdate_to_seconds(struct rtc_datetime *datetime)
{	
	uint16_t i = 0;
	uint32_t seconds = 0;
	uint32_t days = 0;
	uint16_t monthdays[] = {0U, 0U, 31U, 59U, 90U, 120U, 151U, 181U, 212U, 243U, 273U, 304U, 334U};   /*月对应总天数*/
	
	for(i = 1970; i < datetime->year; i++)
	{
		days += DAYS_IN_A_YEAR; 		/* 平年，每年365天 */
		if(rtc_isleapyear(i)) days += 1;/* 闰年多加一天 		*/
	}

	days += monthdays[datetime->month];   /*得到当前年所在月份对应当年总天数*/

	if(rtc_isleapyear(i) && (datetime->month >= 3)) 
    {
        days += 1;/* 闰年，并且当前月份大于等于3月的话加一天 */
    }

	days += datetime->day - 1;           /*减去当前天数*/

	seconds = days * SECONDS_IN_A_DAY + 
				datetime->hour * SECONDS_IN_A_HOUR +
				datetime->minute * SECONDS_IN_A_MINUTE +
				datetime->second;           /*由总天数得到总时间*/
	return seconds;	
}

/*
 * @description		: 设置时间和日期
 * @param - datetime: 要设置的日期和时间
 * @return 			: 无
*/
void rtc_setdatetime(struct rtc_datetime *datetime)
{
	
	unsigned int seconds = 0;
	unsigned int tmp = SNVS->LPCR; 
	
	rtc_disable();	/* 设置寄存器HPRTCMR和HPRTCLR的时候一定要先关闭RTC */

	
	/* 先将时间转换为秒         */
	seconds = rtc_coverdate_to_seconds(datetime);
	
	SNVS->LPSRTCMR = (unsigned int)(seconds >> 17); /* 设置高16位 */
	SNVS->LPSRTCLR = (unsigned int)(seconds << 15); /* 设置地16位 */

	/* 如果此前RTC是打开的在设置完RTC时间以后需要重新打开RTC */
	if (tmp & 0x1)
		rtc_enable();
}

/*
 * @description		: 将秒数转换为时间
 * @param - seconds	: 要转换的秒数
 * @param - datetime: 转换后的日期和时间
 * @return 			: 无
 */
void rtc_convertseconds_to_datetime(uint64_t seconds, struct rtc_datetime *datetime)
{
    uint64_t x;
    uint64_t  secondsRemaining, days;
    unsigned short daysInYear;
    /* 每个月的天数       */
    unsigned char daysPerMonth[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    secondsRemaining = seconds; /* 剩余秒数初始化 */
    days = secondsRemaining / SECONDS_IN_A_DAY + 1; 		/* 根据秒数计算天数,加1是当前天数 */
    secondsRemaining = secondsRemaining % SECONDS_IN_A_DAY;    /*计算天数以后剩余的秒数 */

	/* 计算时、分、秒 */
    datetime->hour = secondsRemaining / SECONDS_IN_A_HOUR;
    secondsRemaining = secondsRemaining % SECONDS_IN_A_HOUR;
    datetime->minute = secondsRemaining / 60;
    datetime->second = secondsRemaining % SECONDS_IN_A_MINUTE;

    /* 计算年 */
    daysInYear = DAYS_IN_A_YEAR;
    datetime->year = YEAR_RANGE_START;
    while(days > daysInYear)
    {
        /* 根据天数计算年 */
        days -= daysInYear;
        datetime->year++;

        /* 处理闰年 */
        if (!rtc_isleapyear(datetime->year))
            daysInYear = DAYS_IN_A_YEAR;
        else	/*闰年，天数加一 */
            daysInYear = DAYS_IN_A_YEAR + 1;
    }
	/*根据剩余的天数计算月份 */
    if(rtc_isleapyear(datetime->year)) /* 如果是闰年的话2月加一天 */
        daysPerMonth[2] = 29;

    for(x = 1; x <= 12; x++)
    {
        if (days <= daysPerMonth[x])
        {
            datetime->month = x;
            break;
        }
        else
        {
            days -= daysPerMonth[x];
        }
    }
    datetime->day = days;
}


/*
 * @description	: 获取RTC当前秒数
 * @param 		: 无
 * @return 		: 当前秒数 
 */
unsigned int rtc_getseconds(void)
{
	unsigned int seconds = 0;
	seconds = (SNVS->LPSRTCMR << 17) | (SNVS->LPSRTCLR >> 15);
	return seconds;
}

/*
 * @description		: 获取当前时间
 * @param - datetime: 获取到的时间，日期等参数
 * @return 			: 无 
 */
void rtc_getdatetime(struct rtc_datetime *datetime)
{
	//unsigned int seconds = 0;
	u64 seconds;
	seconds = rtc_getseconds();
	rtc_convertseconds_to_datetime(seconds, datetime);	
}