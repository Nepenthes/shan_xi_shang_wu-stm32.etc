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

#endif

