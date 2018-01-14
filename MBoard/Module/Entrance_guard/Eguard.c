#include "Eguard.h"
#include "debugUart.h"

extern ARM_DRIVER_USART Driver_USART2;

extern osThreadId tid_fingerID_Thread;
extern osThreadId tid_rfID_Thread;
extern osThreadId tid_kBoard_Thread;
extern osThreadId tid_doorLock_Thread;
					 
osPoolId  EGUD_pool;								 
osPoolDef(EGUD_pool, 20, EGUARD_MEAS);                   // 内存池定义
osMessageQId  MsgBox_EGUD;		
osMessageQDef(MsgBox_EGUD, 5, &EGUARD_MEAS);             // 消息队列定义
osMessageQId  MsgBox_MTEGUD_FID;		
osMessageQDef(MsgBox_MTEGUD_FID, 5, &EGUARD_MEAS);       // 消息队列定义
osMessageQId  MsgBox_MTEGUD_DLOCK;		
osMessageQDef(MsgBox_MTEGUD_DLOCK, 5, &EGUARD_MEAS);       // 消息队列定义
osMessageQId  MsgBox_DPEGUD;		
osMessageQDef(MsgBox_DPEGUD, 10, &EGUARD_MEAS);          // 消息队列定义

void Eguard_Active(void){
	
	static bool memInit_flg = false;
	
	if(!memInit_flg){

		EGUD_pool 			= osPoolCreate(osPool(EGUD_pool));	//创建内存池
		MsgBox_EGUD 		= osMessageCreate(osMessageQ(MsgBox_EGUD), NULL);	//创建消息队列
		MsgBox_MTEGUD_FID 	= osMessageCreate(osMessageQ(MsgBox_MTEGUD_FID), NULL);	//创建消息队列
		MsgBox_MTEGUD_DLOCK = osMessageCreate(osMessageQ(MsgBox_MTEGUD_DLOCK), NULL);	//创建消息队列
		MsgBox_DPEGUD 		= osMessageCreate(osMessageQ(MsgBox_DPEGUD), NULL);	//创建消息队列
		memInit_flg = true;
	}
	
	fingerIDThread_Active();
	rfIDThread_Active();
	kBoardThread_Active();
	doorLockThread_Active();
}

void Eguard_Terminate(void){

	osThreadTerminate(tid_fingerID_Thread);
	osThreadTerminate(tid_rfID_Thread);
	osThreadTerminate(tid_kBoard_Thread);
	osThreadTerminate(tid_doorLock_Thread);
}

