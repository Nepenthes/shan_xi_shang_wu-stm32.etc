#ifndef PYRO_MS_H
#define PYRO_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{

	bool VAL;
}pyroMS_MEAS;

extern osThreadId 	 tid_pyroMS_Thread;
extern osPoolId  	 pyroMS_pool;
extern osMessageQId  MsgBox_pyroMS;
extern osMessageQId  MsgBox_MTpyroMS;
extern osMessageQId  MsgBox_DPpyroMS;

void pyroMS_Init(void);
void pyroMS_Thread(const void *argument);
void pyroMSThread_Active(void);

#endif

