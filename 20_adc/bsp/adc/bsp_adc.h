#ifndef _BSP_ADC_H
#define _BSP_ADC_H
#include "imx6ul.h"

#define kStatus_Success   (0)    /*校准成功*/
#define kStatus_Fail      (1)    /*校准失败*/


uint8_t adc1ch1_Init(void);
uint8_t adc1ch1_calibration(void);
uint32_t getadc_value(void);
uint32_t getadc_avg(uint8_t times);
uint32_t getadc_volt(void);


#endif // !_BSP_ADC_H