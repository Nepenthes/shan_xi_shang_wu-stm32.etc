#ifndef	_MOUDLE_DEC_H_
#define	_MOUDLE_DEC_H_

#include "IO_Map.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"
#include "delay.h"

#include "Tips.h"

#define		extension_FLG	0x80
#define		wireless_FLG	0x40
#define		LCD4_3_FLG		0x20

#define		extension_IF	PEin(7)	
#define		wireless_IF		(((GPIO_ReadInputData(GPIOD) >> 3) & 0x1f) == 0x1f)   //无线模块特殊检测
#define		LCD4_3_IF		PEin(4)	

typedef struct M_attr{

	u8 Extension_ID;
	u8 Wirless_ID;
	
	u8 Alive;	//bit7:扩展模块,bit6:无线通信模块,bit5:显示模块
}Moudle_attr;

extern Moudle_attr Moudle_GTA;

void MoudleDEC_Init(void);

void MBDEC_Thread(const void *argument);

#endif
