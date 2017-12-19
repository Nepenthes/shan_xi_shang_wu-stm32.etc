#include "Tips.h"

void tipsInit(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);		//使能或者失能APB2外设时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//最高输出速率50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			//推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);						//初始化外设GPIOx寄存器
	GPIO_WriteBit(GPIOE, GPIO_Pin_3 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, Bit_SET);
}

osThreadId tid_tips;

void Task0 (void const *argument);
void Task1 (void const *argument);

osThreadDef(tipsThread,osPriorityNormal,1,512);
osTimerDef(Tim0,TTask0);

/*
 * main: initialize and start the system
 */
void tipsLEDActive(void) {

	tipsInit();
	tid_tips = osThreadCreate(osThread(tipsThread), NULL);
}

void LED_Breath(u8 Obj,u16 cycle,bool type){

	u16 loop;
	
	switch(Obj){
	
		case OBJ_SYS:
			
				for(loop = 0;loop < cycle;loop ++){

					if(type)LED_SYS = 1;
					else LED_SYS = 0;
					delay_us(cycle - loop);
					if(type)LED_SYS = 0;
					else LED_SYS = 1;
					delay_us(loop);
				}break;
				
		case OBJ_MSG:
			
				for(loop = 0;loop < cycle;loop ++){

					if(type)LED_MSG = PEout(3) = 1;
					else LED_MSG = PEout(3) = 0;
					delay_us(cycle - loop);
					if(type)LED_MSG = PEout(3) = 0;
					else LED_MSG = PEout(3) = 1;
					delay_us(loop);
				}break;
				
		case OBJ_EXT:
			
				for(loop = 0;loop < cycle;loop ++){

					if(type)LED_EXT = 1;
					else LED_EXT = 0;
					delay_us(cycle - loop);
					if(type)LED_EXT = 0;
					else LED_EXT = 1;
					delay_us(loop);
				}break;
	}
}

void tipsBoardActive(void){

	const u16 time = 500;
	
	LED_Breath(OBJ_SYS,time,true);
	LED_Breath(OBJ_MSG,time,true);
	LED_Breath(OBJ_EXT,time,true);
	
	LED_Breath(OBJ_SYS,time,false);
	LED_Breath(OBJ_MSG,time,false);
	LED_Breath(OBJ_EXT,time,false);
	
	LED_Breath(OBJ_EXT,time,true);
	LED_Breath(OBJ_MSG,time,true);
	LED_Breath(OBJ_SYS,time,true);
	
	LED_Breath(OBJ_EXT,time,false);
	LED_Breath(OBJ_MSG,time,false);
	LED_Breath(OBJ_SYS,time,false);
}

void tipsThread(void const *argument){
		
	osTimerId Tim_id0;
	osEvent evt;
	
	Tim_id0 = osTimerCreate(osTimer(Tim0), osTimerPeriodic, &TTask0);
	
	tipsBoardActive();
	
	osTimerStart(Tim_id0,3000);
	
	for(;;){
	
		evt = osSignalWait (EVTSIG_SYS, 100);
		evt = osSignalWait (EVTSIG_MSG, 100);
		evt = osSignalWait (EVTSIG_EXT, 100);
		if(evt.status == osEventSignal){
		
			switch(evt.value.signals){
			
				case EVTSIG_SYS:	
						
						LED_Breath(OBJ_SYS,500,true);
						osSignalClear (tid_tips,EVTSIG_SYS);
						break;
				
				case EVTSIG_MSG:	
						
						LED_Breath(OBJ_MSG,500,true);
						osSignalClear (tid_tips,EVTSIG_MSG);
						break;
				
				case EVTSIG_EXT:	
						
						LED_Breath(OBJ_EXT,500,true);
						osSignalClear (tid_tips,EVTSIG_EXT);
						break;
				
				default:break;
			}
		}
		
		osDelay(100);
	}
}

void TTask0(void const *argument){

	osDelay(1000);
}

