#ifndef _SPEAK_CM_H_
#define _SPEAK_CM_H_

#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "debugUart.h"

#define PB0		PBout(14)
#define PA0		PBout(15)
#define PA1		PAout(12)
#define SPK_EN	PAout(0)
#define SPK_STP	NVCdat_in(0xfe)

typedef struct{

	uint8_t spk_num;
}speakCM_MEAS;

extern osThreadId 	 tid_speakCM_Thread;
extern osPoolId  	 speakCM_pool;
extern osMessageQId  MsgBox_speakCM;
extern osMessageQId  MsgBox_MTspeakCM;
extern osMessageQId  MsgBox_DPspeakCM;

void speakCM_Init(void);
void speakCM_Thread(const void *argument);
void speakCMThread_Active(void);

#endif

