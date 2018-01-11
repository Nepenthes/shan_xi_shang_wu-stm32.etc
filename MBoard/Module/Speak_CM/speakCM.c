#include "speakCM.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_speakCM_Thread;
osThreadDef(speakCM_Thread,osPriorityNormal,1,512);
			 
osPoolId  speakCM_pool;								 
osPoolDef(speakCM_pool, 10, speakCM_MEAS);                  // 内存池定义
osMessageQId  MsgBox_speakCM;
osMessageQDef(MsgBox_speakCM, 2, &speakCM_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTspeakCM;
osMessageQDef(MsgBox_MTspeakCM, 2, &speakCM_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPspeakCM;
osMessageQDef(MsgBox_DPspeakCM, 2, &speakCM_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void speakCM_Init(void){

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_12;	 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;	 
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5	
}

void NVCdat_in(uint8_t dat){

	uint8_t i;
	
	PA1 = 0;
	PA0 = 1;
	
	osDelay(5); /*5ms */
	for(i=0;i<8;i++){
		
		PA1 = 1;
		if(dat & 1){
			
			delay_us(1200); /* 1200us */
			PA1 = 0;
			delay_us(400); /* 400us */
		}else{
			
			delay_us(400);
			PA1 = 0;
			delay_us(1200);
		}
		dat >>= 1;
	}
	PA1 = 1;
	osDelay(50);
}

void SPK_Select(uint8_t num,uint8_t vol){

	if(num > 14)return;
	
	NVCdat_in(0xe0 + vol);
	//NVCdat_in(0xf2);	//循环播放使能
	NVCdat_in(num);
}

void speakCM_Thread(const void *argument){

	osEvent  evt;
    osStatus status;	

	speakCM_MEAS actuatorData;	//本地输入量
	static speakCM_MEAS  Data_temp   = {1};	//下行数据输入量同步对比缓存
	static speakCM_MEAS  Data_tempDP = {1};	//本地输入量显示数据对比缓存
	
	speakCM_MEAS *mptr = NULL;
	speakCM_MEAS *rptr = NULL;
	
	SPK_EN = 1;
	for(;;){
	
		
		evt = osMessageGet(MsgBox_MTspeakCM, 100);
		if (evt.status == osEventMessage){		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			
			actuatorData.spk_num = rptr->spk_num;

			do{status = osPoolFree(speakCM_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		if(Data_temp.spk_num != actuatorData.spk_num){
		
			Data_temp.spk_num = actuatorData.spk_num;
			SPK_Select(actuatorData.spk_num,6);
		}
	
		if(Data_tempDP.spk_num != actuatorData.spk_num){
		
			Data_tempDP.spk_num = actuatorData.spk_num;
			
			do{mptr = (speakCM_MEAS *)osPoolCAlloc(speakCM_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
			mptr->spk_num = actuatorData.spk_num;
			osMessagePut(MsgBox_DPspeakCM, (uint32_t)mptr, 100);
			osDelay(10);
		}
			
//		for(temp = 0;temp < 14;temp ++){		/**测试语句**/
//		
//			SPK_Select(temp,6);
//			osDelay(3000);
//			SPK_STP;
//		}
	}
}

void speakCMThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		speakCM_pool   = osPoolCreate(osPool(speakCM_pool));	//创建内存池
		MsgBox_speakCM 	= osMessageCreate(osMessageQ(MsgBox_speakCM), NULL);   //创建消息队列
		MsgBox_MTspeakCM = osMessageCreate(osMessageQ(MsgBox_MTspeakCM), NULL);//创建消息队列
		MsgBox_DPspeakCM = osMessageCreate(osMessageQ(MsgBox_DPspeakCM), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	speakCM_Init();
	tid_speakCM_Thread = osThreadCreate(osThread(speakCM_Thread),NULL);
}
