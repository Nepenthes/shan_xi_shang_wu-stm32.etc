#include "pwmCM.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

static uint8_t pwmDevMOUDLE_ID;

static pwmCM_MEAS pwmDevAttr;
static pwmCM_MEAS pwmDevAttr_temp;
static pwmCM_MEAS pwmDevAttr_tempDP;

osThreadId tid_pwmCM_Thread;
osThreadDef(pwmCM_Thread,osPriorityNormal,1,256);

osThreadId tid_DC11detectA_Thread;
osThreadId tid_DC11detectB_Thread;
osThreadDef(DC11detectA_Thread,osPriorityNormal,1,256);
osThreadDef(DC11detectB_Thread,osPriorityNormal,1,256);
			 
osPoolId  pwmCM_pool;								 
osPoolDef(pwmCM_pool, 10, pwmCM_MEAS);                  // 内存池定义
osMessageQId  MsgBox_pwmCM;
osMessageQDef(MsgBox_pwmCM, 2, &pwmCM_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTpwmCM;
osMessageQDef(MsgBox_MTpwmCM, 2, &pwmCM_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPpwmCM;
osMessageQDef(MsgBox_DPpwmCM, 2, &pwmCM_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

osPoolId  memID_pwmCMsigK_pool;								 
osPoolDef(pwmCMsigK_pool, 4, pwmCM_kMSG);              // 按键消息内存池定义
osMessageQId  MsgBox_pwmCMsigK;
osMessageQDef(MsgBox_pwmCMsigK, 2, &pwmCM_kMSG);        // 消息队列定义，用于底板按键信号读取

void pwmCM_pwmInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; 
	TIM_OCInitTypeDef TIM_OCInitStructure; 
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); 
	
	TIM_TimeBaseStructure.TIM_Period =199; 
	TIM_TimeBaseStructure.TIM_Prescaler =359; 
	TIM_TimeBaseStructure.TIM_ClockDivision =0x00; 
	TIM_TimeBaseStructure.TIM_CounterMode =TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
	TIM_OCInitStructure.TIM_OutputState =TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	TIM_OCInitStructure.TIM_Pulse = 100; 
	TIM_OC2Init(TIM2, & TIM_OCInitStructure); 
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM2,ENABLE); 
	TIM_ITConfig(TIM2,TIM_IT_CC1,ENABLE); 
	TIM_CtrlPWMOutputs(TIM2,ENABLE); 
	TIM_Cmd(TIM2,ENABLE);
}

void pwmCM_ioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10| GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	PAout(1) = 0;
}

void pwmCM_Init(void){

	pwmCM_ioInit();
	pwmCM_pwmInit();
}

void DC11detectA_Thread(const void *argument){	//主检测线程
	
	osEvent  evt;
    osStatus status;
	
	pwmCM_MEAS *mptr = NULL;
	pwmCM_MEAS *rptr = NULL;
	
	STMFLASH_Read(MODULE_PWMid_DATADDR,(uint16_t *)&pwmDevMOUDLE_ID,1);
	if((pwmDevMOUDLE_ID != pwmDevMID_unvarLight)&&\
	   (pwmDevMOUDLE_ID != pwmDevMID_varLight)&&\
	   (pwmDevMOUDLE_ID != pwmDevMID_varFan))pwmDevMOUDLE_ID= pwmDevMID_varLight;
	
	osDelay(500);
	
	do{mptr = (pwmCM_MEAS *)osPoolCAlloc(pwmCM_pool);}while(mptr == NULL);	//底板按键事件送显,提示通信分址更改
	mptr->speDPCMD = SPECMD_pwmDevModADDR_CHG;
	mptr->Mod_addr = pwmDevMOUDLE_ID;
	osMessagePut(MsgBox_DPpwmCM, (uint32_t)mptr, 100);
	
	for(;;){
		
	/***********************本地线程数据接收***************************************************/
		evt = osMessageGet(MsgBox_MTpwmCM, 100);
		if (evt.status == osEventMessage) {		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			
			if(rptr->Mod_addr == pwmDevMOUDLE_ID){
			
				pwmDevAttr.Switch	= rptr->Switch;
				pwmDevAttr.pwmVAL	= rptr->pwmVAL;
				
				pwmDevAttr_temp.Switch = pwmDevAttr.Switch;		//缓存区数据同步，避免下行数据回发
				pwmDevAttr_temp.pwmVAL = pwmDevAttr.pwmVAL;
			}			

			do{status = osPoolFree(pwmCM_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		if(!EC11_SW){while(!EC11_SW)osDelay(20);pwmDevAttr.Switch = !pwmDevAttr.Switch;}	//旋钮开关
		if(!LIGHTCM_K1){while(!LIGHTCM_K1)osDelay(20);pwmDevAttr.Switch = true;}		//按键开关
		if(!LIGHTCM_K2){while(!LIGHTCM_K2)osDelay(20);pwmDevAttr.Switch = false;}	//按键开关
		
		if(pwmDevAttr_temp.Switch != pwmDevAttr.Switch){	//若开关开启，启动旋钮检测线程
			
			pwmDevAttr_temp.Switch = pwmDevAttr.Switch;
			
			if(pwmDevAttr.Switch == true){
				
				pwmDevAttr.pwmVAL = 50;
				tid_DC11detectB_Thread	= osThreadCreate(osThread(DC11detectB_Thread),NULL);
			}else{
			
				pwmDevAttr.pwmVAL = 0;
				osThreadTerminate(tid_DC11detectB_Thread);
			}
			
			do{mptr = (pwmCM_MEAS *)osPoolCAlloc(pwmCM_pool);}while(mptr == NULL);	//无线数据传输消息推送
			mptr->Mod_addr = pwmDevMOUDLE_ID;
			mptr->Switch   = pwmDevAttr.Switch;
			mptr->pwmVAL   = pwmDevAttr.pwmVAL;
			osMessagePut(MsgBox_pwmCM, (uint32_t)mptr, 100);
			
			osDelay(10);
		}
		
		if(pwmDevAttr_tempDP.Switch != pwmDevAttr.Switch ||
		   pwmDevAttr_tempDP.pwmVAL != pwmDevAttr.pwmVAL){
		   
				pwmDevAttr_tempDP.Switch = pwmDevAttr.Switch;
				pwmDevAttr_tempDP.pwmVAL = pwmDevAttr.pwmVAL;
			   
				do{mptr = (pwmCM_MEAS *)osPoolCAlloc(pwmCM_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
				mptr->speDPCMD	= SPECMD_pwmDevDATS_CHG;
				mptr->Switch    = pwmDevAttr.Switch;
				mptr->pwmVAL 	= pwmDevAttr.pwmVAL;
				osMessagePut(MsgBox_DPpwmCM, (uint32_t)mptr, 100);
		   }

		TIM_SetCompare2(TIM2,pwmDevAttr.pwmVAL * 2);	//占空比生效
	}
}

u8 scan_encoder(void){

	static  bool  Curr_encoder_b; 
	static  bool  Last_encoder_b;  
	static  bool  updata = 0; 	
	static	u8	  counter;
	
	if(PBout(13) && PBout(14)){

		updata = 0;       
		return 0;    
	}

	Last_encoder_b = PBout(14); 
	while(!PBout(13)){

		Curr_encoder_b = PBout(14);
		updata = 1;
	}
	
	if(updata){

		updata = 0;
		if((Last_encoder_b == 0)&&(Curr_encoder_b == 1)){

//				if(counter == 255)        
//				return;         
//				counter++;  

				return 1;
		}else	
		if((Last_encoder_b == 1)&&(Curr_encoder_b == 0)){

//				if(counter == 0)return;         
//				counter--; 
			
				return 2;
		}
	} 
	
	return 4;
	
}


void DC11detectB_Thread(const void *argument){	//占空比调制线程

	pwmCM_MEAS *mptr = NULL;
	u8 temp;
	char disp[30];
	
	for(;;){
		
		temp = scan_encoder();
		
		temp = 100;

		sprintf(disp,"%d",temp);
		
		Driver_USART1.Send(disp,strlen(disp));
		osDelay(20);
		
//		if(PBin(14)){
//		
//			if(PBin(13)){
//			
//				while(PBin(13));
//				//Driver_USART1.Send("b",1);
//				if(pwmDevAttr.pwmVAL > 60)pwmDevAttr.pwmVAL -= 4;else
//				if(pwmDevAttr.pwmVAL > 20)pwmDevAttr.pwmVAL -= 2;else
//				if(pwmDevAttr.pwmVAL > 1)pwmDevAttr.pwmVAL -= 1;
//				osDelay(5);
//			}
//		}
//		
//		if(!PBin(14)){
//			
//			if(PBin(13)){
//			
//				while(PBin(13));
//				//Driver_USART1.Send("a",1);
//				if(pwmDevAttr.pwmVAL < 20)pwmDevAttr.pwmVAL += 1;else
//				if(pwmDevAttr.pwmVAL < 60)pwmDevAttr.pwmVAL += 2;else
//				if(pwmDevAttr.pwmVAL < 100)pwmDevAttr.pwmVAL += 4;
//				osDelay(5);
//			}
//		}
		
//		if(temp){
//		
//			if(1 == temp){
//			
//				Driver_USART1.Send("b",1);
//				if(pwmDevAttr.pwmVAL > 60)pwmDevAttr.pwmVAL -= 4;else
//				if(pwmDevAttr.pwmVAL > 20)pwmDevAttr.pwmVAL -= 2;else
//				if(pwmDevAttr.pwmVAL > 1)pwmDevAttr.pwmVAL -= 1;
//				osDelay(5);
//			}

//			if(2 == temp){
//			
//				Driver_USART1.Send("a",1);
//				if(pwmDevAttr.pwmVAL < 20)pwmDevAttr.pwmVAL += 1;else
//				if(pwmDevAttr.pwmVAL < 60)pwmDevAttr.pwmVAL += 2;else
//				if(pwmDevAttr.pwmVAL < 100)pwmDevAttr.pwmVAL += 4;
//				osDelay(5);
//			}
//		}	

		if(pwmDevAttr_temp.pwmVAL != pwmDevAttr.pwmVAL){
			
			pwmDevAttr_temp.pwmVAL = pwmDevAttr.pwmVAL;
		
			do{mptr = (pwmCM_MEAS *)osPoolCAlloc(pwmCM_pool);}while(mptr == NULL);	//无线数据传输消息推送
			mptr->Mod_addr = pwmDevMOUDLE_ID;
			mptr->Switch   = pwmDevAttr.Switch;
			mptr->pwmVAL   = pwmDevAttr.pwmVAL;
			osMessagePut(MsgBox_pwmCM, (uint32_t)mptr, 100);
		}
		
		osDelay(10);
	}
}

void pwmCM_Thread(const void *argument){	//主线程(仅做显示)
	
	osEvent  evt;
    osStatus status;
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 5;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	for(;;){
		
	/***********************底板按键线程数据接收***************************************************/
		evt = osMessageGet(MsgBox_pwmCMsigK, 100);
		if (evt.status == osEventMessage){
			
			pwmCM_kMSG *rptr_sigK = NULL;
			pwmCM_MEAS *mptr = NULL;
		
			rptr_sigK = evt.value.p;
			/*底板按键消息处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			
			pwmDevMOUDLE_ID = rptr_sigK->mADDR;
			STMFLASH_Write(MODULE_PWMid_DATADDR,(uint16_t *)&pwmDevMOUDLE_ID,1);
			
			do{mptr = (pwmCM_MEAS *)osPoolCAlloc(pwmCM_pool);}while(mptr == NULL);	//底板按键事件送显,提示通信分址更改
			mptr->speDPCMD = SPECMD_pwmDevModADDR_CHG;
			mptr->Mod_addr = pwmDevMOUDLE_ID;
			osMessagePut(MsgBox_DPpwmCM, (uint32_t)mptr, 100);
			beeps(9);
			
			do{status = osPoolFree(memID_pwmCMsigK_pool, rptr_sigK);}while(status != osOK);	//内存释放
			rptr_sigK = NULL;
		}

//		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
//		else{
//		
//			Pcnt = 0;
//			memset(disp,0,dpSize * sizeof(char));
//			sprintf(disp,"灯光是否开启：%d\n当前亮度：%d%%\n\n",pwmDevAttr.Switch,pwmDevAttr.pwmVAL);
//			Driver_USART1.Send(disp,strlen(disp));
//			osDelay(20);
//		}
		
		osDelay(10);
	}
}

void pwmCM_Terminate(void){

	osThreadTerminate(tid_curtainCM_Thread); 
	osThreadTerminate(tid_DC11detectA_Thread); 
	osThreadTerminate(tid_DC11detectB_Thread); 
}

void pwmCMThread_Active(void){
	
	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		pwmCM_pool   = osPoolCreate(osPool(pwmCM_pool));	//创建内存池
		MsgBox_pwmCM = osMessageCreate(osMessageQ(MsgBox_pwmCM), NULL);	//创建消息队列
		MsgBox_MTpwmCM = osMessageCreate(osMessageQ(MsgBox_MTpwmCM), NULL);	//创建消息队列
		MsgBox_DPpwmCM = osMessageCreate(osMessageQ(MsgBox_DPpwmCM), NULL);	//创建消息队列
		
		memID_pwmCMsigK_pool = osPoolCreate(osPool(pwmCMsigK_pool));
		MsgBox_pwmCMsigK = osMessageCreate(osMessageQ(MsgBox_pwmCMsigK), NULL);
		
		memInit_flg = true;
	}
	
	pwmCM_Init();
	tid_pwmCM_Thread 		= osThreadCreate(osThread(pwmCM_Thread),NULL);
	tid_DC11detectA_Thread	= osThreadCreate(osThread(DC11detectA_Thread),NULL);
}
