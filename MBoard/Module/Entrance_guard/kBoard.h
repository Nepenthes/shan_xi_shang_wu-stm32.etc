#ifndef _KBOARD_H_
#define _KBOARD_H_

#include "stm32f10x.h"
#include "Eguard.h"

#define valKB_NULL	255

#define	PSD_EXERES_TTIT	0xEE

#define	PSD_EXERES_LVMSG_DN	0xE0
#define	PSD_EXERES_LVMSG_UP	0xE1

#define	PSD_EXERES_CALL	0xE3

#define ROW1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6)//读取
#define ROW2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)//读取 
#define ROW3  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)//读取
#define ROW4  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10)//读取 

#define COL4  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)//读取
#define COL3  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)//读取 
#define COL2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7)//读取
#define COL1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6)//读取 

#define GPIO1_KB PCout(6)
#define GPIO2_KB PBout(8)
#define GPIO3_KB PBout(9)
#define GPIO4_KB PBout(10)

#define GPIO5_KB PBout(11)
#define GPIO6_KB PCout(7)
#define GPIO7_KB PBout(7)
#define GPIO8_KB PBout(6)

#define K_VAL0		5
#define K_VAL1		4
#define K_VAL2		8
#define K_VAL3		12
#define K_VAL4		3
#define K_VAL5		7
#define K_VAL6		11
#define K_VAL7		2
#define K_VAL8		6
#define K_VAL9		10

#define K_FUN_CLR	1
#define K_FUN_ENT	13
#define K_FUN_ESC	16
#define K_FUN_PGUP	15
#define K_FUN_PGDN	14

#define K_FUN_PGUP_LONG	0xD1	//上翻键长按

#define K_FUN_LONG_relase	0xFE	//长按释放（用于所有长按）

/*********************键值二次矫正**********************/

#define KBFUN_CLR	10
#define KBFUN_ENT	11
#define KBFUN_ESC	12
#define KBFUN_PGUP	13
#define KBFUN_PGDN	14

uint8_t valKB_get(void);

void kBoard_Thread(const void *argument);
void kBoardThread_Active(void);

#endif

