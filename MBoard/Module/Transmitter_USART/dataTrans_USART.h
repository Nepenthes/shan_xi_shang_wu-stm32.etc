#ifndef DATATRANS_USART_H
#define DATATRANS_USART_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include <dataTrans_USART.h>
#include "Driver_USART.h"
#include "string.h"

#include <Key&Tips.h>

typedef void (*funDebug)(void);

void funDB_keyMB_ON(void);
void funDB_keyMB_OFF(void);
void funDB_keyIFR_ON(void);
void funDB_keyIFR_OFF(void);
	
void myUSART1_callback(uint32_t event);
void myUSART2_callback(uint32_t event);

void USART1Debug_Init(void);
void USART2Wirless_Init(void);

void USARTDebug_Thread(const void *argument);
void USARTWireless_Thread(const void *argument);

void USART_allInit(void);
void USARTthread_Active(void);

#endif
