#ifndef DATATRANS_USART_H
#define DATATRANS_USART_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include <dataTrans_USART.h>
#include "Driver_USART.h"
#include "string.h"

#include <Key&Tips.h>
#include "Moudle_DEC.h"

#include "Eguard.h"
#include "fireMS.h"
#include "gasMS.h"
#include "lightMS.h"
#include "pyroMS.h"
#include "analogMS.h"
#include "smokeMS.h"
#include "tempMS.h"

#include "infraTrans.h"

#define  WIRLESS_THREAD_EN	123

#define  datsTransCMD_UPLOAD	0x10
#define  datsTransCMD_DOWNLOAD	0x20

#define	 ABNORMAL_DAT	0xfe

extern osThreadId tid_USARTWireless_Thread;
	
void myUSART2_callback(uint32_t event);
void USART2Wirless_Init(void);
void USART_WirelessInit(void);
void USARTWireless_Thread(const void *argument);

void wirelessThread_Active(void);

#endif
