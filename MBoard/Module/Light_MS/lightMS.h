#ifndef LIGHT_MS_H
#define LIGHT_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{

	u32 VAL;
}lightMS_MEAS;

extern osThreadId 	 tid_lightMS_Thread;
extern osPoolId  	 lightMS_pool;	
extern osMessageQId  MsgBox_lightMS;
extern osMessageQId  MsgBox_MTlightMS;
extern osMessageQId  MsgBox_DPlightMS;

void lightMS_Init(void);
void lightMS_Thread(const void *argument);
void lightMSThread_Active(void);

#endif

