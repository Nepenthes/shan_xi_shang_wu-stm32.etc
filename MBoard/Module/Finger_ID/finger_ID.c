#include "finger_ID.h"

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明
extern osThreadId tid_USARTDebug_Thread;

osThreadId tid_fingerID_Thread;
osThreadDef(fingerID_Thread,osPriorityNormal,1,512);

osPoolDef(FIDpool, 16, FID_MEAS);                    // Define memory pool
osPoolId  FIDpool;
osMessageQDef(MsgBox, 16, &FID_MEAS);              // Define message queue
osMessageQId  MsgBox;

const u8 FID_FrameHead_size = 2;
const u8 FID_ADDR_size = 4;

const u8 FID_FrameHead[FID_FrameHead_size] = {

	0xEF,0x01
};

const u8 FID_ADDR[FID_ADDR_size] = {

	0xFF,0xFF,0xFF,0xFF
};

void FID_USARTInitCallback(uint32_t event){

	;
}

void USART1fingerID_Init(void){

	/*Initialize the USART driver */
	Driver_USART1.Initialize(FID_USARTInitCallback);
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
	Driver_USART1.Control (ARM_USART_CONTROL_RX, 1);

	Driver_USART1.Send("i'm usart1 for debug log\r\n", 26);
}

u16 ADD_CHECK(u8 dats[],u8 length){  //和校验

	u8 loop;
	u16	result = 0;
	
	for(loop = 0;loop < length;loop ++){
	
		result += dats[loop];
	}
	return result;
}

u16 frameLoad_TX(u8 bufs[],u8 cmd[],u16 cmdlen){
	
	u8  temp;
	u16 memp;
	
	memp = 0;

	memcpy(&bufs[memp],FID_FrameHead,FID_FrameHead_size); //帧头填充
	memp += FID_FrameHead_size;	//指针后推
	memcpy(&bufs[memp],FID_ADDR,FID_ADDR_size);	//地址填充
	memp += FID_ADDR_size;	//指针后推
	bufs[memp ++] = FID_IDENT_CMD;	//标志填充，推指针
	temp = cmdlen + 2;	//包长+2 2为校验
	bufs[memp ++] = (u8)(temp >> 8);	//包长填充，推指针
	bufs[memp ++] = (u8)temp;
	memcpy(&bufs[memp],cmd,cmdlen);		//指令包填充
	memp += cmdlen;	//指针后推
	temp = ADD_CHECK(&bufs[6],memp - 6); //和校验计算
	bufs[memp ++] = (u8)(temp >> 8);	//和校验填充，推指针
	bufs[memp ++] = (u8)temp;	
	
	return memp;
}

void fingerID_Thread(const void *argument){
	
	FID_MEAS *rptr;
	osEvent  evt;

	const u16 FRAME_SIZE = 40;
	u16 TX_num;
	u8  TX_BUF[FRAME_SIZE];
	u8  datsbuf[FRAME_SIZE - 20];

	osThreadTerminate(tid_USARTDebug_Thread);   //关闭debug，串口外设互斥
	USART1fingerID_Init();
	
	for(;;){
	
		evt = osMessageGet(MsgBox, osWaitForever);
		if (evt.status == osEventMessage) {
		 
			rptr = evt.value.p;
			switch(rptr -> CMD){
			
				case FID_MSGCMD_FIDSAVE:	
		
					memset(TX_BUF,0,FRAME_SIZE * sizeof(u8));
					datsbuf[0] = FID_CMD_GETI;
					TX_num = frameLoad_TX(TX_BUF,datsbuf,1);
					Driver_USART1.Send(TX_BUF,TX_num);				
					break;
					
				case FID_MSGCMD_FIDDELE:		break;
					
				case FID_MSGCMD_FIDIDEN:		break;
			}
		}
		
//		memset(TX_BUF,0,FRAME_SIZE * sizeof(u8));		//调试看frame格式
//		datsbuf[0] = 0x0e;
//		datsbuf[1] = 0x04;
//		datsbuf[2] = 0x0c;
//		TX_num = frameLoad_TX(TX_BUF,datsbuf,3);
//		Driver_USART1.Send(TX_BUF,TX_num);
//		
//		osDelay(1000);
	}
}

void fingerID_Active(void){

	tid_fingerID_Thread	= osThreadCreate(osThread(fingerID_Thread),NULL);
}

