#ifndef SIMU_MS_H
#define SIMU_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{
	
	u8  val[4];
}simuMS_MEAS;

extern osThreadId 	 tid_simuMS_Thread;
extern osPoolId  	 simuMS_pool;	
extern osMessageQId  MsgBox_simuMS;
extern osMessageQId  MsgBox_MTsimuMS;
extern osMessageQId  MsgBox_DPsimuMS;

void simuMS_Init(void);
void simuMS_Thread(const void *argument);
void simuMSThread_Active(void);

#endif

