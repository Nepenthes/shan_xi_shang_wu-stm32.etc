#include <Key&Tips.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

osThreadId tid_keyTest_Thread;										//按键监测主线程ID
osThreadDef(keyTest_Thread,osPriorityAboveNormal,1,1024);	//按键监测主线程定义

static	uint8_t 	keyOverFlg 	= 	0;				//按键事件结束标志
static	uint16_t	sKeyCount	=	0;				//连续短按	 计数值
static	uint16_t	sKeyKeep		=	0;				//长按并保持 计数值
static	uint8_t	keyCTflg		=  0;				//按键连按标志

/***按键外设初始化***/
void keyInit(void){	

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);		//时钟分配
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;	//端口属性分配
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/***按键键值读取***/
static uint16_t keyScan(void){

	if(!K1)return KEY_VALUE_1;		//键值1
	if(!K2)return KEY_VALUE_2;		//键值2
	return KEY_NULL;					//无按键
}

/***按键检测状态机***/
uint16_t getKey(void){

	static	uint16_t s_u16KeyState 		= KEY_STATE_INIT;		//状态机检测状态，初始化状态
	static	uint16_t	s_u16LastKey		= KEY_NULL;				//保留历史按键键值	
	static	uint8_t	KeyKeepComfirm		= 0;						//长按后确认保持 确认所用时长计时
	static	uint16_t	s_u16KeyTimeCount	= 0;						//长按时长定义（用来对KEY_TICK进行计数，根据这个计数值来确认是否属于长按）
				uint16_t keyTemp				= KEY_NULL;				//十六进制第一位：按键状态；第二位：保持计数值；第三位：键值；第四位：连按计数值
	
	static	uint32_t osTick_last			= 0xffff0000;			//对osTick进行记录，用于与下一次osTick进行对比获取间隔（此间隔用于判断按键是否属于连续按下）
	
	keyTemp = keyScan();		//获取键值
	
	switch(s_u16KeyState){	//获取状态机状态
	
		case KEY_STATE_INIT:	//初始化状态
			
				if(keyCTflg){	//检测上一次是否为连按
				
					if((osKernelSysTick() - osTick_last) > KEY_CONTINUE_PERIOD){	//上一次是连按则检测本次是否继续连按
					
						keyTemp	= s_u16LastKey & 0x00f0;	//本次不是连按，处理返回值为连按结束状态，同时连按标志位清零
						keyTemp |= KEY_CTOVER;
						keyCTflg = 0;	
					}
				}
		
				if(KEY_NULL != keyTemp)s_u16KeyState = KEY_STATE_WOBBLE;	//检测到有按键，切换状态到抖动检测
				break;
		
		case KEY_STATE_WOBBLE:	//消抖状态检测
			
				s_u16KeyState = KEY_STATE_PRESS;	//确认按键，切换状态到短按检测
				break;
		
		case KEY_STATE_PRESS:	//短按状态检测
		
				if(KEY_NULL != keyTemp){		//按键是否弹起？
				
					s_u16LastKey 	= keyTemp;	//存储按键键值
					keyTemp 		  |= KEY_DOWN;	//按键状态确认为按下
					s_u16KeyState	= KEY_STATE_LONG;	//按键依然未弹起，切换状态到长按检测
				}else{
				
					s_u16KeyState	= KEY_STATE_INIT;	//检测为按键抖动，检测状态机返回初始化状态
				}
				break;
				
		case KEY_STATE_LONG:		//长按状态检测
				
				if(KEY_NULL != keyTemp){	//按键是否弹起？
					
					if(++s_u16KeyTimeCount > KEY_LONG_PERIOD){	//若按键依然未弹起，则根据长按时长进行计数继续确认是否为长按
					
						s_u16KeyTimeCount	= 0;			//确认长按，长按计数值清零
						sKeyKeep				= 0;			//检测状态机切换状态前，对保持确认状态所需计数值提前清零，本计数值为长按后保持计数步长
						KeyKeepComfirm		= 0;			//检测状态机切换状态前，对保持确认状态所需计数值提前清零，本数值用于定义长按后多久进行保持计数
						keyTemp			  |= KEY_LONG;	//按键状态确认为长按
						s_u16KeyState		= KEY_STATE_KEEP;	//按键依然未弹起，切换状态到长按保持检测
						
						keyOverFlg			= KEY_OVER_LONG;	//长按
					}else	keyOverFlg	= KEY_OVER_SHORT;		//短按
				}else{
				
					s_u16KeyState		= KEY_STATE_RELEASE;	//检测确认为长按后按键弹起，切换状态到弹起检测
				}
				break;
				
		case KEY_STATE_KEEP:		//长按后保持状态检测
			
				if(KEY_NULL != keyTemp){		//按键是否弹起？
				
					if(++s_u16KeyTimeCount > KEY_KEEP_PERIOD){	//若按键依然未弹起，则根据长按时长进行计数继续确认是否为长按后继续保持
						
						s_u16KeyTimeCount	= 0;			//确认长按后继续保持，长按计数值清零
						if(KeyKeepComfirm < (KEY_COMFIRM + 3))KeyKeepComfirm++;			//检测是否到允许计数时刻
						if(sKeyKeep < 15 && KeyKeepComfirm > KEY_COMFIRM)sKeyKeep++; 	//检测到允许计数，执行长按后保持计数
						if(sKeyKeep){		//检测到长按后保持计数值不为零，即确认按键状态为长按后继续保持，对返回值进行相应确认处理
						
							keyTemp	|= sKeyKeep << 8;		//保持计数数据左移8位放在十六进制keyTemp第二位
							keyTemp	|= KEY_KEEP;			//按键状态确认为长按后继续保持
						}		
						keyOverFlg	 = KEY_OVER_KEEP;
					}
				}else{
				
					s_u16KeyState	= KEY_STATE_RELEASE;	//按键状态确认为长按后继续保持之后弹起，切换状态到弹起检测
				}
				break;
				
		case KEY_STATE_RELEASE:	//弹起状态检测
				
				s_u16LastKey |= KEY_UP;	//存储按键状态
				keyTemp		  = s_u16LastKey;	//按键状态确认为弹起
				s_u16KeyState = KEY_STATE_RECORD;	//切换状态到按键连按记录
				break;
		
		case KEY_STATE_RECORD:	//按键连按记录状态检测

				if((osKernelSysTick() - osTick_last) < KEY_CONTINUE_PERIOD){	//若两次按键弹起时间间隔小于规定值，则判断为连按
					
					sKeyCount++;	//连按计数		
				}else{
					
					sKeyCount = 0;	//连按断开，计数清零
				}
				
				if(sKeyCount){		//若连按次数不为零，即确认为按键连按，对返回值进行相应处理
													
					keyCTflg	= 1;	//打开连按标志
					keyTemp	= s_u16LastKey & 0x00f0;	//提取键值
					keyTemp	|=	KEY_CONTINUE;				//确认为按键连按
					if(sKeyCount < 15)keyTemp += sKeyCount;	//连按计数数据放在十六进制keyTemp第四位（最低位）
				}
				
				s_u16KeyState	= KEY_STATE_INIT;		//检测状态机返回初始状态
				osTick_last	 	= osKernelSysTick();	//记录osTick，留作下次连按检测对比
				break;
		
		default:break;
	}
	return keyTemp;	//返回按键状态和键值
}

/***按键监测主线程***/
void keyTest_Thread(const void *argument){

/***按键调试（串口1反馈调试信息）****/
#if(KEY_DEBUG)
	uint16_t keyVal;						//按键状态事件
	uint8_t	kCount;						//按键计数值变量，长按保持计数和连按计数使用同一个变量，因为两个状态不会同时发生
	static uint8_t	kCount_rec;			//历史计数值保存
	const	 uint8_t	tipsLen = 30;		//Tips打印字符串长度
	
	char	tips[tipsLen];					//Tips字符串
#endif

#if(KEY_DEBUG)
	for(;;){
		
		keyVal = getKey();
		
		memset(tips,0,tipsLen*sizeof(char));	//每轮Tips打印后清空
		strcat(tips,"Tips-");						//Tips标识
		
		switch(keyVal & 0xf000){
		
			case KEY_LONG		:	
				
					switch(keyVal & 0x00f0){
						
						case KEY_VALUE_1:	strcat(tips,"按键1长按\r\n");	
												Driver_USART1.Send(tips,strlen(tips));	
												break;
						
						case KEY_VALUE_2:	strcat(tips,"按键2长按\r\n");	
												Driver_USART1.Send(tips,strlen(tips));	
												break;
						
									default:	break;
					}
					break;
					
			case KEY_KEEP		:
				
					kCount		= (uint8_t)((keyVal & 0x0f00) >> 8) + '0';  //获取计数值并转为ASCII
					kCount_rec	= kCount;
					switch(keyVal & 0x00f0){
						
						case KEY_VALUE_1:	strcat(tips,"按键1长按保持，保持计数：");	
												strcat(tips,(const char*)&kCount);	
												strcat(tips,"\r\n");	
												Driver_USART1.Send(tips,strlen(tips));	
												break;
						
						case KEY_VALUE_2:	strcat(tips,"按键2长按保持，保持计数：");	
												strcat(tips,(const char*)&(kCount));	
												strcat(tips,"\r\n");	
												Driver_USART1.Send(tips,strlen(tips));	
												break;
						
									default:	break;
					}
					break;
					
			case KEY_DOWN		:
				
					switch(keyVal & 0x00f0){
						
						case KEY_VALUE_1:	strcat(tips,"按键1按下\r\n");	
												Driver_USART1.Send(tips,strlen(tips));	
												break;
						
						case KEY_VALUE_2:	strcat(tips,"按键2按下\r\n");	
												Driver_USART1.Send(tips,strlen(tips));
												break;
									default:	break;
					}
					break;
					
			case KEY_UP			:
				
					switch(keyVal & 0x00f0){
						
						case KEY_VALUE_1:	
								
								switch(keyOverFlg){
								
								
									case KEY_OVER_SHORT		:	strcat(tips,"按键1短按后弹起\r\n");	
																		Driver_USART1.Send(tips,strlen(tips));
																		keyOverFlg = 0;
																		break;
									
									case KEY_OVER_LONG		:	strcat(tips,"按键1长按后弹起\r\n");	
																		Driver_USART1.Send(tips,strlen(tips));
																		keyOverFlg = 0;
																		break;
									
									case KEY_OVER_KEEP		:	strcat(tips,"按键1长按后保持");
																		strcat(tips,(const char*)&kCount_rec);
																		strcat(tips,"次计数后结束\r\n");
																		Driver_USART1.Send(tips,strlen(tips));
																		kCount_rec = 0;
																		break;			
									default:break;
								}
								break;
							
						
						
						case KEY_VALUE_2:	
								
								switch(keyOverFlg){
								
								
									case KEY_OVER_SHORT		:	strcat(tips,"按键2短按后弹起\r\n");	
																		Driver_USART1.Send(tips,strlen(tips));
																		keyOverFlg = 0;
																		break;
									
									case KEY_OVER_LONG		:	strcat(tips,"按键2长按后弹起\r\n");	
																		Driver_USART1.Send(tips,strlen(tips));
																		keyOverFlg = 0;
																		break;
									
									case KEY_OVER_KEEP		:	strcat(tips,"按键2长按后保持");
																		strcat(tips,(const char*)&kCount_rec);
																		strcat(tips,"次计数后结束\r\n");
																		Driver_USART1.Send(tips,strlen(tips));
																		kCount_rec = 0;
																		break;	
									default:break;
								}
								break;
								
						default:break;
					}
					break;
					
			case KEY_CONTINUE	:
				
					kCount 		= (uint8_t)((keyVal & 0x000f) >> 0) + '0';	//获取计数值并转为ASCII
					kCount_rec	= kCount + 1;
					switch(keyVal & 0x00f0){
						
						case KEY_VALUE_1:	strcat(tips,"按键1连按，连按计数：");	
												strcat(tips,(const char*)&kCount);	
												strcat(tips,"\r\n");	
												Driver_USART1.Send(tips,strlen(tips));		
												break;
						
						case KEY_VALUE_2:	strcat(tips,"按键2连按，连按计数：");	
												strcat(tips,(const char*)&kCount);	
												strcat(tips,"\r\n");	
												Driver_USART1.Send(tips,strlen(tips));											
												break;
						
									default:	break;
					}
					break;
					
			case KEY_CTOVER	:
				
					switch(keyVal & 0x00f0){
					
					
						case KEY_VALUE_1:	strcat(tips,"按键1连按");
												strcat(tips,(const char*)&kCount_rec);
												strcat(tips,"次后结束\r\n");
												Driver_USART1.Send(tips,strlen(tips));
												kCount_rec = 0;
												break;
						
						case KEY_VALUE_2:	strcat(tips,"按键2连按");
												strcat(tips,(const char*)&kCount_rec);
												strcat(tips,"次后结束\r\n");
												Driver_USART1.Send(tips,strlen(tips));
												kCount_rec = 0;
												break;
						
									default:	break;
					}
					
					
			default:break;
		}
		
		osDelay(KEY_TICK);
	}
#endif
}
	
void keyTest(void){
	
	tid_keyTest_Thread = osThreadCreate(osThread(keyTest_Thread),NULL);
}



