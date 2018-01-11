#ifndef FIRE_MS_H
#define FIRE_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

#define FIRE_DATA	PAin(0);

typedef struct{

	bool VAL;
}fireMS_MEAS;

extern osThreadId 	 tid_fireMS_Thread;
extern osPoolId  	 fireMS_pool;
extern osMessageQId  MsgBox_fireMS;
extern osMessageQId  MsgBox_MTfireMS;
extern osMessageQId  MsgBox_DPfireMS;

void fireMS_Init(void);
void fireMS_Thread(const void *argument);
void fireMSThread_Active(void);

#endif

