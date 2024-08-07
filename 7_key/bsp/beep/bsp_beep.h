#ifndef __BSP_BEEP_H 
#define __BSP_BEEP_H

#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"

void beep_Init(void);
void beep_switch(int status);

#endif // !__BSP_BEEP_H 
