#include "simuMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_simuMS_Thread;
osThreadDef(simuMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  simuMS_pool;								 
osPoolDef(simuMS_pool, 10, simuMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_simuMS;
osMessageQDef(MsgBox_simuMS, 2, &simuMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTsimuMS;
osMessageQDef(MsgBox_MTsimuMS, 2, &simuMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPsimuMS;
osMessageQDef(MsgBox_DPsimuMS, 2, &simuMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void simuMS_Init(void){

	;
}

void simuMS_Thread(const void *argument){

	;
}

void simuMSThread_Active(void){

	simuMS_Init();
	simuMS_pool   = osPoolCreate(osPool(simuMS_pool));	//创建内存池
	MsgBox_simuMS 	= osMessageCreate(osMessageQ(MsgBox_simuMS), NULL);	//创建消息队列
	MsgBox_MTsimuMS = osMessageCreate(osMessageQ(MsgBox_MTsimuMS), NULL);//创建消息队列
	MsgBox_DPsimuMS = osMessageCreate(osMessageQ(MsgBox_DPsimuMS), NULL);//创建消息队列
	tid_simuMS_Thread = osThreadCreate(osThread(simuMS_Thread),NULL);
}
