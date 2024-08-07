#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "cc.h"
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "MCIMX6Y2.h"

/*枚举按键值*/
enum Keyvalue{
    KEY_NONE= 0,
    KEY0_VALUE,
	KEY1_VALUE,
	KEY2_VALUE,
};

void key_init(void);
uint8_t read_key(void);
uint8_t Key_GetNum(void);

#endif // !__BSP_KEY_H