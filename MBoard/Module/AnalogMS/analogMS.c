#include "analogMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_analogMS_Thread;
osThreadDef(analogMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  analogMS_pool;								 
osPoolDef(analogMS_pool, 10, analogMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_analogMS;
osMessageQDef(MsgBox_analogMS, 2, &analogMS_MEAS);            // 消息队列定义，用于模块进程向无线通讯进程
osMessageQId  MsgBox_MTanalogMS;
osMessageQDef(MsgBox_MTanalogMS, 2, &analogMS_MEAS);          // 消息队列定义,用于无线通讯进程向模块进程
osMessageQId  MsgBox_DPanalogMS;
osMessageQDef(MsgBox_DPanalogMS, 2, &analogMS_MEAS);          // 消息队列定义，用于模块进程向显示模块进程

void analogMS_ADCInit(void){

	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M                     

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5;
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

u16 analogGet_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 analogGet_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val += analogGet_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 

void analogMS_Init(void){

	analogMS_ADCInit();
}

void analogMS_Thread(const void *argument){

	osEvent  evt;
    osStatus status;
	
	const bool UPLOAD_MODE = false;	//1：数据变化时才上传 0：周期定时上传
	
	const uint8_t upldPeriod = 5;	//数据上传周期因数（UPLOAD_MODE = false 时有效）
	
	uint8_t UPLDcnt = 0;
	bool UPLD_EN = false;
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 10;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	analogMS_MEAS sensorData;
	static analogMS_MEAS Data_temp = {1};
	static analogMS_MEAS Data_tempDP = {1};
	
	analogMS_MEAS *mptr = NULL;
	analogMS_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTanalogMS, 100);
		if (evt.status == osEventMessage) {		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地进程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			

			do{status = osPoolFree(analogMS_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}

		sensorData.Ich1 = analogGet_Adc_Average(0,100);
		sensorData.Ich2 = analogGet_Adc_Average(4,100);
		
		if(!UPLOAD_MODE){	//选择上传触发模式
		
			if(UPLDcnt < upldPeriod)UPLDcnt ++;
			else{
			
				UPLDcnt = 0;
				UPLD_EN = true;
			}
		}else{
			
			if(Data_temp.Ich1 != sensorData.Ich1 ||
			   Data_temp.Ich2 != sensorData.Ich2){
			
				Data_temp.Ich1 = sensorData.Ich1;
				Data_temp.Ich2 = sensorData.Ich2;
				UPLD_EN = true;
			}
		}
		
		if(UPLD_EN){
			
			UPLD_EN = false;
			   
			do{mptr = (analogMS_MEAS *)osPoolCAlloc(analogMS_pool);}while(mptr == NULL);
			mptr->Ich1 = sensorData.Ich1;
			mptr->Ich2 = sensorData.Ich2;
			osMessagePut(MsgBox_analogMS, (uint32_t)mptr, 100);
			osDelay(500);
		}
		
		if(Data_tempDP.Ich1 != sensorData.Ich1 ||
		   Data_tempDP.Ich2 != sensorData.Ich2){
		
			Data_tempDP.Ich1 = sensorData.Ich1;
			Data_tempDP.Ich2 = sensorData.Ich2;
			
			do{mptr = (analogMS_MEAS *)osPoolCAlloc(analogMS_pool);}while(mptr == NULL);
			mptr->Ich1 = sensorData.Ich1;
			mptr->Ich2 = sensorData.Ich2;
			osMessagePut(MsgBox_DPanalogMS, (uint32_t)mptr, 100);
			osDelay(10);
		}
		
		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
		else{
		
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"☆--------★\n Ich1:%d\n Ich2:%d\n Vch1:%d\n Vch2:%d\n\n",sensorData.Ich1,sensorData.Ich2,sensorData.Vch1,sensorData.Vch2);
			Driver_USART1.Send(disp,strlen(disp));
			osDelay(20);
		}
		
		osDelay(10);
	}
}

void analogMSThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		analogMS_pool   = osPoolCreate(osPool(analogMS_pool));	//创建内存池
		MsgBox_analogMS 	= osMessageCreate(osMessageQ(MsgBox_analogMS), NULL);   //创建消息队列
		MsgBox_MTanalogMS = osMessageCreate(osMessageQ(MsgBox_MTanalogMS), NULL);//创建消息队列
		MsgBox_DPanalogMS = osMessageCreate(osMessageQ(MsgBox_DPanalogMS), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	analogMS_Init();
	tid_analogMS_Thread = osThreadCreate(osThread(analogMS_Thread),NULL);
}
