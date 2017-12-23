#include "tempMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_tempMS_Thread;
osThreadDef(tempMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  tempMS_pool;								 
osPoolDef(tempMS_pool, 10, tempMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_tempMS;
osMessageQDef(MsgBox_tempMS, 2, &tempMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTtempMS;
osMessageQDef(MsgBox_MTtempMS, 2, &tempMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPtempMS;
osMessageQDef(MsgBox_DPtempMS, 2, &tempMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void tempMS_Init(void){

	;
}

void tempMS_Thread(const void *argument){

	;
}

void tempMSThread_Active(void){

	tempMS_Init();
	tempMS_pool   = osPoolCreate(osPool(tempMS_pool));	//创建内存池
	MsgBox_tempMS 	= osMessageCreate(osMessageQ(MsgBox_tempMS), NULL);	//创建消息队列
	MsgBox_MTtempMS = osMessageCreate(osMessageQ(MsgBox_MTtempMS), NULL);//创建消息队列
	MsgBox_DPtempMS = osMessageCreate(osMessageQ(MsgBox_DPtempMS), NULL);//创建消息队列
	tid_tempMS_Thread = osThreadCreate(osThread(tempMS_Thread),NULL);
}
