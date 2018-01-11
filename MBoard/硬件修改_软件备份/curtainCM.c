#include "curtainCM.h"

static curtainCM_MEAS curtainATTR;

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_curtainCM_Thread;
osThreadDef(curtainCM_Thread,osPriorityNormal,1,512);
			 
osPoolId  curtainCM_pool;								 
osPoolDef(curtainCM_pool, 10, curtainCM_MEAS);                  // 内存池定义
osMessageQId  MsgBox_curtainCM;
osMessageQDef(MsgBox_curtainCM, 2, &curtainCM_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTcurtainCM;
osMessageQDef(MsgBox_MTcurtainCM, 2, &curtainCM_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPcurtainCM;
osMessageQDef(MsgBox_DPcurtainCM, 2, &curtainCM_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void curtainCM_ioInit(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE );	                

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;	//输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;	//输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	PBout(6) = PBout(7) = 0;
}

void curtainCM_ADCInit(void){

	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M                     

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   


	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束

	ADC_StartCalibration(ADC1);	 //开启AD校准

	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束

//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
}

u16 curtGet_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 curtGet_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val += curtGet_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 

void curtainCM_Init(void){

	curtainCM_ioInit();
	curtainCM_ADCInit();
}

void curtain_logInitCallback(uint32_t event){

	;
}

void curtain_logInit(void){

	/*Initialize the USART driver */
	Driver_USART1.Initialize(curtain_logInitCallback);
	/*Power up the USART peripheral */
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 4800 Bits/sec */
	Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
									ARM_USART_DATA_BITS_8 |
									ARM_USART_PARITY_NONE |
									ARM_USART_STOP_BITS_1 |
							ARM_USART_FLOW_CONTROL_NONE, 115200);

	/* Enable Receiver and Transmitter lines */
	Driver_USART1.Control (ARM_USART_CONTROL_TX, 1);
}

void curtainCM_Thread(const void *argument){
	
	const uint8_t dpSize = 50;
	const uint8_t dpPeriod = 40;
	char  disp[dpSize];
	uint8_t Pcnt;
	
	static bool CurtainUpEN = 0;
	static bool CurtainDnEN = 0;
	static bool CurtainEN = 0;
	static uint16_t Curtain_valElec = 0;
	u8 Kcnt;
//	static curtainCM_MEAS curtainATTR_temp;
	
	for(;;){
	
		curtIOCHG_Kin();
		
		if(!curtKeySW){	//按键总使能——翻转
			
			Kcnt = 100;
			while(!curtKeySW && Kcnt){osDelay(20);Kcnt --;}
			CurtainEN = !CurtainEN;
		}if(!CurtainEN){CurtainUpEN = CurtainDnEN = 0; curtMTUP = curtMTDN = 0;}		//所有按键失能状态执行	
		
		if(CurtainEN && !curtKeyUP){	//开窗使能——翻转
			
			Kcnt = 100;
			while(!curtKeyUP && Kcnt){osDelay(20);Kcnt --;}
			CurtainUpEN = !CurtainUpEN; 
			CurtainDnEN = 0;
		}	
		if(CurtainEN && !curtKeyDN){	//关窗使能——翻转
		
			Kcnt = 100;
			while(!curtKeyDN && Kcnt){osDelay(20);Kcnt --;}
			CurtainDnEN = !CurtainDnEN; 
			CurtainUpEN = 0;
		}	
		
		if(CurtainUpEN)curtMTUP = 1;else curtMTUP = 0;	//开窗状态执行
		if(CurtainDnEN)curtMTDN = 1;else curtMTDN = 0;	//关窗状态执行
		
		Curtain_valElec = curtGet_Adc_Average(0,5);
		
		curtainATTR.CurtainENs   = CurtainEN;
		curtainATTR.CurtainUpENs = CurtainUpEN;
		curtainATTR.CurtainDnENs = CurtainDnEN;
		curtainATTR.valElec      = Curtain_valElec;
		
		curtIOCHG_DB();
		
		if(Pcnt < dpPeriod){
			
			osDelay(20);
			Pcnt ++;
		}else{
			
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"★--------------☆\n CurtainEN : %d\n CurtainUpEN : %d\n CurtainDnEN : %d\n valElec : %d\n",curtainATTR.CurtainENs,curtainATTR.CurtainUpENs,curtainATTR.CurtainDnENs,curtainATTR.valElec);
			Driver_USART1.Send(disp,strlen(disp));
			osDelay(20);
		}
	}
}

void curtainCMThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		curtainCM_pool   = osPoolCreate(osPool(curtainCM_pool));	//创建内存池
		MsgBox_curtainCM 	= osMessageCreate(osMessageQ(MsgBox_curtainCM), NULL);   //创建消息队列
		MsgBox_MTcurtainCM = osMessageCreate(osMessageQ(MsgBox_MTcurtainCM), NULL);//创建消息队列
		MsgBox_DPcurtainCM = osMessageCreate(osMessageQ(MsgBox_DPcurtainCM), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	curtainCM_Init();
	tid_curtainCM_Thread = osThreadCreate(osThread(curtainCM_Thread),NULL);
}
