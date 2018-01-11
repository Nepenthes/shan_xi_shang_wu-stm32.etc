#ifndef SOURCE_CM_H
#define SOURCE_CM_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

#define soceRELAY	PAout(0)

typedef struct{

	bool Switch;
	uint16_t anaVal;
}sourceCM_MEAS;

extern osThreadId 	 tid_sourceCM_Thread;
extern osPoolId  	 sourceCM_pool;
extern osMessageQId  MsgBox_sourceCM;
extern osMessageQId  MsgBox_MTsourceCM;
extern osMessageQId  MsgBox_DPsourceCM;

void sourceCM_Init(void);
void sourceCM_Thread(const void *argument);
void sourceCMThread_Active(void);

#endif

