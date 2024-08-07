#include "bsp_delay.h"

/*短延时*/
void delay_short(volatile unsigned int n)
{
	while(n--)
    {

    }
}

/*延时，
主频396MHz,设置位0x7ff,一次循环大概是1ms，n:延时毫秒数
主频528MHz,设置位0xaa9,一次循环大概是1ms，n:延时毫秒数
*/
void delay(volatile unsigned int n)
{
	while(n--)
	{
		delay_short(0xaa9);
	}
}