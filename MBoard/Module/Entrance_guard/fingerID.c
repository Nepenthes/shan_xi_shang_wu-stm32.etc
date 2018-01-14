#include "fingerID.h"

osThreadId tid_fingerID_Thread;
osThreadDef(fingerID_Thread,osPriorityNormal,1,1024);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明
extern osThreadId tid_USARTDebug_Thread;

/*******指纹识别模块控制数据包缓存格式内容及属性*******/
const u8 FID_FrameHead_size = 2;
const u8 FID_ADDR_size = 4;
const u8 FID_FrameHead[FID_FrameHead_size + 1] = {	//+1方便尾部留\0,便于使用字符串库函数

	0xEF,0x01
};
const u8 FID_ADDR[FID_ADDR_size + 1] = {	

	0xFF,0xFF,0xFF,0xFF
};

/*******指纹识别模块控制指令内容及属性*******/
const u8 FID_CMD_TYPENUM = 20;		//指令包总数
const u8 FID_CMD_SIZEMAX = 20;		//单指令最长长度
const u8 FID_CMD[FID_CMD_TYPENUM][FID_CMD_SIZEMAX] = {		//指令码和指令码后内容都一起存放此缓存

	{0x01},				//指纹图像读取
	{0x02,0x01},		//图像生成特征存放于buffer1
	{0x02,0x02},		//图像生成特征存放于buffer2
	{0x04,0x01,0x00,0x01,0x00,0xff},	//buffer1 图像识别 page1 - page255
	{0x05},				//buffer1与buffer2 合并注册模板
	{0x06,0x01},		//动态复合指令：模板存储，仅存buffer1，后期识别也只识别buffer1   （还差两字节page 码，因为页码数量限制，单字节高八位动态复合指令处理时由0x00填充，而上位机提供 单字节低八位）！！ 
	{0x0c},				//动态复合指令：模板删除，还差两字节的pageID号 和 两字节的数量
	{0x0e,0x04,0x0c},	//模块参数设置
};
const u8 FID_CMDSIZE[FID_CMD_TYPENUM] = {	//指令包内指令对应长度

	1,
	2,
	2,
	6,
	1,
	2,
	1,
	3,
};

const u8 FID_CMDlen = 20;	//动态复合自定义指令限长
u8 FID_CMDptr; 		//动态复合自定义指令游标
u8 FID_CMDusr[];	//动态复合自定义指令缓存

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
	
		result += (u16)dats[loop];
	}
	return result;
}

/*数据缓存包，指令包，指令包长*/
u16 FIDframeLoad_TX(u8 bufs[],u8 cmd[],u16 cmdlen){  //将指令按格式装载进数据缓存包，返回数据包长，指令包含指令码及指令码后、校验码前所有内容
	
	u16 temp;
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
	memcpy(&bufs[memp],cmd,cmdlen);		//指令包填充 ，指令包包含指令码及指令码后、校验码前所有内容
	memp += cmdlen;	//指针后推
	temp = ADD_CHECK(&bufs[6],memp - 6); //和校验计算
	bufs[memp ++] = (u8)(temp >> 8);	//和校验填充，推指针
	bufs[memp ++] = (u8)temp;	

	return memp;	//返回数组游标指针，即包长
}

/******按指令编号发送指令********************/
/******CMD_ID为0xff时，发送动态复合指令******/
EGUARD_MEAS *fingerID_CMDTX(u8 CMD_ID,u8 rpt_num){	//指令编号，重发次数

	const u16 FRAME_SIZE = 100; //数据包缓存长度
	u16 TX_num;		//实际发送数据包长度
	u8  TX_BUF[FRAME_SIZE],RX_BUF[FRAME_SIZE]; //数据包缓存
	u8  datsbuf[FRAME_SIZE - 20]; //指令包缓存
	char  *p_rec;	//接受包缓存指针
	u16 ADD_RES,RX_num;	
	u8  TX_CNT = 0;
	EGUARD_MEAS *result = NULL;
	
	do{result = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(result == NULL);	//申请返回数据类型指针
	
	osDelay(50);
	memset(TX_BUF,0,FRAME_SIZE * sizeof(u8));	//缓存清空
	memset(datsbuf,0,(FRAME_SIZE - 20) * sizeof(u8));
	if(CMD_ID != 0xff){		//是否为动态指令
		
		memcpy(datsbuf,FID_CMD[CMD_ID],FID_CMDSIZE[CMD_ID]);	//指令装填
		TX_num = FIDframeLoad_TX(TX_BUF,datsbuf,FID_CMDSIZE[CMD_ID]);	//指令包装填进数据包缓存
	}else{
	
		memcpy(datsbuf,FID_CMDusr,FID_CMDptr);
		TX_num = FIDframeLoad_TX(TX_BUF,datsbuf,FID_CMDptr);
	}

	do{
		Driver_USART1.Send(TX_BUF,TX_num);	//数据发送
		osDelay(10);
		Driver_USART1.Receive(RX_BUF,FRAME_SIZE);		
		osDelay(100);						
		p_rec = strstr((const char*)RX_BUF,(const char*)FID_FrameHead);
		TX_CNT ++;
		if(p_rec){	//帧头校验
		
			if(p_rec[6] == FID_IDENT_ANS){	//标识校验
			
				RX_num  = 0;
				RX_num |= ((u16)p_rec[7]) << 8;		//取包长
				RX_num |= (u16)p_rec[8];
				
				ADD_RES = ADD_CHECK((u8 *)&p_rec[6],RX_num + 1);
				if(((u8)ADD_RES >> 8) == p_rec[RX_num + 7] && (u8)ADD_RES == p_rec[RX_num + 8]){	//和校验
					
//					Driver_USART2.Send(&p_rec[9],1);	 /****调试语句*****/
//					osDelay(100);						 /****调试语句*****/
				
					if(!p_rec[9]){	//成功识别,数据处理,确认码识别
					
						result -> CMD = FID_EXERES_SUCCESS;
						if(RX_num > 3)result -> DAT = p_rec[11];	//数据长度大于3，预设有限命令内应该是ID读取，page内容为2个字节，因为限位到Page1-255而取低字节
						else result -> DAT = 0;
						osDelay(100);
						return result;
					}	
				}
			}
		}
		memset(RX_BUF,0,FRAME_SIZE * sizeof(u8));	//数据包缓存清空
		osDelay(100);	
	}while(TX_CNT < rpt_num);	//重复次数用尽
	
	result -> CMD = FID_EXERES_FAIL;	//识别失败
	result -> DAT = 0;
	osDelay(100);
	return result;
}

void fingerID_Thread(const void *argument){
	
	EGUARD_MEAS *mptr = NULL;		//发送消息队列缓存
	EGUARD_MEAS *rptr = NULL;		//接收消息队列缓存
	EGUARD_MEAS *sptr = NULL;		//指纹指令缓存
	osEvent   evt;	 	//事件缓存
	osStatus  status;
	
	u8 loop;
	const u8 cmdQ_fidSave_len = 5;
	const u8 cmdQ_fidIden_len = 3;
	const u8 cmdQ_fidSave[cmdQ_fidSave_len] = {0,1,0,2,4};	//指纹存储指令队列，5条静态指令 + 1条动态复合指令，/**指纹删除仅一条动态复合指令，无静态指令表**/
	const u8 cmdQ_fidIden[cmdQ_fidIden_len] = {0,1,3}; //指纹识别指令队列，3条静态指令
	const u8 TX_rept = 2;
	
	bool cmd_continue = false;

	osThreadTerminate(tid_USARTDebug_Thread);   //关闭debug，串口外设互斥
	USART1fingerID_Init();	//重新初始化
	
	for(;;){
	
		evt = osMessageGet(MsgBox_MTEGUD_FID, 200);
		if (evt.status == osEventMessage){		//等待消息指令
		 
			rptr = evt.value.p;
			switch(rptr -> CMD){
			
				case FID_MSGCMD_FIDSAVE:	//执行指纹存储指令
					
						do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//外发消息内存申请
					
						osDelay(3000);
						sptr = fingerID_CMDTX(0,20);
						if(sptr -> CMD == FID_EXERES_SUCCESS){		//扫描20次是否有手指取结果
							
							do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
							sptr = NULL;							
							osDelay(100);
							for(loop = 0;loop < cmdQ_fidSave_len;loop ++){
							
								//-----------------------------------------------------------//（按表发送静态指令）
								sptr = fingerID_CMDTX(cmdQ_fidSave[loop],TX_rept );	//静态指令队列发送
								if(sptr -> CMD == FID_EXERES_SUCCESS){		//取结果
									
									if(loop == cmdQ_fidSave_len - 1)cmd_continue = true;	//前面四条静态指令都被成功执行，则使能最后一条动态复合指令
									osDelay(500);
								}else{
									
									osDelay(200);
									mptr -> CMD = FID_EXERES_FAIL;								
									mptr -> DAT = 0x00;	
									osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
									do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
									sptr = NULL;	
									break;
								}
								do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
								sptr = NULL;
							}
							
							if(cmd_continue){	//执行最后一条动态复合指令
								
								cmd_continue = false;
							
								//-----------------------------------------------------------//(动态复合指令)
								memset(FID_CMDusr, 0, FID_CMDlen * sizeof(u8));	//缓存清空
								memcpy(FID_CMDusr,FID_CMD[5],FID_CMDSIZE[5]);	//指令表指令复制
								FID_CMDusr[FID_CMDSIZE[5]] = 0x00;	//因为页码数量限制，page页码高八位字节0填充
								FID_CMDusr[FID_CMDSIZE[5] + 1] = rptr -> DAT;	//上位机只提供单字节低八位
								FID_CMDptr = FID_CMDSIZE[5] + 2; //指令长度 +2
								sptr = fingerID_CMDTX(0xff,TX_rept);	//发送自定义动态指令返回结果
								if(sptr -> CMD == FID_EXERES_SUCCESS){	//取结果，同时整列指令队列成功执行完毕
								
									mptr -> CMD = FID_EXERES_SUCCESS;	//无数据反馈，仅填充结果，数据内容0x00填充
									mptr -> DAT = 0x00;		
									osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
								}else{
									
									mptr -> CMD = FID_EXERES_FAIL;
									mptr -> DAT = 0x00;	
									osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
								}osDelay(200);
								do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
								sptr = NULL;
							}
						}else{
							
							osDelay(200);
							mptr -> CMD = FID_EXERES_FAIL;								
							mptr -> DAT = 0x00;	
							osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
							do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
							sptr = NULL;
							break;
						}
						osPoolFree(EGUD_pool, sptr);	//释放指纹指令包内存
						sptr = NULL;
						break;	   
					
				case FID_MSGCMD_FIDDELE:		//执行指纹删除指令
					
						do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//外发消息内存申请
					
						//-----------------------------------------------------------//(动态复合指令)
						memset(FID_CMDusr, 0, FID_CMDlen * sizeof(u8));	//缓存清空
						memcpy(FID_CMDusr,FID_CMD[6],FID_CMDSIZE[6]);	//指令表指令复制
						FID_CMDusr[FID_CMDSIZE[6]] = 0x00;	//因为页码数量限制，page页码高八位字节0填充
						FID_CMDusr[FID_CMDSIZE[6] + 1] = rptr -> DAT;	//上位机只提供单字节低八位
						FID_CMDusr[FID_CMDSIZE[6] + 2] = 0x00;	//指令简化，只删除指定pageID，即删除数量为1
						FID_CMDusr[FID_CMDSIZE[6] + 3] = 0x01;
						FID_CMDptr = FID_CMDSIZE[6] + 4;	//指令长度 +4
						sptr = fingerID_CMDTX(0xff,TX_rept);	//发送自定义动态指令返回结果
						if(sptr -> CMD == FID_EXERES_SUCCESS){	//取结果，同时整列指令队列成功执行完毕
						
							mptr -> CMD = FID_EXERES_SUCCESS;	//无数据反馈，仅填充结果，数据内容0x00填充
							mptr -> DAT = 0x00;				
						}else{
							
							mptr -> CMD = FID_EXERES_FAIL;
							mptr -> DAT = 0x00;								
						}
						osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
						do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);	//释放指纹指令包内存
						sptr = NULL;
						sptr = fingerID_CMDTX(0,1);			//二次无意义调用，冲洗脏指针
						do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);	//释放指纹指令包内存
						sptr = NULL;
						break;
					
				case FID_MSGCMD_FIDIDEN:		//执行指纹识别指令队列
					
						do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//外发消息内存申请
				
						for(loop = 0;loop < cmdQ_fidIden_len;loop ++){
						
							//-----------------------------------------------------------//（按表发送静态指令）
							sptr = fingerID_CMDTX(cmdQ_fidIden[loop],TX_rept );	//静态指令队列发送
							if(sptr -> CMD == FID_EXERES_SUCCESS){//取结果
								
								if(loop == cmdQ_fidIden_len - 1){	//整列指令队列成功执行完毕
								
									mptr -> CMD = FID_EXERES_SUCCESS;	//有数据反馈
									mptr -> DAT = sptr -> DAT;
									osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
								}
								osDelay(200);
							}else{
		
								mptr -> CMD = FID_EXERES_FAIL;
								mptr -> DAT = 0x00;	
								osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
								do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);	//释放指纹指令包内存
								sptr = NULL;
								break;
							}osDelay(500);
							
							do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);	//释放指纹指令包内存
							sptr = NULL;
						}
						break;
						
				default:break;
			}
			osPoolFree(EGUD_pool, rptr);	//释放消息队列内存
		}
		/*上位机无主动指令，被动执行周期循环检测*/
		
		sptr = fingerID_CMDTX(0,1);
		if(sptr -> CMD == FID_EXERES_SUCCESS){
			
			do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
			sptr = NULL;
	
			for(loop = 0;loop < cmdQ_fidIden_len;loop ++){
			
				//-----------------------------------------------------------//（按表发送静态指令）
				sptr = fingerID_CMDTX(cmdQ_fidIden[loop],TX_rept );	//静态指令队列发送
				if(sptr -> CMD == FID_EXERES_SUCCESS){//取结果
					
					if(loop == cmdQ_fidIden_len - 1){	//整列指令队列成功执行完毕
						
						do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//外发消息内存申请
						mptr -> CMD = FID_EXERES_TTIT;	//有数据反馈,主动上传
						mptr -> DAT = sptr -> DAT;
						osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
						
						do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//1.44LCD送显
						mptr -> CMD = FID_EXERES_TTIT;	
						mptr -> DAT = sptr -> DAT;
						osMessagePut(MsgBox_DPEGUD, (uint32_t)mptr, 100);
						
						beeps(2);
					}
					osDelay(200);
				}else{					//主动周期检测，失败无动作
					
//					do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//外发消息内存申请
//					mptr -> CMD = FID_EXERES_FAIL;
//					mptr -> DAT = 0x00;	
//					osMessagePut(MsgBox_EGUD, (uint32_t)mptr, 100);
					do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
					sptr = NULL;
					break;
				}osDelay(500);
				
				do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK);
				sptr = NULL;
			}
		}else {do{status = osPoolFree(EGUD_pool, sptr);}while(status != osOK); sptr = NULL;}
		osDelay(20);
	}
}

void fingerIDThread_Active(void){
	
	tid_fingerID_Thread	= osThreadCreate(osThread(fingerID_Thread),NULL);
}

