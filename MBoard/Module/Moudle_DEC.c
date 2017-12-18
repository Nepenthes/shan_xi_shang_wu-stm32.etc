#include "Moudle_DEC.h"

Moudle_attr Moudle_GTA;

osThreadId tid_MBDEC_Thread;
osThreadDef(MBDEC_Thread,osPriorityAboveNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

void MoudleDEC_ioInit(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD	| RCC_APB2Periph_GPIOE, ENABLE );	  //使能ADC1通道时钟

	GPIO_InitStructure.GPIO_Pin |= 0xfff8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin |= GPIO_Pin_7 | GPIO_Pin_4;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin |= GPIO_Pin_3;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	PEout(3) = 0;
}

void MoudleDEC_Init(void){
	
	MoudleDEC_ioInit();
	tid_MBDEC_Thread = osThreadCreate(osThread(MBDEC_Thread),NULL);
}

void MBDEC_Thread(const void *argument){	//循环检测
	
	u16 ID_temp,alive_temp;
	static u8 Alive_local = 0xE0;
	const char disp_size = 50;
	char disp[disp_size];
	
	bool M_CHG;
	
	for(;;){
	
		if(!extension_IF)Moudle_GTA.Alive |= extension_FLG;else Moudle_GTA.Alive &= ~extension_FLG;
		if(!wireless_IF) Moudle_GTA.Alive |= wireless_FLG; else Moudle_GTA.Alive &= ~wireless_FLG;
		if(!LCD4_3_IF)	 Moudle_GTA.Alive |= LCD4_3_FLG;   else Moudle_GTA.Alive &= ~LCD4_3_FLG;
		
		osDelay(200);
		
		if(Alive_local != Moudle_GTA.Alive){
		
			osDelay(500);
			if(Alive_local != Moudle_GTA.Alive){
			
				M_CHG = true;
			}
		}
		
		if(M_CHG){
		
			alive_temp = Alive_local ^ Moudle_GTA.Alive;
			Alive_local = Moudle_GTA.Alive;
			
			if(alive_temp & extension_FLG){
			
				if(!extension_IF){
				
					ID_temp = (GPIO_ReadInputData(GPIOD) >> 8) & 0x00ff;
					Moudle_GTA.Extension_ID = (u8)ID_temp;	

					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"扩展模块：0x%02X(ID)已被安装并激活\r\n",Moudle_GTA.Extension_ID);
					Driver_USART1.Send(disp,strlen(disp));
				}else{
				
					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"扩展模块：0x%02X(ID)已被拔除\r\n",Moudle_GTA.Extension_ID);
					Driver_USART1.Send(disp,strlen(disp));
				}
			}
			
			if(alive_temp & wireless_FLG){
			
				if(!wireless_IF){
				
					ID_temp = (GPIO_ReadInputData(GPIOD) >> 3) & 0x001f;
					Moudle_GTA.Wirless_ID = (u8)ID_temp;	

					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"无线通讯模块：0x%02X(ID)已被安装并激活\r\n",Moudle_GTA.Wirless_ID);
					Driver_USART1.Send(disp,strlen(disp));
				}else{
				
					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"无线通讯模块：0x%02X(ID)已被拔除\r\n",Moudle_GTA.Wirless_ID);
					Driver_USART1.Send(disp,strlen(disp));
				}
			}
			
			if(alive_temp & LCD4_3_FLG){
			
				if(!LCD4_3_IF){

					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"显示模块已被安装并激活\r\n");
					Driver_USART1.Send(disp,strlen(disp));
				}else{
				
					osDelay(100);
					memset(disp,0,disp_size * sizeof(char));
					sprintf(disp,"显示模块已被拔除\r\n");
					Driver_USART1.Send(disp,strlen(disp));
				}
			}
		}
	}
}
