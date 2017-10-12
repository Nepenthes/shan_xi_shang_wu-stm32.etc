#ifndef WIRELESSTRABS_USART_H
#define WIRELESSTRABS_USART_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include <WirelessTrans_USART.h>
#include "Driver_USART.h"
#include "string.h"

void myUSART_callback(uint32_t event);
void USARTInit1(void);
void USARTInit2(void);
void USARTTest_Thread(const void *argument);
void USARTTest(void);

#endif
