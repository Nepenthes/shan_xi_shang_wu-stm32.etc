#ifndef TEST_H
#define TEST_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"

#define LED1_0		GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_SET)
#define LED1_1		GPIO_WriteBit(GPIOC, GPIO_Pin_3, Bit_RESET)
#define LED2_0		GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_SET)
#define LED2_1		GPIO_WriteBit(GPIOC, GPIO_Pin_4, Bit_RESET)

void testInit (void);
void LEDTest  (void);

#endif
