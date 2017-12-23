#include "fireMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_fireMS_Thread;
osThreadDef(fireMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  fireMS_pool;								 
osPoolDef(fireMS_pool, 10, fireMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_fireMS;
osMessageQDef(MsgBox_fireMS, 2, &fireMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTfireMS;
osMessageQDef(MsgBox_MTfireMS, 2, &fireMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPfireMS;
osMessageQDef(MsgBox_DPfireMS, 2, &fireMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void fireMS_Init(void){

	;
}

void fireMS_Thread(const void *argument){

	;
}

void fireMSThread_Active(void){

	fireMS_Init();
	fireMS_pool   = osPoolCreate(osPool(fireMS_pool));	//创建内存池
	MsgBox_fireMS 	= osMessageCreate(osMessageQ(MsgBox_fireMS), NULL);	//创建消息队列
	MsgBox_MTfireMS = osMessageCreate(osMessageQ(MsgBox_MTfireMS), NULL);//创建消息队列
	MsgBox_DPfireMS = osMessageCreate(osMessageQ(MsgBox_DPfireMS), NULL);//创建消息队列
	tid_fireMS_Thread = osThreadCreate(osThread(fireMS_Thread),NULL);
}
