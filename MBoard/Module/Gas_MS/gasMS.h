#ifndef GAS_MS_H
#define GAS_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

#define GAS_DATA	PAin(0);

typedef struct{
	
	bool	 	VAL;
	uint16_t anaDAT;	
}gasMS_MEAS;

extern osThreadId 	 tid_gasMS_Thread;
extern osPoolId  	 gasMS_pool;
extern osMessageQId  MsgBox_gasMS;
extern osMessageQId  MsgBox_MTgasMS;
extern osMessageQId  MsgBox_DPgasMS;

void gasMS_Init(void);
void gasMS_Thread(const void *argument);
void gasMSThread_Active(void);

#endif

