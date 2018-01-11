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

pyroDIO_Init(void){

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);		
}

void pyroMS_Init(void){

	pyroDIO_Init();
}

void pyroMS_Thread(const void *argument){
	
	osEvent  evt;
    osStatus status;	
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 40;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	pyroMS_MEAS	sensorData;
	static pyroMS_MEAS Data_temp = {1};
	
	pyroMS_MEAS *mptr = NULL;
	fireMS_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTpyroMS, 100);
		if (evt.status == osEventMessage) {		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			

			do{status = osPoolFree(pyroMS_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		sensorData.VAL = PYRO_DATA;	//数据采集
		
		if(Data_temp.VAL != sensorData.VAL){	//数据推送（数据更替时才触发）
		
			Data_temp.VAL = sensorData.VAL;
			
			do{mptr = (pyroMS_MEAS *)osPoolCAlloc(pyroMS_pool);}while(mptr == NULL);
			mptr->VAL = sensorData.VAL;
			osMessagePut(MsgBox_pyroMS, (uint32_t)mptr, 100);
			
			do{mptr = (pyroMS_MEAS *)osPoolCAlloc(pyroMS_pool);}while(mptr == NULL);
			mptr->VAL = sensorData.VAL;
			osMessagePut(MsgBox_DPpyroMS, (uint32_t)mptr, 100);
			osDelay(10);
		}
		
		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
		else{
		
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"\n\ris anybody here now? : %d\n\r", !sensorData.VAL);			
			Driver_USART1.Send(disp,strlen(disp));
			osDelay(20);
		}
		
		osDelay(10);
	}
}

void pyroMSThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		pyroMS_pool   = osPoolCreate(osPool(pyroMS_pool));	//创建内存池
		MsgBox_pyroMS 	= osMessageCreate(osMessageQ(MsgBox_pyroMS), NULL);   //创建消息队列
		MsgBox_MTpyroMS = osMessageCreate(osMessageQ(MsgBox_MTpyroMS), NULL);//创建消息队列
		MsgBox_DPpyroMS = osMessageCreate(osMessageQ(MsgBox_DPpyroMS), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	pyroMS_Init();
	tid_pyroMS_Thread = osThreadCreate(osThread(pyroMS_Thread),NULL);
}
