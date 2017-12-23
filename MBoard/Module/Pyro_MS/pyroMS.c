#include "pyroMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_pyroMS_Thread;
osThreadDef(pyroMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  pyroMS_pool;								 
osPoolDef(pyroMS_pool, 10, pyroMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_pyroMS;
osMessageQDef(MsgBox_pyroMS, 2, &pyroMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTpyroMS;
osMessageQDef(MsgBox_MTpyroMS, 2, &pyroMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPpyroMS;
osMessageQDef(MsgBox_DPpyroMS, 2, &pyroMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void pyroMS_Init(void){

	;
}

void pyroMS_Thread(const void *argument){

	;
}

void pyroMSThread_Active(void){

	pyroMS_Init();
	pyroMS_pool   = osPoolCreate(osPool(pyroMS_pool));	//创建内存池
	MsgBox_pyroMS 	= osMessageCreate(osMessageQ(MsgBox_pyroMS), NULL);	//创建消息队列
	MsgBox_MTpyroMS = osMessageCreate(osMessageQ(MsgBox_MTpyroMS), NULL);//创建消息队列
	MsgBox_DPpyroMS = osMessageCreate(osMessageQ(MsgBox_DPpyroMS), NULL);//创建消息队列
	tid_pyroMS_Thread = osThreadCreate(osThread(pyroMS_Thread),NULL);
}
