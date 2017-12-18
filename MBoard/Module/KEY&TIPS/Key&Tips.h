#ifndef KEY$TIPS_H
#define KEY$TIPS_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "stdint.h"
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"

#define KEY_DEBUG_ON		0x00000004		//开启按键调试信息输出信号（串口1反馈调试信息）
#define KEY_DEBUG_OFF	0x00000005		//关闭按键调试信息输出信号（串口1反馈调试信息）

#define K1	GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_11)		//按键1监测
#define K2	GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_12)		//按键2监测

#define KEY_TICK						20			 // 按键节拍 / (ms)

#define KEY_COMFIRM					3			 // 按键长按后继续保持多久开始计数，确认时长 / 乘以KEY_LONG_PERIOD * KEY_TICK(ms)

#define KEY_LONG_PERIOD				150		 // 长按时长定义 / 乘以KEY_TICK(ms) 二进制8位 最大值254
#define KEY_KEEP_PERIOD				40			 // 长按保持间隔 / 乘以KEY_TICK(ms)
#define KEY_CONTINUE_PERIOD		40		*1000000		// 连按间隔 / (us)

#define KEY_VALUE_0	0x0000		//按键键值0
#define KEY_VALUE_1	0x0010		//按键键值1
#define KEY_VALUE_2	0x0020		//按键键值2
#define KEY_VALUE_3	0x0030		//按键键值3
#define KEY_VALUE_4	0x0040		//按键键值4
#define KEY_VALUE_5	0x0050		//按键键值5
#define KEY_VALUE_6	0x0060		//按键键值6
#define KEY_VALUE_7	0x0070		//按键键值7
#define KEY_VALUE_8	0x0080		//按键键值8
#define KEY_VALUE_9	0x0090		//按键键值9
#define KEY_VALUE_10	0x00A0		//按键键值10
#define KEY_VALUE_11	0x00B0		//按键键值11
#define KEY_VALUE_12	0x00C0		//按键键值12
#define KEY_VALUE_13	0x00D0		//按键键值13
#define KEY_VALUE_14	0x00E0		//按键键值14
#define KEY_VALUE_15	0x00F0		//按键键值15

#define KEY_DOWN		0x1000		//按键状态：按下
#define KEY_CONTINUE	0x2000		//按键状态：按键连按
#define KEY_LONG		0x3000		//按键状态：按键长按
#define KEY_KEEP		0x4000		//按键状态：按键长按后保持
#define KEY_UP			0x5000		//按键状态：按键弹起
#define KEY_NULL		0x6000		//按键状态：无按键事件
#define KEY_CTOVER	0x7000		//按键状态：连按结束

#define KEY_STATE_INIT		0x0100	//检测状态机状态：初始化
#define KEY_STATE_WOBBLE	0x0200	//检测状态机状态：消抖检测
#define KEY_STATE_PRESS		0x0300	//检测状态机状态：按键短按检测
#define KEY_STATE_CONTINUE	0x0400	//检测状态机状态：按键连按检测	作废，用KEY_STATE_RECORD替代
#define KEY_STATE_LONG		0x0500	//检测状态机状态：按键长按检测
#define KEY_STATE_KEEP		0x0600	//检测状态机状态：按键长按后保持检测
#define KEY_STATE_RELEASE	0x0700	//检测状态机状态：按键释放检测

#define KEY_STATE_RECORD	0x0800	//检测状态机状态：按键记录及连按确认检测

#define KEY_OVER_SHORT		0x01
#define KEY_OVER_LONG		0x02
#define KEY_OVER_KEEP		0x03

typedef void (* funEventKey)(void);

typedef void (* funKeyInit)(void);
typedef uint16_t (*funKeyScan)(void);

typedef struct keyStatus{

	uint8_t 	keyOverFlg;			//按键事件结束标志
	uint16_t	sKeyCount;			//连续短按	 计数值
	uint16_t	sKeyKeep;			//长按并保持 计数值
	uint8_t	keyCTflg;			//按键连按标志
}Obj_keyStatus;

typedef struct keyEvent{		//创建一个按键检测线程对象封装

	funKeyInit keyInit;			//按键引脚初始化
	funKeyScan keyScan;			//按键键值及状态扫描
	/*想要哪个事件就创建对应函数进行赋值即可*/
	funEventKey funKeyLONG[16];	//长按事件
	funEventKey funKeySHORT[16];	//短按事件
	funEventKey funKeyCONTINUE[16][8];	//长按保持事件
	funEventKey funKeyKEEP[16][8];	//连按事件
}Obj_eventKey;

void key_Thread(funKeyInit key_Init,Obj_keyStatus *orgKeyStatus,funKeyScan key_Scan,Obj_eventKey keyEvent,const char *Tips_head);

void keyInit(void);
void keyMboard_Thread(const void *argument);
void keyMboardActive(void);

#endif
