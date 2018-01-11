#ifndef	_EGUARD_H_
#define	_EGUARD_H_

#include "fingerID.h"
#include "rfID.h"
#include "kBoard.h"

#include "IO_Map.h"
#include "stm32f10x.h"

#include "Tips.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"
#include "delay.h"

typedef struct{

	u8 CMD;
	u8 DAT;
	u8 rfidDAT[4];
	u8 Psd[8];
}EGUARD_MEAS;

extern osPoolId  	 EGUD_pool;	
extern osMessageQId  MsgBox_EGUD_FID;							 
extern osMessageQId  MsgBox_MTEGUD;
extern osMessageQId  MsgBox_DPEGUD;

void Eguard_Active(void);
void Eguard_Terminate(void);

#endif

