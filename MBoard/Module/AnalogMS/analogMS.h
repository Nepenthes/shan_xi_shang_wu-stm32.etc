#ifndef ANALOG_MS_H
#define ANALOG_MS_H

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

typedef struct{

	uint16_t Ich1;
	uint16_t Ich2;
	uint16_t Vch1;
	uint16_t Vch2;
}analogMS_MEAS;

extern osThreadId 	 tid_analogMS_Thread;
extern osPoolId  	 analogMS_pool;
extern osMessageQId  MsgBox_analogMS;
extern osMessageQId  MsgBox_MTanalogMS;
extern osMessageQId  MsgBox_DPanalogMS;

void analogMS_Init(void);
void analogMS_Thread(const void *argument);
void analogMSThread_Active(void);

#endif

