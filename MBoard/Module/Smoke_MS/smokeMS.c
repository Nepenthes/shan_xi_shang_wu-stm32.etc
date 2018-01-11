#include "smokeMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_smokeMS_Thread;
osThreadDef(smokeMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  smokeMS_pool;								 
osPoolDef(smokeMS_pool, 10, smokeMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_smokeMS;
osMessageQDef(MsgBox_smokeMS, 2, &smokeMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTsmokeMS;
osMessageQDef(MsgBox_MTsmokeMS, 2, &smokeMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPsmokeMS;
osMessageQDef(MsgBox_DPsmokeMS, 2, &smokeMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void smokeMS_DIOinit(void){

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 
}

void smokeMS_AIOinit(void){

	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);	  //使能ADC1通道时钟
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M


	//PC0 1 作为模拟通道输入引脚  ADC12_IN8                       

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
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

void smokeMS_Init(void){

	smokeMS_DIOinit();
	smokeMS_AIOinit();
}

//获得ADC值
//ch:通道值 0~3
uint16_t smokeGet_Adc(uint8_t ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

uint16_t smokeGet_Adc_Average(uint8_t ch,uint8_t times)
{
	u32 temp_val=0;
	uint8_t t;
	
	for(t=0;t<times;t++)
	{
		temp_val += smokeGet_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	

void smokeMS_Thread(const void *argument){
	
	osEvent  evt;
    osStatus status;
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 20;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	smokeMS_MEAS	sensorData;
	static smokeMS_MEAS Data_temp = {1};
	
	smokeMS_MEAS *mptr = NULL;
	smokeMS_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTsmokeMS, 100);
		if (evt.status == osEventMessage) {		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			

			do{status = osPoolFree(smokeMS_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		sensorData.anaDAT	= (uint8_t)(smokeGet_Adc_Average(1,8) / 41);		//数据采集
		sensorData.VAL		= GAS_DATA;
		
		if(Data_temp.anaDAT != sensorData.anaDAT || Data_temp.VAL != sensorData.VAL){	//数据推送（数据更替时才触发）
		
			Data_temp.anaDAT = sensorData.anaDAT;
			Data_temp.VAL 	 = sensorData.VAL;
			
			do{mptr = (smokeMS_MEAS *)osPoolCAlloc(smokeMS_pool);}while(mptr == NULL);
			mptr->anaDAT = sensorData.anaDAT;
			mptr->VAL = sensorData.VAL;
			osMessagePut(MsgBox_smokeMS, (uint32_t)mptr, 100);
			
			do{mptr = (smokeMS_MEAS *)osPoolCAlloc(smokeMS_pool);}while(mptr == NULL);
			mptr->anaDAT = sensorData.anaDAT;
			mptr->VAL = sensorData.VAL;
			osMessagePut(MsgBox_DPsmokeMS, (uint32_t)mptr, 100);
			osDelay(10);
		}
		
		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
		else{
		
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"\n\rvalAnalog : %d,valDigital : %d\n\r", sensorData.anaDAT,sensorData.VAL);			
			Driver_USART1.Send(disp,strlen(disp));	
			osDelay(20);
		}
		
		osDelay(10);
	}
}

void smokeMSThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		smokeMS_pool   = osPoolCreate(osPool(smokeMS_pool));	//创建内存池
		MsgBox_smokeMS 	= osMessageCreate(osMessageQ(MsgBox_smokeMS), NULL);   //创建消息队列
		MsgBox_MTsmokeMS = osMessageCreate(osMessageQ(MsgBox_MTsmokeMS), NULL);//创建消息队列
		MsgBox_DPsmokeMS = osMessageCreate(osMessageQ(MsgBox_DPsmokeMS), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	smokeMS_Init();
	tid_smokeMS_Thread = osThreadCreate(osThread(smokeMS_Thread),NULL);
}
