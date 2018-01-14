#include "kBoard.h"

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

uint8_t  valKeyBoard;            //显示缓存
uint8_t  valKeyBoard_scan;        
uint8_t  key; 

osThreadId tid_kBoard_Thread;
osThreadDef(kBoard_Thread,osPriorityNormal,1,512);

void KEY_RInit(void) //IO初始化
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
}

void KEY_CInit(void) //IO初始化
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		
	GPIO_Init(GPIOC, &GPIO_InitStructure);		
}

uint8_t Eguard_KBScanA(void){ 	
	
	uint8_t  key;
	 
	KEY_RInit();
	GPIO1_KB=0;GPIO2_KB=0;GPIO3_KB=0;GPIO4_KB=0;
	GPIO5_KB=1;GPIO6_KB=1;GPIO7_KB=1;GPIO8_KB=1; 
	delay_us(10);
	if(COL1==0)
	{
		key=1;	
	} 
	if(COL2==0)
	{
		key=2;		
	}    
	if(COL3==0)
	{
		key=3;		
	} 
	if(COL4==0)
	{
		key=4;		
	}
	 
	 //temp = 0;
	KEY_CInit();
	GPIO1_KB=1;GPIO2_KB=1;GPIO3_KB=1;GPIO4_KB=1;
	GPIO5_KB=0;GPIO6_KB=0;GPIO7_KB=0;GPIO8_KB=0; 
	delay_us(10);// 
	if(ROW1==0)
	{
		key=key+0;			
	} 
	if(ROW2==0)
	{
		key=key+4;			
	}    
	if(ROW3==0)
	{
		key=key+8;			
	} 
	if(ROW4==0)
	{
		key=key+12;			
	}

	return key;	    //键值入显示缓存
 }

uint8_t Eguard_KBScanB(void)
{   
	KEY_RInit();
	GPIO1_KB=0;GPIO2_KB=0;GPIO3_KB=0;GPIO4_KB=0;
	GPIO5_KB=1;GPIO6_KB=1;GPIO7_KB=1;GPIO8_KB=1; 
	delay_us(10);
	
	if((COL1==0)|(COL2==0)|(COL3==0)|(COL4==0))
	{
		return Eguard_KBScanA();	
	}else return valKB_NULL;
}

uint8_t Eguard_KBScanC(void){

	u8 key_val;
	static u8 val_old = valKB_NULL;
	const u8 kkeep_time = 120;
	static bool kkeep_FLG = false;
	static u8 KP_cnt = 0;
	
	key_val = Eguard_KBScanB();
	if(key_val != val_old){				//是否为新键值
		
		KP_cnt = 0;
		if(kkeep_FLG)kkeep_FLG = false;
		
		if(key_val != valKB_NULL){		//有按键，记录历史值，用于消抖
		
			 osDelay(10);
			 val_old = key_val;
			 return key_val;
		}else{							//无按键，更新历史值为NULL
		
			 val_old = valKB_NULL;
		}
	}else{								
	
		if(val_old == K_FUN_PGUP){		//键值重复保持处理
		
			if(KP_cnt > kkeep_time){
			
				kkeep_FLG = true;
				return K_FUN_PGUP_LONG;
			}
			else KP_cnt ++;
		}
	}
	
	return valKB_NULL;
}

u8 Eguard_KBvalget(void){

	static bool klong_FLG = false; 
	const u8 valTab_len = 15;
	const u8 valTrue_Tab[valTab_len] = {	//键值二次矫正表
	
		K_VAL0,K_VAL1,K_VAL2,K_VAL3,
		K_VAL4,K_VAL5,K_VAL6,K_VAL7,
		K_VAL8,K_VAL9,
		K_FUN_CLR,
		K_FUN_ENT,
		K_FUN_ESC,
		K_FUN_PGUP,
		K_FUN_PGDN,
	};
	u8 valTemp;
	u8 loop;
	
	valTemp = Eguard_KBScanC();
	
	if((valTemp & 0xF0) == 0xD0){	//长按检测
	
		if(!klong_FLG){
		
			klong_FLG = true;
			return valTemp;
		}else{
		
			return valKB_NULL;		//长按未弹起，返回NULL
		}
	}else 
	if(klong_FLG){					//长按弹起
	
		klong_FLG = false;
		return K_FUN_LONG_relase;
	}
	//非长按，短按处理
	for(loop = 0;loop < valTab_len;loop ++)
		if(valTemp == valTrue_Tab[loop])return loop;
		
	return valKB_NULL;
}

void kBoard_Init(void){

	;
}

void kBoard_Thread(const void *argument){

	const u8 PSD_LEN = 9;
//	char disp[30];
	static u8 key_val;
	static u8 Eguard_Password[PSD_LEN] = {0};
	static u8 Password_temp[PSD_LEN] = {0};
	static u8 PSD_ptr = 0;
	
	const u16 PSD_tempClr_period = 1000;
	u16 PSD_tempClr_cnt;
	
	EGUARD_MEAS *mptr = NULL;
	
	for(;;){
	
		key_val = Eguard_KBvalget();
		
		if(key_val != valKB_NULL){
			
			PSD_tempClr_cnt = PSD_tempClr_period;
		
			if(key_val < 10){
			
				if(PSD_ptr < PSD_LEN - 1)Eguard_Password[PSD_ptr ++] = key_val + '0';
				tips_beep(2,50,3);
			}else{
			
				switch(key_val){
				
					case KBFUN_CLR:	if(PSD_ptr > 0){Eguard_Password[-- PSD_ptr] = 0;tips_beep(5,20,1);}
									break;
						
					case KBFUN_ENT:{
						
										do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL);	//无线数据传输消息推送
										mptr->CMD = PSD_EXERES_TTIT;
										memcpy(mptr->Psd,Eguard_Password,PSD_LEN);
										osMessagePut(MsgBox_EGUD, (uint32_t)mptr, osWaitForever);
										beeps(0);
										memset(Eguard_Password,0,PSD_LEN * sizeof(u8));
										PSD_ptr = 0;
									}break;	
									
					case KBFUN_ESC:	break;

					case KBFUN_PGUP:break;

					case KBFUN_PGDN:{
										
										do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL);	//无线数据传输消息推送
										mptr->CMD = PSD_EXERES_CALL;
										osMessagePut(MsgBox_EGUD, (uint32_t)mptr, osWaitForever);
										
										do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//1.44LCD送显
										mptr -> CMD = PSD_EXERES_CALL;	
										osMessagePut(MsgBox_DPEGUD, (uint32_t)mptr, 100);
										beeps(1);
									}break;
									
					case K_FUN_PGUP_LONG:{
						
										do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL);	//无线数据传输消息推送
										mptr->CMD = PSD_EXERES_LVMSG_DN;
										osMessagePut(MsgBox_EGUD, (uint32_t)mptr, osWaitForever);
										
										do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//1.44LCD送显
										mptr -> CMD = PSD_EXERES_LVMSG_DN;	
										osMessagePut(MsgBox_DPEGUD, (uint32_t)mptr, 100);
										beeps(1);
									}break;
					
					case K_FUN_LONG_relase:{
					
											do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL);	//无线数据传输消息推送
											mptr->CMD = PSD_EXERES_LVMSG_UP;
											osMessagePut(MsgBox_EGUD, (uint32_t)mptr, osWaitForever);
		
											do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL); 	//1.44LCD送显
											mptr -> CMD = PSD_EXERES_LVMSG_UP;	
											osMessagePut(MsgBox_DPEGUD, (uint32_t)mptr, 100);
											beeps(2);
									}break;

					default:break;
				}
			}
		}
		
		if(PSD_tempClr_cnt)PSD_tempClr_cnt --;		//密码缓存清零
		else{
			
			PSD_ptr = 0;
			memset(Eguard_Password,0,PSD_LEN * sizeof(u8));
		}

		if(strcmp((const char*)Password_temp,(const char*)Eguard_Password)){
		
			memcpy(Password_temp,(const char*)Eguard_Password,PSD_LEN);
			
			do{mptr = (EGUARD_MEAS *)osPoolCAlloc(EGUD_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
			mptr->CMD = PSD_EXERES_TTIT;
			memcpy(mptr->Psd,Eguard_Password,PSD_LEN);
			osMessagePut(MsgBox_DPEGUD, (uint32_t)mptr, osWaitForever);
		}
		
//		sprintf(disp,"%d\r\n",key_val);
//		Driver_USART1.Send(disp,strlen(disp));
		
		osDelay(10);
	}
}

void kBoardThread_Active(void){

	kBoard_Init();
	tid_kBoard_Thread = osThreadCreate(osThread(kBoard_Thread),NULL);
}
