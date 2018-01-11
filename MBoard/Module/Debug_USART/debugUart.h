#ifndef DEBUG_USART_H
#define DEBUG_USART_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include <dataTrans_USART.h>
#include "Driver_USART.h"
#include "string.h"

#include <Key&Tips.h>

#include "Eguard.h"

#define	USARTDEBUG_THREAD_EN	123

typedef void (*funDebug)(void);

void USART1Debug_Init(void);

extern osThreadId tid_USARTDebug_Thread;

void funDB_keyMB_ON(void);
void funDB_keyMB_OFF(void);
void funDB_keyIFR_ON(void);
void funDB_keyIFR_OFF(void);

void myUSART1_callback(uint32_t event);
void USART1Debug_Init(void);
void USART_debugInit(void);
void USARTDebug_Thread(const void *argument);

void debugThread_Active(void);

#endif



