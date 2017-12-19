#include "finger_ID.h"

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明
extern osThreadId tid_USARTDebug_Thread;

osThreadId tid_fingerID_Thread;
osThreadDef(fingerID_Thread,osPriorityNormal,1,512);

osPoolDef(FIDpool, 16, FID_MEAS);                    // 内存池定义
osPoolId  FIDpool;									 //ID
osMessageQDef(MsgBox, 16, &FID_MEAS);              // 消息队列定义
osMessageQId  MsgBox;							   //ID

/*******指纹识别模块控制数据包缓存格式内容及属性*******/
const u8 FID_FrameHead_size = 2;
const u8 FID_ADDR_size = 4;
const u8 FID_FrameHead[FID_FrameHead_size + 1] = {

	0xEF,0x01
};
const u8 FID_ADDR[FID_ADDR_size + 1] = {

	0xFF,0xFF,0xFF,0xFF
};

/*******指纹识别模块控制指令内容及属性*******/
const u8 FID_CMD_TYPENUM = 20;
const u8 FID_CMD_SIZEMAX = 20;
const u8 FID_CMD[FID_CMD_TYPENUM][FID_CMD_SIZEMAX] = {

	{0x01},
	{0x02},
	{0x02},
	{0x0e,0x04,0x0c},
};
const u8 FID_CMDSIZE[FID_CMD_TYPENUM] = {

	1,
	1,
	1,
	3,
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

/*数据缓存包，指令包，指令包长*/
u16 frameLoad_TX(u8 bufs[],u8 cmd[],u16 cmdlen){  //按格式装载数据缓存包，返回数据包长
	
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

FID_MEAS *fingerID_CMDTX(u8 CMD_ID,u8 rpt_num){	//指令编号，重发次数

	const u16 FRAME_SIZE = 100; //数据包缓存长度
	u16 TX_num;		//实际发送数据包长度
	u8  TX_BUF[FRAME_SIZE],RX_BUF[FRAME_SIZE]; //数据包缓存
	u8  datsbuf[FRAME_SIZE - 20]; //指令包缓存
	char  *p_rec;	//接受包缓存指针
	u16 ADD_RES,RX_num;	
	u8  TX_CNT = 0;
	FID_MEAS *result;
	
	result = osPoolAlloc(FIDpool);
	
	osDelay(100);
	memset(TX_BUF,0,FRAME_SIZE * sizeof(u8));	//缓存清空
	memset(datsbuf,0,(FRAME_SIZE - 20) * sizeof(u8));
	memcpy(datsbuf,FID_CMD[CMD_ID],FID_CMDSIZE[CMD_ID]);	//指令装填
	TX_num = frameLoad_TX(TX_BUF,datsbuf,FID_CMDSIZE[CMD_ID]);	//指令包装填进数据包缓存
	
	do{
		
		Driver_USART1.Send(TX_BUF,TX_num);	//数据发送
		osDelay(20);
		Driver_USART1.Receive(RX_BUF,FRAME_SIZE);		
		osDelay(100);						
		p_rec = strstr((const char*)RX_BUF,(const char*)FID_FrameHead);
		TX_CNT ++;
		if(p_rec){	//帧头校验
		
			if(p_rec[6] == FID_IDENT_ANS){	//标识校验
			
				RX_num |= ((u16)p_rec[7]) << 8;		//取包长
				RX_num |= (u16)p_rec[8];
				
				ADD_RES = ADD_CHECK((u8 *)&p_rec[6],RX_num + 1);
				if(((u8)ADD_RES >> 8) == p_rec[RX_num + 7] && (u8)ADD_RES == p_rec[RX_num + 8]){	//和校验
				
					result -> CMD = 0xAA;	//成功识别
					return result;
				}
			}
		}
		memset(RX_BUF,0,FRAME_SIZE * sizeof(u8));	//数据包缓存清空	
	}while(TX_CNT < rpt_num);	
	
	memset(TX_BUF,0,FRAME_SIZE * sizeof(u8));	//数据包缓存清空
	
	result -> CMD = 0xBB;	//识别失败
	return result;
}

void fingerID_Thread(const void *argument){
	
	FID_MEAS *rptr;		//消息队列缓存
	osEvent  evt;	//事件缓存

	osThreadTerminate(tid_USARTDebug_Thread);   //关闭debug，串口外设互斥
	USART1fingerID_Init();	//重新初始化
	
	rptr = osPoolAlloc(FIDpool); 	//内存申请
	
	for(;;){
	
		evt = osMessageGet(MsgBox, osWaitForever);
		if (evt.status == osEventMessage) {		//等待消息指令
		 
			rptr = evt.value.p;
			switch(rptr -> CMD){
			
				case FID_MSGCMD_FIDSAVE:

						rptr = fingerID_CMDTX(0,5);	//发送0号指令返回结果
						if(rptr -> CMD == 0xAA){	//取结果

							osPoolFree(FIDpool, rptr);	//释放内存
							while(1);
						}
						osPoolFree(FIDpool, rptr);	//释放内存
						break;	   
					
				case FID_MSGCMD_FIDDELE:		break;
					
				case FID_MSGCMD_FIDIDEN:		break;
			}
		}
	}
}

void fingerID_Active(void){

	FIDpool = osPoolCreate(osPool(FIDpool));	//创建内存池
	MsgBox = osMessageCreate(osMessageQ(MsgBox), NULL);	//创建消息队列
	tid_fingerID_Thread	= osThreadCreate(osThread(fingerID_Thread),NULL);
}

