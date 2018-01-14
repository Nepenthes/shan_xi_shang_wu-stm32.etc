#include "Tips.h"
#include "Moudle_DEC.h"

const u8 spect_size = 15;
const u8 spect_len[spect_size] = {2,2,2,4,4,4,7,7,7,6,5};
const u8 spect[spect_size][8] = {

	{1,2},
	{1,4},
	{1,7},
	{3,3,4,5},
	{5,4,3,2},
	{7,2,3,1},
	{5,5,6,4,3,1,5},
	{7,7,5,6,1,2,4},
	{6,6,2,3,1,4,7},
	{1,2,5,6,3,4},
	{1,2,1,2,6},
};

/****音调，时间，音量(十级)****/
void tips_beep(u8 tones, u16 time, u8 vol){

	const u8 vol_fa = 10;	//音量分级
	u16 tones_base = tones * 50 + 100;  //周期时间 = 音调倍乘 + 音调基数 
	u16 cycle = time * 1000 / tones_base;	//循环次数 = 总时间 / 频率
	u16 loop;

	for(loop = 0;loop < cycle;loop ++){
	
		PEout(6) = 1;
		delay_us(tones_base / vol_fa * vol);
		PEout(6) = 0;
		delay_us(tones_base / vol_fa * (vol_fa - vol));
	}
}

void beeps(u8 num){

	u8 loop;
	
	for(loop = 0;loop < spect_len[num];loop ++)
		tips_beep(spect[num][loop],50,3);
}

void tipsInit(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);		//使能或者失能APB2外设时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_9 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	//底板指示灯
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//最高输出速率50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			//推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);						//初始化外设GPIOx寄存器
	GPIO_WriteBit(GPIOE, GPIO_Pin_3 | GPIO_Pin_9 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15, Bit_SET);
	GPIO_WriteBit(GPIOE, GPIO_Pin_6, Bit_RESET);
}

osThreadId tid_tips;

void Task0 (void const *argument);
void Task1 (void const *argument);

osThreadDef(tipsThread,osPriorityNormal,1,512);
osTimerDef(Tim0,TTask0);

/******呼吸效果：LED对象，周期，呼吸类型（灭亮，亮灭）******/
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

					if(type)LED_MSG_N = LED_MSG = 1;
					else LED_MSG_N = LED_MSG = 0;
					delay_us(cycle - loop);
					if(type)LED_MSG_N = LED_MSG = 0;
					else LED_MSG_N = LED_MSG = 1;
					delay_us(loop);
				}break;
				
		case OBJ_EXT:
			
				for(loop = 0;loop < cycle;loop ++){

					if(type)LED_EXT_N = LED_EXT = 1;
					else LED_EXT_N = LED_EXT = 0;
					delay_us(cycle - loop);
					if(type)LED_EXT_N = LED_EXT = 0;
					else LED_EXT_N = LED_EXT = 1;
					delay_us(loop);
				}break;
			
		default:break;
	}
}

/******闪烁效果：LED对象，周期，呼吸类型（灭亮，亮灭），闪烁次数******/
void LED_Flash(u8 Obj,u16 cycle,bool type,u8 time){

	u16 loop;
	
	switch(Obj){
	
		case OBJ_SYS:
			
				for(loop = 1;loop < time;loop ++){

					if(type)LED_SYS = 0;
					else LED_SYS = 1;
					osDelay(cycle);
					if(type)LED_SYS = 1;
					else LED_SYS = 0;
					osDelay(cycle);
				}break;
				
		case OBJ_MSG:
			
				for(loop = 1;loop < time;loop ++){

					if(type)LED_MSG_N = LED_MSG = 0;
					else LED_MSG_N = LED_MSG = 1;
					osDelay(cycle);
					if(type)LED_MSG_N = LED_MSG = 1;
					else LED_MSG_N = LED_MSG = 0;
					osDelay(cycle);
				}break;
				
		case OBJ_EXT:
			
				for(loop = 1;loop < time;loop ++){

					if(type)LED_EXT_N = LED_EXT = 0;
					else LED_EXT_N = LED_EXT = 1;
					osDelay(cycle);
					if(type)LED_EXT_N = LED_EXT = 1;
					else LED_EXT_N = LED_EXT = 0;
					osDelay(cycle);
				}break;
		
		default:break;
	}
}

void tipsBoardActive(void){

	const u16 time = 400;
	
	LED_Breath(OBJ_SYS,time,true);
	LED_Breath(OBJ_MSG,time,true);
	LED_Breath(OBJ_EXT,time,true);
	
	LED_Breath(OBJ_SYS,time,false);
	LED_Breath(OBJ_MSG,time,false);
	LED_Breath(OBJ_EXT,time,false);
	
	osDelay(300);
	
	LED_Breath(OBJ_EXT,time,true);
	LED_Breath(OBJ_MSG,time,true);
	LED_Breath(OBJ_SYS,time,true);
	
	LED_Breath(OBJ_EXT,time,false);
	LED_Breath(OBJ_MSG,time,false);
	LED_Breath(OBJ_SYS,time,false);
}

void tipsThread(void const *argument){
	
	const u16 flash_cycle  = 100;
	const u8  flash_time   = 3;
	const u16 time_wait    = 500;
	const u16 breath_cycle = 500;
		
	osTimerId Tim_id0;
	osEvent evt;
	
	Tim_id0 = osTimerCreate(osTimer(Tim0), osTimerPeriodic, NULL);
	osTimerStart(Tim_id0,3000);
	
	//tipsBoardActive();
	
	osSignalSet(tid_MBDEC_Thread, EVTSIG_MDEC_EN);
	
	for(;;){
	
		evt = osSignalWait (0, 100);
		if(evt.status == osEventSignal){
		
			switch(evt.value.signals){
			
				case EVTSIG_SYS_A:			//系统状态指示A
						
						LED_Flash(OBJ_SYS,flash_cycle,true,flash_time);
						osDelay(time_wait);
						LED_Breath(OBJ_SYS,breath_cycle,true);
						osSignalClear (tid_tips,EVTSIG_SYS_A);
						break;
				
				case EVTSIG_SYS_B:	
						
						LED_Flash(OBJ_SYS,flash_cycle,false,flash_time);
						osDelay(time_wait);
						LED_Breath(OBJ_SYS,breath_cycle,false);
						osSignalClear (tid_tips,EVTSIG_SYS_B);
						break;
				
				case EVTSIG_MSG_A:			//通讯模块拆装提示
						
						LED_Flash(OBJ_MSG,flash_cycle,true,flash_time);
						osDelay(time_wait);
						LED_Breath(OBJ_MSG,breath_cycle,true);
						osSignalClear (tid_tips,EVTSIG_MSG_A);
						break;
				
				case EVTSIG_MSG_B:	
						
						LED_Flash(OBJ_MSG,flash_cycle,false,flash_time);
						osDelay(time_wait);
						LED_Breath(OBJ_MSG,breath_cycle,false);
						osSignalClear (tid_tips,EVTSIG_MSG_B);
						break;
				
				case EVTSIG_EXT_A:			//扩展模块拆装提示
						
						LED_Flash(OBJ_EXT,flash_cycle,true,flash_time);
						osDelay(time_wait);
						LED_Breath(OBJ_EXT,breath_cycle,true);
						osSignalClear (tid_tips,EVTSIG_EXT_A);
						break;

				case EVTSIG_EXT_B:	
						
						LED_Flash(OBJ_EXT,flash_cycle,false,flash_time);
						osDelay(time_wait);
						LED_Breath(OBJ_EXT,breath_cycle,false);
						osSignalClear (tid_tips,EVTSIG_EXT_B);
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

void tipsLEDActive(void) {

	tipsInit();
	tid_tips = osThreadCreate(osThread(tipsThread), NULL);
	//beeps(9);
}

