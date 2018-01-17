#ifndef	_INFRATRANS_H_
#define	_INFRATRANS_H_

#include "IO_Map.h"
#include "stm32f10x.h"

#include "stmflash.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"
#include "delay.h"

#include "Tips.h"

#include <Key&Tips.h>

#define IFRLRN_STATUS	PBout(13)
#define RDATA 			PBin(14)
#define REMOTE_ID 		0

#define Tab_size 	255	//信号采样表

#define kifrSTATUS_NONLRN	100
#define kifrSTATUS_WAITK	101
#define kifrSTATUS_WAITSG	102
#define kifrSTATUS_LRNOVR	103
#define kifrSTATUS_SGOUT	104

#define ifrvalK_NULL		0

#define  ifrDevMID_video	0x0B
#define  ifrDevMID_audio	0x0A

#define  SPECMD_ifrDevModADDR_CHG		0x0A
#define  SPECMD_ifrDevDATS_CHG			0x0B

extern bool measure_en;
extern uint8_t tabHp,tabLp;
extern uint16_t HTtab[Tab_size];
extern uint16_t LTtab[Tab_size];

typedef struct{

	u8 speDPCMD;	//特殊送显命令
	
	u8 Mod_addr;
	
	u8 VAL_KEY;
	u8 STATUS;
}IFR_MEAS;

typedef struct{

	uint8_t mADDR;
}IFR_kMSG;

extern osThreadId 	 tid_keyIFR_Thread;
extern osPoolId  	 IFR_pool;	
extern osMessageQId  MsgBox_IFR;
extern osMessageQId  MsgBox_MTIFR;
extern osMessageQId  MsgBox_DPIFR;

extern osPoolId  	 memID_IFRsigK_pool;	
extern osMessageQId  MsgBox_IFRsigK;

u16 HW_ReceiveTime(void);
u16 LW_ReceiveTime(void);

void keyIFR_Thread_umdScan(const void *argument);
void keyIFR_Thread(const void *argument);
void keyIFR_Terminate(void);
void keyIFRActive(void);

#endif

