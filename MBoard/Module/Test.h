#ifndef TEST_H
#define TEST_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"

#define LED1_0		GPIO_WriteBit(GPIOE, GPIO_Pin_13, Bit_SET)
#define LED1_1		GPIO_WriteBit(GPIOE, GPIO_Pin_13, Bit_RESET)
#define LED2_0		GPIO_WriteBit(GPIOE, GPIO_Pin_14, Bit_SET)
#define LED2_1		GPIO_WriteBit(GPIOE, GPIO_Pin_14, Bit_RESET)
#define LED3_0		GPIO_WriteBit(GPIOE, GPIO_Pin_15, Bit_SET)
#define LED3_1		GPIO_WriteBit(GPIOE, GPIO_Pin_15, Bit_RESET)

void testInit (void);
void LEDTest  (void);

#endif
