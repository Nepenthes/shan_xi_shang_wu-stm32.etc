#include "lightMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_lightMS_Thread;
osThreadDef(lightMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  lightMS_pool;								 
osPoolDef(lightMS_pool, 10, lightMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_lightMS;
osMessageQDef(MsgBox_lightMS, 2, &lightMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTlightMS;
osMessageQDef(MsgBox_MTlightMS, 2, &lightMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPlightMS;
osMessageQDef(MsgBox_DPlightMS, 2, &lightMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void lightMS_Init(void){

	;
}

void lightMS_Thread(const void *argument){

	;
}

void lightMSThread_Active(void){

	lightMS_Init();
	lightMS_pool   = osPoolCreate(osPool(lightMS_pool));	//创建内存池
	MsgBox_lightMS 	= osMessageCreate(osMessageQ(MsgBox_lightMS), NULL);	//创建消息队列
	MsgBox_MTlightMS = osMessageCreate(osMessageQ(MsgBox_MTlightMS), NULL);//创建消息队列
	MsgBox_DPlightMS = osMessageCreate(osMessageQ(MsgBox_DPlightMS), NULL);//创建消息队列
	tid_lightMS_Thread = osThreadCreate(osThread(lightMS_Thread),NULL);
}
