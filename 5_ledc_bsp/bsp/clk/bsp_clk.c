#include "bsp_clk.h"

/*使能IMX6U所有外设时钟*/
void clk_enable(void)
{
    CCM->CCGR0=0xFFFFFFFF;
    CCM->CCGR0=0xFFFFFFFF;
    CCM->CCGR0=0xFFFFFFFF;
    CCM->CCGR0=0xFFFFFFFF;
    CCM->CCGR0=0xFFFFFFFF;
    CCM->CCGR0=0xFFFFFFFF;
    CCM->CCGR0=0xFFFFFFFF;
}