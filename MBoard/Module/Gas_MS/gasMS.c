#include "gasMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_gasMS_Thread;
osThreadDef(gasMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  gasMS_pool;								 
osPoolDef(gasMS_pool, 10, gasMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_gasMS;
osMessageQDef(MsgBox_gasMS, 2, &gasMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTgasMS;
osMessageQDef(MsgBox_MTgasMS, 2, &gasMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPgasMS;
osMessageQDef(MsgBox_DPgasMS, 2, &gasMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void gasMS_Init(void){

	;
}

void gasMS_Thread(const void *argument){

	;
}

void gasMSThread_Active(void){

	gasMS_Init();
	gasMS_pool   = osPoolCreate(osPool(gasMS_pool));	//创建内存池
	MsgBox_gasMS 	= osMessageCreate(osMessageQ(MsgBox_gasMS), NULL);	//创建消息队列
	MsgBox_MTgasMS = osMessageCreate(osMessageQ(MsgBox_MTgasMS), NULL);//创建消息队列
	MsgBox_DPgasMS = osMessageCreate(osMessageQ(MsgBox_DPgasMS), NULL);//创建消息队列
	tid_gasMS_Thread = osThreadCreate(osThread(gasMS_Thread),NULL);
}
