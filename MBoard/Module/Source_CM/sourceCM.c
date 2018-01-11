#include "sourceCM.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_sourceCM_Thread;
osThreadDef(sourceCM_Thread,osPriorityNormal,1,512);
			 
osPoolId  sourceCM_pool;								 
osPoolDef(sourceCM_pool, 10, sourceCM_MEAS);                  // 内存池定义
osMessageQId  MsgBox_sourceCM;
osMessageQDef(MsgBox_sourceCM, 2, &sourceCM_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTsourceCM;
osMessageQDef(MsgBox_MTsourceCM, 2, &sourceCM_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPsourceCM;
osMessageQDef(MsgBox_DPsourceCM, 2, &sourceCM_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void sourceCM_ioInit(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );	                

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	//输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	PBout(0)= 0;
}

void sourceCM_ADCInit(void){

	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M                     

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
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

void sourceCM_Init(void){

	sourceCM_ioInit();
	sourceCM_ADCInit();
}

u16 soceGet_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 soceGet_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val += soceGet_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 

void sourceCM_Thread(const void *argument){

	osEvent  evt;
    osStatus status;	
	
	const uint8_t dpSize = 40;
	const uint8_t dpPeriod = 10;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	sourceCM_MEAS actuatorData;	//本地输入量
	static sourceCM_MEAS Data_temp   = {1};	//下行数据输入量同步对比缓存
	static sourceCM_MEAS Data_tempDP = {1};	//本地输入量显示数据对比缓存
	
	sourceCM_MEAS *mptr = NULL;
	sourceCM_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTsourceCM, 100);
		if (evt.status == osEventMessage){		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			
			actuatorData.Switch = rptr->Switch;
			
			Data_temp.Switch = actuatorData.Switch;

			do{status = osPoolFree(sourceCM_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		soceRELAY = actuatorData.Switch;
		actuatorData.anaVal = soceGet_Adc_Average(4,5);
		
		if(Data_temp.anaVal != actuatorData.anaVal){	
		
				Data_temp.anaVal = actuatorData.anaVal;
				
				do{mptr = (sourceCM_MEAS *)osPoolCAlloc(sourceCM_pool);}while(mptr == NULL);	//无线数据传输消息推送
				mptr->anaVal = actuatorData.anaVal;
				osMessagePut(MsgBox_sourceCM, (uint32_t)mptr, 100);
		}
		
		if(Data_tempDP.anaVal != actuatorData.anaVal ||
		   Data_tempDP.Switch != actuatorData.Switch){
			   
				Data_tempDP.anaVal = actuatorData.anaVal;
				Data_tempDP.Switch = actuatorData.Switch;
		   
				do{mptr = (sourceCM_MEAS *)osPoolCAlloc(sourceCM_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
				mptr->anaVal = actuatorData.anaVal;
				mptr->Switch = actuatorData.Switch;
				osMessagePut(MsgBox_DPsourceCM, (uint32_t)mptr, 100);
				osDelay(10);
		   }
	
		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
		else{
		
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"★-------------☆\n\r开关状态 : %d\n模拟量：%d\n\n\r", actuatorData.Switch,actuatorData.anaVal);			
			Driver_USART1.Send(disp,strlen(disp));
			osDelay(20);
		}
		
		osDelay(10);
	}
}

void sourceCMThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		sourceCM_pool   = osPoolCreate(osPool(sourceCM_pool));	//创建内存池
		MsgBox_sourceCM = osMessageCreate(osMessageQ(MsgBox_sourceCM), NULL);   //创建消息队列
		MsgBox_MTsourceCM = osMessageCreate(osMessageQ(MsgBox_MTsourceCM), NULL);//创建消息队列
		MsgBox_DPsourceCM = osMessageCreate(osMessageQ(MsgBox_DPsourceCM), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	sourceCM_Init();
	tid_sourceCM_Thread = osThreadCreate(osThread(sourceCM_Thread),NULL);
}
