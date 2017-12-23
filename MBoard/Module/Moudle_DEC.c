#include "Moudle_DEC.h"
#include "Tips.h"

Moudle_attr Moudle_GTA;

osThreadId tid_MBDEC_Thread;
osThreadDef(MBDEC_Thread,osPriorityNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;		//设备驱动库串口一设备声明

void MoudleDEC_ioInit(void){		//模块检测脚初始化

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD	| RCC_APB2Periph_GPIOE, ENABLE );	  //使能ADC1通道时钟

	GPIO_InitStructure.GPIO_Pin |= 0xfff8;				//硬件ID检测引脚初始化
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin |= GPIO_Pin_7 | GPIO_Pin_4;		//Moudle_Check引脚初始化
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

void MBDEC_Thread(const void *argument){	//循环检测
	
	const u16 signel_waitTime = 1500;
	
	u16 ID_temp,alive_temp;
	static u8 Alive_local = 0x00;	
	const char disp_size = 50;
	char disp[disp_size];
	
	bool M_CHG;
	
	osSignalWait(EVTSIG_MDEC_EN,osWaitForever);			//底板初始化加载等待
	osSignalClear(tid_MBDEC_Thread,EVTSIG_MDEC_EN);		
	
	for(;;){
	
		if(extension_IF){
			
			Moudle_GTA.Alive |= extension_FLG;
		}else Moudle_GTA.Alive &= ~extension_FLG;	//扩展模块检测  
		if(wireless_IF){
		
			Moudle_GTA.Alive |= wireless_FLG;
		}else Moudle_GTA.Alive &= ~wireless_FLG;	//无线通信模块检测
		if(LCD4_3_IF){
		
			Moudle_GTA.Alive |= LCD4_3_FLG;
		}else Moudle_GTA.Alive &= ~LCD4_3_FLG; 		//4.3寸LCD模块检测
		
		osDelay(200);
		
		if(Alive_local != Moudle_GTA.Alive){	//是否有模块插拔，消抖
		
			osDelay(500);
			if(Alive_local != Moudle_GTA.Alive){
			
				M_CHG = true;
			}
		}
		
		if(M_CHG){
			
			M_CHG = false;
		
			alive_temp = Alive_local ^ Moudle_GTA.Alive;	//获取插拔变动模块
			Alive_local = Moudle_GTA.Alive;
			
			if(alive_temp & wireless_FLG){
			
				if(wireless_IF){
					
					ID_temp = (GPIO_ReadInputData(GPIOD) >> 3) & 0x001f;		//无线通信模块ID读取
					Moudle_GTA.Wirless_ID = (u8)ID_temp;
					
					osSignalSet (tid_tips, EVTSIG_MSG_A);
					osDelay(signel_waitTime);
					
					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"无线通讯模块：0x%02X(ID)已被安装并激活\r\n",Moudle_GTA.Wirless_ID);
					Driver_USART1.Send(disp,strlen(disp));
					
					switch(Moudle_GTA.Wirless_ID){				//通讯模块驱动激活
					
						case MID_TRANS_Zigbee	:	break;
							
						case MID_TRANS_Wifi		:	break;
							
						default:break;
					}
					
					wirelessThread_Active();	//无线传输线程启动（等待使能信号激活）
					
				}else{
					
					osSignalSet (tid_tips, EVTSIG_MSG_B);
					osDelay(signel_waitTime);
																	
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"无线通讯模块：0x%02X(ID)已被拔除\r\n",Moudle_GTA.Wirless_ID);
					Driver_USART1.Send(disp,strlen(disp));
					
					switch(Moudle_GTA.Wirless_ID){				//通讯模块驱动终止
					
						case MID_TRANS_Zigbee	:	break;
							
						case MID_TRANS_Wifi		:	break;
							
						default:break;
					}
					
					osThreadTerminate(tid_keyIFR_Thread);  //无线传输线程终止
				}
			}
			
			if(alive_temp & extension_FLG){
			
				if(extension_IF){
					
					ID_temp = (GPIO_ReadInputData(GPIOD) >> 8) & 0x00ff;		//扩展模块ID读取
					Moudle_GTA.Extension_ID = (u8)ID_temp;	
					
					osSignalSet (tid_tips, EVTSIG_EXT_A);
					osDelay(signel_waitTime);

					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"扩展模块：0x%02X(ID)已被安装并激活\r\n",Moudle_GTA.Extension_ID);
					Driver_USART1.Send(disp,strlen(disp));
					
					switch(Moudle_GTA.Extension_ID){	//卡槽一，扩展模块驱动激活
					
						case MID_SENSOR_FIRE :	break;
						
						case MID_SENSOR_PYRO :	break;
						
						case MID_SENSOR_SMOKE :	break;
						
						case MID_SENSOR_GAS  :	break;
						
						case MID_SENSOR_TEMP :	break;
						
						case MID_SENSOR_LIGHT:	break;
						
						case MID_SENSOR_SIMU :	break;
						
						case MID_SENSOR_FID :	
							
								fingerID_Active();
								break;
						
						case MID_EXEC_IFR	 :	
							
								keyIFRActive();
								osSignalSet (tid_USARTDebug_Thread, USARTDEBUG_THREAD_EN);
								break;
						
						case MID_EXEC_SOURCE :  break;
						
						
						default:break;
					}
					
					osSignalSet (tid_USARTWireless_Thread, WIRLESS_THREAD_EN); //激活无线传输线程
					
				}else{
					
					osSignalSet (tid_tips, EVTSIG_EXT_B);
					osDelay(signel_waitTime);
					
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"扩展模块：0x%02X(ID)已被拔除\r\n",Moudle_GTA.Extension_ID);
					Driver_USART1.Send(disp,strlen(disp));
					
					switch(Moudle_GTA.Extension_ID){	//卡槽一，扩展模块驱动中止
					
						case MID_SENSOR_FIRE :	break;
						
						case MID_SENSOR_PYRO :	break;
						
						case MID_SENSOR_SMOKE :	break;
						
						case MID_SENSOR_GAS  :	break;
						
						case MID_SENSOR_TEMP :	break;
						
						case MID_SENSOR_LIGHT:	break;
						
						case MID_SENSOR_SIMU :	break;
						
						case MID_SENSOR_FID :	
							
								osThreadTerminate(tid_fingerID_Thread); 
								break;
						
						case MID_EXEC_IFR	 :	
							
								osThreadTerminate(tid_keyIFR_Thread); 
								break;
						
						case MID_EXEC_SOURCE :  break;
						
						
						default:break;
					}
				}
			}
			
			if(alive_temp & LCD4_3_FLG){	//显示模块无ID，直接检测是否在线
			
				if(LCD4_3_IF){

					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"显示模块已被安装并激活\r\n");
					Driver_USART1.Send(disp,strlen(disp));
				}else{
					
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"显示模块已被拔除\r\n");
					Driver_USART1.Send(disp,strlen(disp));
				}
			}
		}
	}
}

void MoudleDEC_Init(void){	//模块检测线程激活
	
	MoudleDEC_ioInit();
	tid_MBDEC_Thread = osThreadCreate(osThread(MBDEC_Thread),NULL);
}
