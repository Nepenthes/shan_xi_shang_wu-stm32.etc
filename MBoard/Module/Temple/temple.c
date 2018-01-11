#include "templeMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_templeMS_Thread;
osThreadDef(templeMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  templeMS_pool;								 
osPoolDef(templeMS_pool, 10, templeMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_templeMS;
osMessageQDef(MsgBox_templeMS, 2, &templeMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTtempleMS;
osMessageQDef(MsgBox_MTtempleMS, 2, &templeMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPtempleMS;
osMessageQDef(MsgBox_DPtempleMS, 2, &templeMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void templeMS_Init(void){

	;
}

void templeMS_Thread(const void *argument){

	;
}

void templeMSThread_Active(void){

	templeMS_Init();
	templeMS_pool   = osPoolCreate(osPool(templeMS_pool));	//创建内存池
	MsgBox_templeMS 	= osMessageCreate(osMessageQ(MsgBox_templeMS), NULL);	//创建消息队列
	MsgBox_MTtempleMS = osMessageCreate(osMessageQ(MsgBox_MTtempleMS), NULL);//创建消息队列
	MsgBox_DPtempleMS = osMessageCreate(osMessageQ(MsgBox_DPtempleMS), NULL);//创建消息队列
	tid_templeMS_Thread = osThreadCreate(osThread(templeMS_Thread),NULL);
}
