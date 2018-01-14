#include "tempMS.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

osThreadId tid_tempMS_Thread;
osThreadDef(tempMS_Thread,osPriorityNormal,1,512);
			 
osPoolId  tempMS_pool;								 
osPoolDef(tempMS_pool, 10, tempMS_MEAS);                  // 内存池定义
osMessageQId  MsgBox_tempMS;
osMessageQDef(MsgBox_tempMS, 2, &tempMS_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTtempMS;
osMessageQDef(MsgBox_MTtempMS, 2, &tempMS_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPtempMS;
osMessageQDef(MsgBox_DPtempMS, 2, &tempMS_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void reset(void);
void sht11_start(void);
void sendByte(uint8 value);
uint8 recvByte(uint8 ack);

void SHT_WInit(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.
}

void SHT_RInit(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.
}


void convert_shtxx(float * hum, float *temp)
{
	const float C1=-4;//-2.0468;           // for 12 Bit RH
	const float C2=+0.0405;//+0.0367;           // for 12 Bit RH
	const float C3=-0.0000028;//-0.0000015955;     // for 12 Bit RH
	const float T1=+0.01;             // for 12 Bit RH
	const float T2=+0.00008;          // for 12 Bit RH

	float rh=*hum;             // rh:      Humidity [Ticks] 12 Bit
	float t=*temp;           // t:       Temperature [Ticks] 14 Bit
	float rh_lin;                     // rh_lin:  Humidity linear
	float rh_true;                    // rh_true: Temperature compensated humidity
	float t_C;                        // t_C   :  Temperature

	t_C     = t*0.01 - 39.66;                //calc. temperature  from 14 bit temp. ticks @ 3.5V
	rh_lin  = C3*rh*rh + C2*rh + C1;     //calc. humidity from ticks to [%RH]
	rh_true = (t_C - 25)*(T1 + T2*rh) + rh_lin;   //calc. temperature compensated humidity [%RH]

	if(rh_true > 100)
		rh_true = 100;       //cut if the value is outside of

	if(rh_true < 0.1)
		rh_true = 0.1;       //the physical possible range

	*temp = t_C;               //return temperature
	*hum  = rh_true;              //return humidity[%RH]
}


/**************************************************************************
 *@fn       readTH
 *
 *@brief    read temperature and humidity sensor data.
 *
 *@param    pData-data buffer.
 *          sht11_startIndex- .
 *
 *@return   none
 */



void readTH(uint8 *pData, uint8 *pStartIndex)
{
	/*Reading temperature*/
	reset();
	sht11_start();
	sendByte(SHT_MEASURE_TEMP);
	SHT_DATA_IN;

	*pStartIndex = 0;

	while (SHT_RD_DATA());

	pData[(*pStartIndex) + 1] = recvByte(0x1); // send ack to sensor
	pData[(*pStartIndex) + 1] &= 0x3f; /*00111111*/
	pData[(*pStartIndex)] = recvByte(0x1); // send ack to sensor
	*pStartIndex += 2;
	(void)recvByte(0x0);//crc chksum and nack to sensor, end transfer, throw away

	/*Reading humidity*/
	reset();
	sht11_start();
	sendByte(SHT_MEASURE_HUMI);
	SHT_DATA_IN;

	while (SHT_RD_DATA());

	pData[(*pStartIndex) + 1] = recvByte(0x1);
	pData[(*pStartIndex) + 1] &= 0x0f; /*00001111*/
	pData[(*pStartIndex)] = recvByte(0x1);
	*pStartIndex += 2;
	(void)recvByte(0x0);//crc chksum and nack to sensor, end transfer, throw away
}

void sht11_start()
{
  SHT_DATA_OUT;
  //SHT_SCK_OUT;

  SHT_DATA(1);
  SHT_SCK(0);// = 0;

  udelay();

  SHT_SCK(1);// = 1;
  udelay();

  SHT_DATA(0); // = 0;
  udelay();

  SHT_SCK(0);// = 0;
  udelay();

  SHT_SCK(1);// = 1;
  udelay();

  SHT_DATA(1);// = 1;
  udelay();

  SHT_SCK(0);// = 0;
}

void reset()
//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transsht11_start
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
{
  uint8_t i;

  SHT_DATA_OUT;
  //SHT_SCK_OUT;


  SHT_DATA(1);// = 1;
  SHT_SCK(0);// = 0;                    //Initial state
  udelay();

  for (i=0; i<9; i++)                  //9 SCK cycles
  {
    SHT_SCK(1);// = 1;
    udelay();

    SHT_SCK(0);// = 0;

    udelay();

  }
  sht11_start();                   //transmission sht11_start
}


void sendByte(uint8 value)
{
  uint8 i;

  SHT_DATA_OUT;
  //SHT_SCK_OUT;

  SHT_SCK(0);// = 0;
   udelay();

  for (i=0x80; i>0; i/=2)
  {
    if (i & value)
    {
      SHT_DATA(1);// = 1;
    }
    else
    {
      SHT_DATA(0);// = 0;
    }

    SHT_SCK(1);// = 1;
    udelay();

    SHT_SCK(0);// = 0;
    	udelay();
  }

  //SHT_DATA(1);// = 1;
  //SHT_SCK(1);// = 1;

  SHT_DATA_IN;
  SHT_SCK(1);// = 1;

  while(SHT_RD_DATA());//wait for ack

  SHT_SCK(0);// = 0;
  SHT_DATA_OUT;
}


uint8 recvByte(uint8 ack)
{
  uint8 i,val = 0;

  //SHT_SCK_OUT;


  SHT_DATA_OUT;
  SHT_DATA(1);// = 1;////////////////////

  SHT_DATA_IN;

  udelay();

  for (i=0x80; i>0; i/=2)
  {
    SHT_SCK(1);// = 1;
    udelay();

    if (SHT_RD_DATA())
    {
      val = (val | i);
    }

    SHT_SCK(0);// = 0;
    udelay();
  }

  SHT_DATA_OUT;

  SHT_DATA(!ack);// = !ack;                     //in case of "ack==1" pull down DATA-Line
  SHT_SCK(1);// = 1;                            //clk #9 for ack
  udelay();

  SHT_SCK(0);// = 0;
  SHT_DATA(1);// = 1;                          //release DATA-line
  return val;
}

void tempMS_Init(void){

	;
}

void tempMS_Thread(const void *argument){

	osEvent  evt;
    osStatus status;
	
	const bool UPLOAD_MODE = false;	//1：数据变化时才上传 0：周期定时上传
	
	const uint8_t upldPeriod = 15;	//数据上传周期因数（UPLOAD_MODE = false 时有效）
	
	uint8_t UPLDcnt = 0;
	bool UPLD_EN = false;
	
	const uint8_t dpSize = 30;
	const uint8_t dpPeriod = 5;
	
	static uint8_t Pcnt = 0;
	char disp[dpSize];
	
	result_t res;
	uint8_t len;
	
	tempMS_MEAS	sensorData = {0.0,0.0};
	static tempMS_MEAS Data_temp = {1};
	static tempMS_MEAS Data_tempDP = {1};
	
	tempMS_MEAS *mptr = NULL;
	tempMS_MEAS *rptr = NULL;
	
	for(;;){
		
		evt = osMessageGet(MsgBox_MTtempMS, 100);
		if (evt.status == osEventMessage) {		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			

			do{status = osPoolFree(tempMS_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		readTH((uint8 *)res.sensor_dat, &len);
		
		sensorData.hum 	 = res.hum_temp.temp;
		sensorData.temp  = res.hum_temp.hum;
		convert_shtxx(&sensorData.hum, &sensorData.temp);
		
		if(!UPLOAD_MODE){	//选择上传触发模式
		
			if(UPLDcnt < upldPeriod)UPLDcnt ++;
			else{
			
				UPLDcnt = 0;
				UPLD_EN = true;
			}
		}else{
			
			if(Data_temp.hum != sensorData.hum || 
			   Data_temp.temp != sensorData.temp){
			
				Data_temp.hum  = sensorData.hum;
				Data_temp.temp = sensorData.temp;
				UPLD_EN = true;
			}
		}

		if(UPLD_EN){
			
			UPLD_EN = false;
			
			do{mptr = (tempMS_MEAS *)osPoolCAlloc(tempMS_pool);}while(mptr == NULL);
			mptr->hum  = sensorData.hum;
			mptr->temp = sensorData.temp;
			osMessagePut(MsgBox_tempMS, (uint32_t)mptr, 100);
			osDelay(500);
		}
		
		if(Data_tempDP.hum != sensorData.hum || 
		   Data_tempDP.temp != sensorData.temp){
		
			Data_tempDP.hum  = sensorData.hum;
			Data_tempDP.temp = sensorData.temp;
			
			do{mptr = (tempMS_MEAS *)osPoolCAlloc(tempMS_pool);}while(mptr == NULL);
			mptr->hum  = sensorData.hum;
			mptr->temp = sensorData.temp;
			osMessagePut(MsgBox_DPtempMS, (uint32_t)mptr, 100);
			osDelay(10);
		}
			
		if(Pcnt < dpPeriod){osDelay(10);Pcnt ++;}
		else{
		
			Pcnt = 0;
			memset(disp,0,dpSize * sizeof(char));
			sprintf(disp,"\n\r当前湿度： %.2f%%, 当前温度： %.2f℃\n\r", sensorData.hum, sensorData.temp);			
			Driver_USART1.Send(disp,strlen(disp));	
			osDelay(20);
		}
		
		osDelay(10);
	}
}

void tempMSThread_Active(void){

	static bool memInit_flg = false;
	
	if(!memInit_flg){
	
		tempMS_pool   = osPoolCreate(osPool(tempMS_pool));	//创建内存池
		MsgBox_tempMS 	= osMessageCreate(osMessageQ(MsgBox_tempMS), NULL);   //创建消息队列
		MsgBox_MTtempMS = osMessageCreate(osMessageQ(MsgBox_MTtempMS), NULL);//创建消息队列
		MsgBox_DPtempMS = osMessageCreate(osMessageQ(MsgBox_DPtempMS), NULL);//创建消息队列
		
		memInit_flg = true;
	}

	tempMS_Init();
	tid_tempMS_Thread = osThreadCreate(osThread(tempMS_Thread),NULL);
}
