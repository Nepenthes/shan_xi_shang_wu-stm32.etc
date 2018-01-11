#ifndef FIRE_MS_H
#define FIRE_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{

	bool VAL;
}templeMS_MEAS;

extern osThreadId 	 tid_templeMS_Thread;
extern osPoolId  	 templeMS_pool;
extern osMessageQId  MsgBox_templeMS;
extern osMessageQId  MsgBox_MTtempleMS;
extern osMessageQId  MsgBox_DPtempleMS;

void templeMS_Init(void);
void templeMS_Thread(const void *argument);
void templeMSThread_Active(void);

#endif

