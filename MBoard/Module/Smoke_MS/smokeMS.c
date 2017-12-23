#include "smokeMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_smokeMS_Thread;
osThreadDef(smokeMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  smokeMS_pool;								 
osPoolDef(smokeMS_pool, 10, smokeMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_smokeMS;
osMessageQDef(MsgBox_smokeMS, 2, &smokeMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTsmokeMS;
osMessageQDef(MsgBox_MTsmokeMS, 2, &smokeMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPsmokeMS;
osMessageQDef(MsgBox_DPsmokeMS, 2, &smokeMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void smokeMS_Init(void){

	;
}

void smokeMS_Thread(const void *argument){

	;
}

void smokeMSThread_Active(void){

	smokeMS_Init();
	smokeMS_pool   = osPoolCreate(osPool(smokeMS_pool));	//创建内存池
	MsgBox_smokeMS 	= osMessageCreate(osMessageQ(MsgBox_smokeMS), NULL);	//创建消息队列
	MsgBox_MTsmokeMS = osMessageCreate(osMessageQ(MsgBox_MTsmokeMS), NULL);//创建消息队列
	MsgBox_DPsmokeMS = osMessageCreate(osMessageQ(MsgBox_DPsmokeMS), NULL);//创建消息队列
	tid_smokeMS_Thread = osThreadCreate(osThread(smokeMS_Thread),NULL);
}
