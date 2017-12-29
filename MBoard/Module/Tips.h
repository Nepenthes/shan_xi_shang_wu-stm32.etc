#ifndef _TIPS_H__
#define _TIPS_H__

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"

#include "IO_Map.h"
#include "delay.h"

#define LED_MSGZigbee_OK	PEout(3)
#define LED_EXECIfr_OK		PBout(13)

#define LED_SYS			PEout(13)
#define LED_MSG			PEout(14)
#define LED_EXT			PEout(15)
#define LED_MSG_N		PEout(3)
#define LED_EXT_N		PEout(9)

#define OBJ_SYS			1
#define OBJ_MSG			2
#define OBJ_EXT			3

#define EVTSIG_SYS_A		0x0000001A
#define EVTSIG_MSG_A		0x0000001B
#define EVTSIG_EXT_A		0x0000001C

#define EVTSIG_SYS_B		0x0000000A
#define EVTSIG_MSG_B		0x0000000B
#define EVTSIG_EXT_B		0x0000000C

extern osThreadId tid_tips;

void tipsLEDActive(void);
void tipsInit(void);
void tipsBoardActive(void);
void tipsThread(void const *argument);

void tips_beep(u8 tones, u16 time, u8 vol);

void TTask0(void const *argument);

#endif
