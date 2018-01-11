#include "gasMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_gasMS_Thread;
osThreadDef(gasMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  gasMS_pool;								 
osPoolDef(gasMS_pool, 10, gasMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_gasMS;
osMessageQDef(MsgBox_gasMS, 2, &gasMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTgasMS;
osMessageQDef(MsgBox_MTgasMS, 2, &gasMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPgasMS;
osMessageQDef(MsgBox_DPgasMS, 2, &gasMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

void gasMS_DIOinit(void){

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 
}

void gasMS_AIOinit(void){

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

void gasMS_Init(void){

	gasMS_DIOinit();
	gasMS_AIOinit();
}

//获得ADC值
//ch:通道值 0~3
uint16_t gasGet_Adc(uint8_t ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

uint16_t gasGet_Adc_Average(uint8_t ch,uint8_t times)
{
	u32 temp_val=0;
	uint8_t t;
	
	for(t=0;t<times;t++)
	{
		temp_val += gasGet_Adc(ch);
		delay_ms(5);
	}
	return temp_val / times;
} 	

void gasMS_Thread(const void *argument){

	osEvent  evt;
    osStatus status;	
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 20;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	gasMS_MEAS	sensorData;
	static gasMS_MEAS Data_temp = {1};
	
	gasMS_MEAS *mptr = NULL;
	gasMS_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTgasMS, 100);
		if (evt.status == osEventMessage) {		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			

			do{status = osPoolFree(gasMS_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		sensorData.anaDAT	= (uint8_t)(gasGet_Adc_Average(1,8) / 41);		//数据采集
		sensorData.VAL		= GAS_DATA;
		
		if(Data_temp.anaDAT != sensorData.anaDAT || 	//数据推送（数据更替时才触发）
		   Data_temp.VAL != sensorData.VAL){	
		
			Data_temp.anaDAT = sensorData.anaDAT;
			Data_temp.VAL 	 = sensorData.VAL;
			
			do{mptr = (gasMS_MEAS *)osPoolCAlloc(gasMS_pool);}while(mptr == NULL);
			mptr->anaDAT = sensorData.anaDAT;
			mptr->VAL = sensorData.VAL;
			osMessagePut(MsgBox_gasMS, (uint32_t)mptr, 100);
			
			do{mptr = (gasMS_MEAS *)osPoolCAlloc(gasMS_pool);}while(mptr == NULL);
			mptr->anaDAT = sensorData.anaDAT;
			mptr->VAL = sensorData.VAL;
			osMessagePut(MsgBox_DPgasMS, (uint32_t)mptr, 100);
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

void gasMSThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		gasMS_pool   = osPoolCreate(osPool(gasMS_pool));	//创建内存池
		MsgBox_gasMS 	= osMessageCreate(osMessageQ(MsgBox_gasMS), NULL);   //创建消息队列
		MsgBox_MTgasMS = osMessageCreate(osMessageQ(MsgBox_MTgasMS), NULL);//创建消息队列
		MsgBox_DPgasMS = osMessageCreate(osMessageQ(MsgBox_DPgasMS), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	gasMS_Init();
	tid_gasMS_Thread = osThreadCreate(osThread(gasMS_Thread),NULL);
}
