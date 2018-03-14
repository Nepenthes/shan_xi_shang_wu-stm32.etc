#ifndef RELAYS_CM_H
#define RELAYS_CM_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{

	u8 relay_con;	//bit0 bit1 ÓÐÐ§
}RelaysCM_MEAS;

extern osThreadId 	 tid_RelaysCM_Thread;
extern osPoolId  	 RelaysCM_pool;
extern osMessageQId  MsgBox_RelaysCM;
extern osMessageQId  MsgBox_MTRelaysCM;
extern osMessageQId  MsgBox_DPRelaysCM;

void RelaysCM_Init(void);
void RelaysCM_Thread(const void *argument);
void RelaysCMThread_Active(void);

#endif

