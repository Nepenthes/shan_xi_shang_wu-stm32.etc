#ifndef SMOKE_MS_H
#define SMOKE_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{

	bool VAL;
}smokeMS_MEAS;

extern osThreadId 	 tid_smokeMS_Thread;
extern osPoolId  	 smokeMS_pool;
extern osMessageQId  MsgBox_smokeMS;
extern osMessageQId  MsgBox_MTsmokeMS;
extern osMessageQId  MsgBox_DPsmokeMS;

void smokeMS_Init(void);
void smokeMS_Thread(const void *argument);
void smokeMSThread_Active(void);

#endif

