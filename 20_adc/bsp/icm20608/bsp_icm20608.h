#ifndef _BSP_ICM20608_H
#define _BSP_ICM20608_H
#include "imx6ul.h"

enum icm20608_CS
{
    icm20608_start = 0x0, 		/* SS0低电平 */
    icm20608_stop = 0x1,  		/* SS0高电平 */
} ;

#define ICM20608G_ID			0XAF	/* ID值 */
#define ICM20608D_ID			0XAE	/* ID值 */

/* ICM20608寄存器 
 *复位后所有寄存器地址都为0，除了
 *Register 107(0X6B) Power Management 1 	= 0x40
 *Register 117(0X75) WHO_AM_I 				= 0xAF或0xAE
 */
/* 陀螺仪和加速度自测(出产时设置，用于与用户的自检输出值比较） */
#define	ICM20_SELF_TEST_X_GYRO		0x00
#define	ICM20_SELF_TEST_Y_GYRO		0x01
#define	ICM20_SELF_TEST_Z_GYRO		0x02
#define	ICM20_SELF_TEST_X_ACCEL		0x0D
#define	ICM20_SELF_TEST_Y_ACCEL		0x0E
#define	ICM20_SELF_TEST_Z_ACCEL		0x0F

/* 陀螺仪静态偏移 */
#define	ICM20_XG_OFFS_USRH			0x13
#define	ICM20_XG_OFFS_USRL			0x14
#define	ICM20_YG_OFFS_USRH			0x15
#define	ICM20_YG_OFFS_USRL			0x16
#define	ICM20_ZG_OFFS_USRH			0x17
#define	ICM20_ZG_OFFS_USRL			0x18

#define	ICM20_SMPLRT_DIV			0x19  /*SMLPRT_DIV[7:0]，设置输出速率，输出速率计算公式如下：SAMPLE_RATE=INTERNAL_SAMPLE_RATE/(1 + SMPLRT_DIV)，
                                            其中INTERNAL_SAMPLE_RATE=1kHz*/
#define	ICM20_CONFIG			    0x1A  /*DLPF_CFG[2:0]，芯片配置，设置陀螺仪和温度传感器低通滤波。可设置 0~7*/
#define	ICM20_GYRO_CONFIG			0x1B  /*FS_SEL[4:3]，陀螺仪量程设置，0：±250dps； 1：±500dps； 2：±1000dps；3：±2000dps*/
#define	ICM20_ACCEL_CONFIG			0x1C  /*ACC_FS_SEL[4:3]，加速度计量程设置，0：±2g； 1：±4g； 2：±8g； 3：±16g*/
#define	ICM20_ACCEL_CONFIG2			0x1D  /*A_DLPF_CFG[2:0],加速度计低通滤波设置，设置加速度计的低通滤波，可设置 0~7*/
#define	ICM20_LP_MODE_CFG			0x1E  /*GYRO_CYCLE[7],陀螺仪低功耗使能，0：关闭陀螺仪的低功耗功能。1：使能陀螺仪的低功耗功能*/
#define	ICM20_ACCEL_WOM_THR			0x1F
#define	ICM20_FIFO_EN				0x23  /*FIFO 使能控制，TEMP_FIFO_EN[7] 1：使能温度传感器FIFO、0：关闭温度传感器FIFO
                                                          XG_FIFO_EN[6] 1：使能陀螺仪X轴FIFO、0：关闭陀螺仪X轴FIFO
                                                          YG_FIFO_EN[5] 1：使能陀螺仪Y轴FIFO、0：关闭陀螺仪Y轴FIFO
                                                          ZG_FIFO_EN[4] 1：使能陀螺仪Z轴FIFO、0：关闭陀螺仪Z轴 FIFO
                                                          ACCEL_FIFO_EN[3] 1：使能加速度计 FIFO、0：关闭加速度计 FIFO*/
#define	ICM20_FSYNC_INT				0x36   /*FSYNC中断状态*/
#define	ICM20_INT_PIN_CFG			0x37   /*INT/DRDY引脚/旁路启用配置*/
#define	ICM20_INT_ENABLE			0x38   /*中断使能*/
#define	ICM20_INT_STATUS			0x3A   /*中断状态*/

/* 加速度输出 */
#define	ICM20_ACCEL_XOUT_H			0x3B
#define	ICM20_ACCEL_XOUT_L			0x3C
#define	ICM20_ACCEL_YOUT_H			0x3D
#define	ICM20_ACCEL_YOUT_L			0x3E
#define	ICM20_ACCEL_ZOUT_H			0x3F
#define	ICM20_ACCEL_ZOUT_L			0x40

/* 温度输出 */
#define	ICM20_TEMP_OUT_H			0x41
#define	ICM20_TEMP_OUT_L			0x42

/* 陀螺仪输出 */
#define	ICM20_GYRO_XOUT_H			0x43
#define	ICM20_GYRO_XOUT_L			0x44
#define	ICM20_GYRO_YOUT_H			0x45
#define	ICM20_GYRO_YOUT_L			0x46
#define	ICM20_GYRO_ZOUT_H			0x47
#define	ICM20_GYRO_ZOUT_L			0x48

/**/
#define	ICM20_SIGNAL_PATH_RESET		0x68   /*信号通路复位*/
#define	ICM20_ACCEL_INTEL_CTRL 		0x69   /*加速度计智能控制*/
#define	ICM20_USER_CTRL				0x6A   /*用户控制寄存器*/
#define	ICM20_PWR_MGMT_1			0x6B   /*电源管理寄存器1，DEVICE_RESET[7]——1：复位 ICM-20608，
                                                           SLEEP[6]——0：退出休眠模式； 1，进入休眠模式
                                                           CLKSEL[2:0]——时钟源选择，0与6：内部20MHz振荡器，1～5：自动选择最佳可用时钟源-PLL，否则使用内部振荡器，7：停止时钟并保持定时发生器复位*/
#define	ICM20_PWR_MGMT_2			0x6C   /*电源管理寄存器2， STBY_XA[5] 0：使能加速度计 X 轴、1：关闭加速度计 X 轴
                                                            STBY_YA[4] 0：使能加速度计 Y 轴、1：关闭加速度计 Y 轴
                                                            STBY_ZA[3] 0：使能加速度计 Z 轴、1：关闭加速度计 Z 轴
                                                            STBY_XG[2] 0：使能陀螺仪 X 轴、1：关闭陀螺仪 X 轴
                                                            STBY_YG[1] 0：使能陀螺仪 Y 轴、1：关闭陀螺仪 Y 轴
                                                            STBY_ZG[0] 0：使能陀螺仪 Z 轴、1：关闭陀螺仪 Z 轴*/
#define	ICM20_FIFO_COUNTH			0x72
#define	ICM20_FIFO_COUNTL			0x73
#define	ICM20_FIFO_R_W				0x74
#define	ICM20_WHO_AM_I 				0x75

/* 加速度静态偏移 */
#define	ICM20_XA_OFFSET_H			0x77
#define	ICM20_XA_OFFSET_L			0x78
#define	ICM20_YA_OFFSET_H			0x7A
#define	ICM20_YA_OFFSET_L			0x7B
#define	ICM20_ZA_OFFSET_H			0x7D
#define	ICM20_ZA_OFFSET_L 			0x7E

/*ICM20608结构体*/
struct icm20608_dev_struc
{
	int32_t gyro_x_adc;		/* 陀螺仪X轴原始值 			*/
	int32_t gyro_y_adc;		/* 陀螺仪Y轴原始值 			*/
	int32_t gyro_z_adc;		/* 陀螺仪Z轴原始值 			*/
	int32_t accel_x_adc;		/* 加速度计X轴原始值 			*/
	int32_t accel_y_adc;		/* 加速度计Y轴原始值 			*/
	int32_t accel_z_adc;		/* 加速度计Z轴原始值 			*/
	int32_t temp_adc;		/* 温度原始值 				*/

	/* 下面是计算得到的实际值，扩大100倍 */
	int32_t gyro_x_act;		/* 陀螺仪X轴实际值 			*/
	int32_t gyro_y_act;		/* 陀螺仪Y轴实际值 			*/
	int32_t gyro_z_act;		/* 陀螺仪Z轴实际值 			*/
	int32_t accel_x_act;		/* 加速度计X轴实际值 			*/
	int32_t accel_y_act;		/* 加速度计Y轴实际值 			*/
	int32_t accel_z_act;		/* 加速度计Z轴实际值 			*/
	int32_t temp_act;		/* 温度实际值 				*/
};

extern struct icm20608_dev_struc icm20608_dev;	/* icm20608设备 */

uint8_t icm20608_Init(void);
void icm20608_CSN(enum icm20608_CS key);
void icm20608_send_reg(uint8_t addr,const uint8_t data);
uint8_t icm20608_receive_reg(uint8_t addr);
void icm20608_receive_array(uint8_t addr,uint8_t *array,uint8_t len);
float icm20608_gyro_scale(void);
float icm20608_accel_scale(void);
void icm20608_data(void);


#endif // !_BSP_ICM20608_H
