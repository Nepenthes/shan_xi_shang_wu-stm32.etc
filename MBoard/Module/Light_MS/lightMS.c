#include "lightMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_lightMS_Thread;
osThreadDef(lightMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  lightMS_pool;								 
osPoolDef(lightMS_pool, 10, lightMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_lightMS;
osMessageQDef(MsgBox_lightMS, 2, &lightMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTlightMS;
osMessageQDef(MsgBox_MTlightMS, 2, &lightMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPlightMS;
osMessageQDef(MsgBox_DPlightMS, 2, &lightMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

cdsADC_Init(void){

//	ADC_InitTypeDef ADC_InitStructure; 
//	GPIO_InitTypeDef GPIO_InitStructure;

//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟

//	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M                     

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
//	GPIO_Init(GPIOA, &GPIO_InitStructure);	

//	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

//	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
//	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
//	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
//	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
//	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
//	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
//	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   


//	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

//	ADC_ResetCalibration(ADC1);	//使能复位校准  
//	 
//	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束

//	ADC_StartCalibration(ADC1);	 //开启AD校准

//	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束

////	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能
}

u16 cdsGet_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 cdsGet_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val += cdsGet_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 

//IO初始化
void tsl2561IO_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB,PE端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;	 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5

 GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	
}

void TSL2561_Init(void)
{
	tsl2561IO_Init();
	TSLSDA_OUT();
	TSLIIC_SCL=1;
	TSLIIC_SDA=1;
	TSL2561_Write(CONTROL,0x03);
	delay_ms(100);
	TSL2561_Write(TIMING,0x02);
}

void tsl2561_start(void)
{
	TSLSDA_OUT();     //sda
	TSLIIC_SDA=1;	  	  
	TSLIIC_SCL=1;
	delay_us(4);
 	TSLIIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	TSLIIC_SCL=0;//
}

void stop(void)
{
	TSLSDA_OUT();//sda
	TSLIIC_SCL=0;
	TSLIIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	TSLIIC_SCL=1; 
	TSLIIC_SDA=1;//
	delay_us(4);							   	
}

void respons(void)
{
	TSLIIC_SCL=0;
	TSLSDA_OUT();
	TSLIIC_SDA=0;
	delay_us(2);
	TSLIIC_SCL=1;
	delay_us(2);
	TSLIIC_SCL=0;
}

void write_byte(uint8 value)
{
    uint8_t t;   
	TSLSDA_OUT(); 	    
    TSLIIC_SCL=0;
    for(t=0;t<8;t++)
    {              
        //TSLIIC_SDA=(txd&0x80)>>7;
		if((value&0x80)>>7)
			TSLIIC_SDA=1;
		else
			TSLIIC_SDA=0;
		value<<=1; 	  
		delay_us(2);   
		TSLIIC_SCL=1;
		delay_us(2); 
		TSLIIC_SCL=0;	
		delay_us(2);
    }	 
}

uint8 read_byte(void)
{
	unsigned char i,receive=0;
	TSLSDA_IN();//SDA
    for(i=0;i<8;i++ )
	{
        TSLIIC_SCL=0; 
        delay_us(2);
		TSLIIC_SCL=1;
        receive<<=1;
        if(TSLREAD_SDA)receive++;   
		delay_us(1); 
    }					 

	TSLSDA_OUT();
	TSLIIC_SDA=1;//release DATA-line
	return receive;
}


void TSL2561_Write(uint8 command,uint8 data)
{
	tsl2561_start();
	write_byte(SLAVE_ADDR_WR);
	respons();
	write_byte(command);
	respons();
	write_byte(data);
	respons();
	stop();
}


uint8 TSL2561_Read(uint8 command)
{
	uint8 data;
	tsl2561_start();
	write_byte(SLAVE_ADDR_WR);
	respons();
	write_byte(command);
	respons();
	
	tsl2561_start();
	write_byte(SLAVE_ADDR_RD);
	respons();
	data=read_byte();
	stop();
	return data;
}

uint32 Read_Light(void)
{
//	uint16 Channel0,Channel1;
//	uint8 Data0_L,Data0_H,Data1_L,Data1_H;
//	
//	Data0_L=TSL2561_Read(DATA0LOW);
//	Data0_H=TSL2561_Read(DATA0HIGH);
//	Channel0=(256*Data0_H + Data0_L);
//	
//	Data1_L=TSL2561_Read(DATA1LOW);
//	Data1_H=TSL2561_Read(DATA1HIGH);
//	Channel1=(256*Data1_H + Data1_L);
//	
//	return calculateLux(Channel0,Channel1);
	
	return (uint32)cdsGet_Adc_Average(0,5);
}

uint32_t calculateLux(uint16_t ch0, uint16_t ch1)
{
		uint32_t chScale;
		uint32_t channel1;
		uint32_t channel0;
		uint32_t temp;
		uint32_t ratio1 = 0;
		uint32_t ratio	;
		uint32_t lux_temp;
		uint16_t b, m;
		chScale=(1 <<TSL2561_LUX_CHSCALE);           //这是时间寄存器为0x02的
		chScale = chScale << 4;                      //这是增益为1的，增益为16不用写这一条
		// scale the channel values
		channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
		channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;
		// find the ratio of the channel values (Channel1/Channel0)
		if (channel0 != 0)
		ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;
		ratio = (ratio1 + 1) >> 1;	  									 // round the ratio value
		if ((ratio > 0) && (ratio <= TSL2561_LUX_K1T))
			{
				b=TSL2561_LUX_B1T;
				m=TSL2561_LUX_M1T;
			}
		else if (ratio <= TSL2561_LUX_K2T)
			{
				b=TSL2561_LUX_B2T;
				m=TSL2561_LUX_M2T;
			}
		else if (ratio <= TSL2561_LUX_K3T)
			{
				b=TSL2561_LUX_B3T;
				m=TSL2561_LUX_M3T;
			}
		else if (ratio <= TSL2561_LUX_K4T)
			{
				b=TSL2561_LUX_B4T;
				m=TSL2561_LUX_M4T;
			}
		else if (ratio <= TSL2561_LUX_K5T)
			{
				b=TSL2561_LUX_B5T;
				m=TSL2561_LUX_M5T;
			}
		else if (ratio <= TSL2561_LUX_K6T)
			{
				b=TSL2561_LUX_B6T;
				m=TSL2561_LUX_M6T;
			}
		else if (ratio <= TSL2561_LUX_K7T)
			{
				b=TSL2561_LUX_B7T;
				m=TSL2561_LUX_M7T;
			}
		else if (ratio > TSL2561_LUX_K8T)
			{
				b=TSL2561_LUX_B8T;
				m=TSL2561_LUX_M8T;
			}
		temp = ((channel0 * b) - (channel1 * m));
		if (temp < 1)  temp = 0;							// do not allow negative lux value
		temp += (1 << (TSL2561_LUX_LUXSCALE-1));			// round lsb (2^(LUX_SCALE-1))
		lux_temp = temp >> TSL2561_LUX_LUXSCALE;			// strip off fractional portion
		return lux_temp;		  							// Signal I2C had no errors
}

void lightMS_Init(void){

//	TSL2561_Init();
	cdsADC_Init();
}

void lightMS_Thread(const void *argument){

	osEvent  evt;
    osStatus status;	
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 40;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	lightMS_MEAS sensorData;
	static lightMS_MEAS Data_temp = {1};
	
	lightMS_MEAS *mptr = NULL;
	lightMS_MEAS *rptr = NULL;
	
	for(;;){
		
	/***********************本地线程数据接收***************************************************/
	//传感器构件数据仅作上传，接收数据功能保留，暂时不用
		evt = osMessageGet(MsgBox_MTlightMS, 100);
		if (evt.status == osEventMessage) {		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			

			do{status = osPoolFree(lightMS_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}

		sensorData.illumination = Read_Light();
		
		if(Data_temp.illumination != sensorData.illumination){	//数据推送（数据更替时才触发）
		
			Data_temp.illumination = sensorData.illumination;
			
			do{mptr = (lightMS_MEAS *)osPoolCAlloc(lightMS_pool);}while(mptr == NULL);	//无线数据传输消息推送
			mptr->illumination = sensorData.illumination;
			osMessagePut(MsgBox_lightMS, (uint32_t)mptr, 100);
			
			do{mptr = (lightMS_MEAS *)osPoolCAlloc(lightMS_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
			mptr->illumination = sensorData.illumination;
			osMessagePut(MsgBox_DPlightMS, (uint32_t)mptr, 100);
			osDelay(10);
		}
		
		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
		else{
		
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"当前光照强度为：%d Lux \r\n",sensorData.illumination);
			Driver_USART1.Send(disp,strlen(disp));
			osDelay(20);
		}	
		osDelay(10);
	}
}

void lightMSThread_Active(void){
	
	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		lightMS_pool   = osPoolCreate(osPool(lightMS_pool));	//创建内存池
		MsgBox_lightMS 	= osMessageCreate(osMessageQ(MsgBox_lightMS), NULL);   //创建消息队列
		MsgBox_MTlightMS = osMessageCreate(osMessageQ(MsgBox_MTlightMS), NULL);//创建消息队列
		MsgBox_DPlightMS = osMessageCreate(osMessageQ(MsgBox_DPlightMS), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	lightMS_Init();
	tid_lightMS_Thread = osThreadCreate(osThread(lightMS_Thread),NULL);
}
