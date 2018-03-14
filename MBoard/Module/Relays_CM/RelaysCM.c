#include "RelaysCM.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_RelaysCM_Thread;
osThreadDef(RelaysCM_Thread,osPriorityNormal,1,512);
			 
osPoolId  RelaysCM_pool;								 
osPoolDef(RelaysCM_pool, 10, RelaysCM_MEAS);                  // 内存池定义
osMessageQId  MsgBox_RelaysCM;
osMessageQDef(MsgBox_RelaysCM, 2, &RelaysCM_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTRelaysCM;
osMessageQDef(MsgBox_MTRelaysCM, 2, &RelaysCM_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPRelaysCM;
osMessageQDef(MsgBox_DPRelaysCM, 2, &RelaysCM_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void RelaysCM_Init(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	PAout(0) = PAout(1) = 0;
}

void RelaysCM_Thread(const void *argument){

	osEvent  evt;
    osStatus status;	
	
	u8 UPLD_cnt;
	const u8 UPLD_period = 5;

	RelaysCM_MEAS actuatorData;	//本地输入量
	static RelaysCM_MEAS  Data_temp   = {0};	//下行数据输入量同步对比缓存
	static RelaysCM_MEAS  Data_tempDP = {0};	//本地输入量显示数据对比缓存
	
	RelaysCM_MEAS *mptr = NULL;
	RelaysCM_MEAS *rptr = NULL;
	
	for(;;){
	
		evt = osMessageGet( MsgBox_MTRelaysCM, 100);
		if (evt.status == osEventMessage){		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			
			actuatorData.relay_con = rptr->relay_con;

			do{status = osPoolFree(RelaysCM_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		if(Data_temp.relay_con != actuatorData.relay_con){
		
			Data_temp.relay_con = actuatorData.relay_con;
			
			PAout(0) = (Data_temp.relay_con >> 0) & 0x01;
			PAout(1) = (Data_temp.relay_con >> 1) & 0x01;
		}
	
		if(Data_tempDP.relay_con != actuatorData.relay_con){
		
			Data_tempDP.relay_con = actuatorData.relay_con;
			
			do{mptr = (RelaysCM_MEAS *)osPoolCAlloc(RelaysCM_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
			mptr->relay_con = actuatorData.relay_con;
			osMessagePut(MsgBox_DPRelaysCM, (uint32_t)mptr, 100);
			osDelay(10);
		}

		if(UPLD_cnt < UPLD_period)UPLD_cnt ++;	//数据定时上传
		else{
		
			UPLD_cnt = 0;
			
			do{mptr = (RelaysCM_MEAS *)osPoolCAlloc(RelaysCM_pool);}while(mptr == NULL);
			mptr->relay_con = actuatorData.relay_con;
			osMessagePut(MsgBox_RelaysCM, (uint32_t)mptr, 100);
			osDelay(10);
		}
	}
}

void RelaysCMThread_Active(void){

	RelaysCM_Init();
	RelaysCM_pool   = osPoolCreate(osPool(RelaysCM_pool));	//创建内存池
	MsgBox_RelaysCM 	= osMessageCreate(osMessageQ(MsgBox_RelaysCM), NULL);	//创建消息队列
	MsgBox_MTRelaysCM = osMessageCreate(osMessageQ(MsgBox_MTRelaysCM), NULL);//创建消息队列
	MsgBox_DPRelaysCM = osMessageCreate(osMessageQ(MsgBox_DPRelaysCM), NULL);//创建消息队列
	tid_RelaysCM_Thread = osThreadCreate(osThread(RelaysCM_Thread),NULL);
}
