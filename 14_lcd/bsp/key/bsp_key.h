#ifndef _BSP_KEY_H
#define _BSP_KEY_H

#include "imx6ul.h"

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