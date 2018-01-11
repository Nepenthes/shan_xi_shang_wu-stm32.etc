#ifndef TEMP_MS_H
#define TEMP_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{

	float temp;
	float hum;
}tempMS_MEAS;

extern osThreadId 	 tid_tempMS_Thread;
extern osPoolId  	 tempMS_pool;
extern osMessageQId  MsgBox_tempMS;
extern osMessageQId  MsgBox_MTtempMS;
extern osMessageQId  MsgBox_DPtempMS;

void tempMS_Init(void);
void tempMS_Thread(const void *argument);
void tempMSThread_Active(void);

/***********************************************************************
 * CONSTANTS
 */
#define SHT_NOACK       0
#define SHT_ACK         1

#define SHT_MODE_TEMP   0
#define SHT_MODE_HUMI   1

//adr  command  r/w
#define SHT_STATUS_REG_W        0x06   //000   0011    0
#define SHT_STATUS_REG_R        0x07   //000   0011    1
#define SHT_MEASURE_TEMP        0x03   //000   0001    1
#define SHT_MEASURE_HUMI        0x05   //000   0010    1
#define SHT_RESET               0x1e

/***********************************************************************
 * MACROS
 */

typedef  uint8_t uint8;
typedef  uint16_t uint16;
typedef  u32 uint32;

typedef union
{
	uint8_t sensor_dat[4];
	struct{
		uint16_t temp;
		uint16_t hum;
	}hum_temp;
}result_t;

#define SHT_RD_DATA()  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)
#define SHT_DATA(level)  (PBout(7)=level)
#define SHT_DATA_OUT  (SHT_WInit())
#define SHT_DATA_IN  (SHT_RInit())
#define SHT_SCK(level)  (PBout(6)=level)

#define udelay()	delay_us(1);

#endif

