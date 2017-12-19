#ifndef _TIPS_H__
#define _TIPS_H__

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"

#include "IO_Map.h"
#include "delay.h"

#define LED_SYS		PEout(13)
#define LED_MSG		PEout(14)
#define LED_EXT		PEout(15)

#define OBJ_SYS		1
#define OBJ_MSG		2
#define OBJ_EXT		3

#define EVTSIG_SYS		0x0000000A
#define EVTSIG_MSG		0x0000000B
#define EVTSIG_EXT		0x0000000C

extern osThreadId tid_tips;

void tipsLEDActive(void);
void tipsInit(void);
void tipsBoardActive(void);
void tipsThread(void const *argument);

void TTask0(void const *argument);

#endif
