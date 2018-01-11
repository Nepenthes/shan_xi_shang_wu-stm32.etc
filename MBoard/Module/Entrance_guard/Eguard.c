#include "Eguard.h"
#include "debugUart.h"

extern ARM_DRIVER_USART Driver_USART2;

extern osThreadId tid_fingerID_Thread;
extern osThreadId tid_rfID_Thread;
extern osThreadId tid_kBoard_Thread;
					 
osPoolId  EGUD_pool;								 
osPoolDef(EGUD_pool, 10, EGUARD_MEAS);                    // 内存池定义
osMessageQId  MsgBox_EGUD_FID;		
osMessageQDef(MsgBox_EGUD_FID, 3, &EGUARD_MEAS);             // 消息队列定义
osMessageQId  MsgBox_MTEGUD;		
osMessageQDef(MsgBox_MTEGUD, 3, &EGUARD_MEAS);             // 消息队列定义
osMessageQId  MsgBox_DPEGUD;		
osMessageQDef(MsgBox_DPEGUD, 3, &EGUARD_MEAS);             // 消息队列定义

void Eguard_Active(void){
	
	static bool memInit_flg = false;
	
	if(!memInit_flg){

		EGUD_pool = osPoolCreate(osPool(EGUD_pool));	//创建内存池
		MsgBox_EGUD_FID = osMessageCreate(osMessageQ(MsgBox_EGUD_FID), NULL);	//创建消息队列
		MsgBox_MTEGUD = osMessageCreate(osMessageQ(MsgBox_MTEGUD), NULL);	//创建消息队列
		MsgBox_DPEGUD = osMessageCreate(osMessageQ(MsgBox_DPEGUD), NULL);	//创建消息队列
		memInit_flg = true;
	}
	
	//fingerIDThread_Active();
	//rfIDThread_Active();
	kBoardThread_Active();
}

void Eguard_Terminate(void){

	osThreadTerminate(tid_fingerID_Thread);
	osThreadTerminate(tid_rfID_Thread);
	osThreadTerminate(tid_kBoard_Thread);
}

