#ifndef	_INFRATRANS_H_
#define	_INFRATRANS_H_

#include "IO_Map.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"
#include "delay.h"

#include <Key&Tips.h>

#define RDATA 		PBin(14)
#define REMOTE_ID 	0

typedef struct{

	u8 VAL_KEY;
	u8 STATUS;
}IFR_MEAS;

extern osThreadId 	 tid_keyIFR_Thread;

extern osPoolId  	 IFR_pool;	
extern osMessageQId  MsgBox_IFR;
extern osMessageQId  MsgBox_MTIFR;
extern osMessageQId  MsgBox_DPIFR;

uint16_t getKey2(Obj_keyStatus *orgKeyStatus,funKeyScan keyScan);
void key_Thread2(funKeyInit key_Init,Obj_keyStatus *orgKeyStatus,funKeyScan key_Scan,Obj_eventKey keyEvent,const char *Tips_head);

void keyIFR_Thread(const void *argument);
void keyIFRActive(void);

#endif

