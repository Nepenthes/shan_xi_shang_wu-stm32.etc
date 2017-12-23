#include <dataTrans_USART.h>

extern ARM_DRIVER_USART Driver_USART2;

extern osThreadId tid_keyMboard_Thread;	//声明主板按键任务ID，便于传递信息调试使能信号
extern osThreadId tid_keyIFR_Thread;	//声明红外转发扩展板按键任务ID，便于传递信息调试使能信号

osThreadId tid_USARTWireless_Thread;

osThreadDef(USARTWireless_Thread,osPriorityNormal,1,1024);

const u8 dataTransFrameHead_size = 1;
const u8 dataTransFrameHead[dataTransFrameHead_size + 1] = {

	0x7f
};

const u8 dataTransFrameTail_size = 2;
const u8 dataTransFrameTail[dataTransFrameTail_size + 1] = {

	0x0d,0x0a
};

void *memmem(void *start, unsigned int s_len, void *find,unsigned int f_len){
	
	char *p, *q;
	unsigned int len;
	p = start, q = find;
	len = 0;
	while((p - (char *)start + f_len) <= s_len){
			while(*p++ == *q++){
					len++;
					if(len == f_len)
							return(p - f_len);
			};
			q = find;
			len = 0;
	};
	return(NULL);
}

void USART2Wirless_Init(void){
	
	/*Initialize the USART driver */
	Driver_USART2.Initialize(myUSART2_callback);
	/*Power up the USART peripheral */
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 4800 Bits/sec */
	Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS |
									ARM_USART_DATA_BITS_8 |
									ARM_USART_PARITY_NONE |
									ARM_USART_STOP_BITS_1 |
							ARM_USART_FLOW_CONTROL_NONE, 115200);

	/* Enable Receiver and Transmitter lines */
	Driver_USART2.Control (ARM_USART_CONTROL_TX, 1);
	Driver_USART2.Control (ARM_USART_CONTROL_RX, 1);

	Driver_USART2.Send("i'm usart2 for wireless datstransfor\r\n", 38);
}

void myUSART2_callback(uint32_t event){

	;
}

/*****************发送帧数据填装*****************/
//发送帧缓存，命令，模块地址，数据长度，核心数据包，核心数据包长
u16 dataTransFrameLoad_TX(u8 bufs[],u8 cmd,u8 Maddr,u8 dats[],u8 datslen){

	u16 memp;
	
	memp = 0;
	
	memcpy(&bufs[memp],dataTransFrameHead,dataTransFrameHead_size); //帧头填充
	memp += dataTransFrameHead_size;	//指针后推
	bufs[memp ++] = cmd;
	bufs[memp ++] = Maddr;
	bufs[memp ++] = datslen;
	memcpy(&bufs[memp],dats,datslen);
	memp += datslen;
	memcpy(&bufs[memp],dataTransFrameTail,dataTransFrameTail_size);
	memp += dataTransFrameTail_size;
	
	return memp;
}

void USARTWireless_Thread(const void *argument){
	
	osEvent  evt;
	
	bool RX_FLG = false; //有效数据获取标志
	
	const u8 frameDatatrans_totlen = 100;	//帧缓存限长
	const u8 dats_BUFtemp_len = frameDatatrans_totlen - 20;	//核心数据包缓存限长
	u8 dataTrans_TXBUF[frameDatatrans_totlen] = {0};  //发送帧缓存
	u8 dataTrans_RXBUF[frameDatatrans_totlen] = {0};	//接收帧缓存
	u8 TXdats_BUFtemp[dats_BUFtemp_len] = {0};	//发送核心数据包缓存
	u8 RXdats_BUFtemp[dats_BUFtemp_len] = {0};	//接收核心数据包缓存
	u8 memp;
	char *p;
	
//	osSignalWait(WIRLESS_THREAD_EN,osWaitForever);		//等待线程使能信号
//	osSignalClear(tid_USARTWireless_Thread,WIRLESS_THREAD_EN);	
	
	Moudle_GTA.Extension_ID = MID_SENSOR_FID;   /****调试语句*****/
	Moudle_GTA.Wirless_ID = 0xAA;   			/****调试语句*****/
	
	for(;;){
		
		memset(TXdats_BUFtemp, 0, sizeof(u8) * dats_BUFtemp_len);		//所有缓存清零
		memset(RXdats_BUFtemp, 0, sizeof(u8) * dats_BUFtemp_len);
		memset(dataTrans_TXBUF, 0, sizeof(u8) * frameDatatrans_totlen);
		memset(dataTrans_RXBUF, 0, sizeof(u8) * frameDatatrans_totlen);
		memp = 0;
		
		osDelay(20);
		
		memcpy(&RXdats_BUFtemp[memp],dataTransFrameHead,dataTransFrameHead_size);
		memp += dataTransFrameHead_size;
		RXdats_BUFtemp[memp ++] = Moudle_GTA.Wirless_ID;
		RXdats_BUFtemp[memp ++] = datsTransCMD_DOWNLOAD;
		RXdats_BUFtemp[memp ++] = Moudle_GTA.Extension_ID;
		
		osDelay(20);
		
		Driver_USART2.Receive(dataTrans_RXBUF,frameDatatrans_totlen);	
		
		osDelay(50);
		
		p = memmem(dataTrans_RXBUF,frameDatatrans_totlen,RXdats_BUFtemp,memp);	//帧头获取及校验
		
		if(p){
			
			memset(RXdats_BUFtemp, 0, sizeof(u8) * dats_BUFtemp_len);
			memp = dataTransFrameHead_size + 3; //memp 复值为包长游标
			memcpy(RXdats_BUFtemp, (const char*)&p[4 + dataTransFrameHead_size + p[memp]], dataTransFrameTail_size); //取帧尾
			
//			Driver_USART2.Send(RXdats_BUFtemp,2);		/****调试语句*****/
//			osDelay(20);								/****调试语句*****/
			
			if(!memcmp((const char*)RXdats_BUFtemp,dataTransFrameTail,dataTransFrameTail_size)){	//帧尾校验
			
				memset(RXdats_BUFtemp, 0, sizeof(u8) * dats_BUFtemp_len);
				memcpy(RXdats_BUFtemp, (const char*)&p[memp + 1], p[memp]);		//核心数据获取
				RX_FLG = true;
				
//				Driver_USART2.Send(RXdats_BUFtemp,2);		/****调试语句*****/
//				osDelay(20);								/****调试语句*****/
			}
			
			memset(dataTrans_RXBUF, 0, sizeof(u8) * frameDatatrans_totlen);
		}
		
		if(RX_FLG){
			
			RX_FLG = false;
		
			switch(Moudle_GTA.Extension_ID){	//数据接收
			
				case MID_SENSOR_FIRE :	break;
				
				case MID_SENSOR_PYRO :	break;
				
				case MID_SENSOR_SMOKE :	break;
				
				case MID_SENSOR_GAS  :	break;
				
				case MID_SENSOR_TEMP :	break;
				
				case MID_SENSOR_LIGHT:	break;
				
				case MID_SENSOR_SIMU :	break;
				
				case MID_SENSOR_FID :	
					
						{
							FID_MEAS *mptr;
							
							mptr = osPoolAlloc(FID_pool); 
							mptr -> CMD = RXdats_BUFtemp[0];  //下行命令加载
							mptr -> DAT = RXdats_BUFtemp[1];  //下行数据加载
							
							osMessagePut(MsgBox_MTFID, (uint32_t)mptr, osWaitForever);	//指令推送至模块驱动
							osDelay(100);
						}break;
				
				case MID_EXEC_IFR	 :	break;
				
				case MID_EXEC_SOURCE :  break;
				
				
				default:break;
			}
			
			memset(RXdats_BUFtemp, 0, sizeof(u8) * dats_BUFtemp_len); //数据缓存清零
		}
		
		switch(Moudle_GTA.Extension_ID){	//数据发送
		
			case MID_SENSOR_FIRE :	
				
					{
						fireMS_MEAS *rptr;
						
						evt = osMessageGet(MsgBox_fireMS, 100);
					}break;
			
			case MID_SENSOR_PYRO :	
				
					{
						pyroMS_MEAS *rptr;
						
						evt = osMessageGet(MsgBox_pyroMS, 100);
					}break;
			
			case MID_SENSOR_SMOKE :	
				
					{
						smokeMS_MEAS *rptr;
						
						evt = osMessageGet(MsgBox_smokeMS, 100);
					}break;
			
			case MID_SENSOR_GAS  :	
				
					{
						gasMS_MEAS *rptr;
						
						evt = osMessageGet(MsgBox_gasMS, 100);
					}break;
			
			case MID_SENSOR_TEMP :	
				
					{
						tempMS_MEAS *rptr;
						
						evt = osMessageGet(MsgBox_tempMS, 100);
					}break;
			
			case MID_SENSOR_LIGHT:	
				
					{
						lightMS_MEAS *rptr;
						
						evt = osMessageGet(MsgBox_lightMS, 100);
					}break;
			
			case MID_SENSOR_SIMU :	
				
					{
						simuMS_MEAS *rptr;
						
						evt = osMessageGet(MsgBox_simuMS, 100);
					}break;
					
			case MID_SENSOR_FID :

					{
						FID_MEAS *rptr;
						evt = osMessageGet(MsgBox_FID, 100);
						if (evt.status == osEventMessage) {		//等待消息指令
							
							rptr = evt.value.p;
							
							if(rptr -> CMD == FID_EXERES_SUCCESS){
							
								TXdats_BUFtemp[0] = rptr -> CMD;
								TXdats_BUFtemp[1] = rptr -> DAT;
								memp = dataTransFrameLoad_TX(dataTrans_TXBUF,datsTransCMD_UPLOAD,Moudle_GTA.Extension_ID,TXdats_BUFtemp,2);
								Driver_USART2.Send(dataTrans_TXBUF,memp);
								osDelay(20);
								
							}else{
							
							
								
							}
							osPoolFree(FID_pool, rptr); 	//内存释放
						}						
					}break;			
			
			case MID_EXEC_IFR	 :	
				
					
					keyIFRActive();
					break;
			
			case MID_EXEC_SOURCE :  break;
			
			
			default:break;
		}	
	}
}

void USART_WirelessInit(void){

	USART2Wirless_Init();
}

void wirelessThread_Active(void){
	
	USART_WirelessInit();
	tid_USARTWireless_Thread = osThreadCreate(osThread(USARTWireless_Thread),NULL);
}
