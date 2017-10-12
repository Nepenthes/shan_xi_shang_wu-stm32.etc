#ifndef KEY$TIPS_H
#define KEY$TIPS_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "stdint.h"
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"

#define KEY_DEBUG		1		//是否开启按键调试（串口1反馈调试信息）

#define K1	GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_5)		//按键1监测
#define K2	GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_6)		//按键2监测

#define KEY_TICK						10			 // 按键节拍 / ms

#define KEY_COMFIRM					1			 // 按键长按后继续保持确认时长 / ms*KEY_TICK

#define KEY_LONG_PERIOD				200		 // 长按时长定义 / ms*KEY_TICK 二进制8位 最大值254
#define KEY_KEEP_PERIOD				50			 // 长按保持间隔 / ms*KEY_TICK
#define KEY_CONTINUE_PERIOD		40		*1000000		// 连按间隔 / us

#define KEY_VALUE_1	0x0010		//按键键值1
#define KEY_VALUE_2	0x0020		//按键键值2

#define KEY_DOWN		0x1000		//按键状态：按下
#define KEY_CONTINUE	0x2000		//按键状态：按键连按
#define KEY_LONG		0x3000		//按键状态：按键长按
#define KEY_KEEP		0x4000		//按键状态：按键长按后保持
#define KEY_UP			0x5000		//按键状态：按键弹起
#define KEY_NULL		0x6000		//按键状态：无按键事件

#define KEY_STATE_INIT		0x0100	//检测状态机状态：初始化
#define KEY_STATE_WOBBLE	0x0200	//检测状态机状态：消抖检测
#define KEY_STATE_PRESS		0x0300	//检测状态机状态：按键短按检测
#define KEY_STATE_CONTINUE	0x0400	//检测状态机状态：按键连按检测	作废，用KEY_STATE_RECORD替代
#define KEY_STATE_LONG		0x0500	//检测状态机状态：按键长按检测
#define KEY_STATE_KEEP		0x0600	//检测状态机状态：按键长按后保持检测
#define KEY_STATE_RELEASE	0x0700	//检测状态机状态：按键释放检测

#define KEY_STATE_RECORD	0x0800	//检测状态机状态：按键记录及连按确认检测

void keyInit(void);
void keyTest_Thread(const void *argument);
void keyTest(void);

#endif
