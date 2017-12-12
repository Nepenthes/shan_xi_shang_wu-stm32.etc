#include "Test.h"

void testInit (void){

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);		//使能或者失能APB2外设时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//最高输出速率50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			//推挽输出
	GPIO_Init(GPIOE, &GPIO_InitStructure);							//初始化外设GPIOx寄存器
	GPIO_WriteBit(GPIOE, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, Bit_SET);
}

osThreadId tid1,tid2,tid3;

void Task0 (void const *argument);
void Task1 (void const *argument);

void TTask0 (void const *argument);
void TTask1 (void const *argument);

osThreadDef(Task0,osPriorityNormal,1,0);
osTimerDef(Tim0,TTask0);

/*
 * main: initialize and start the system
 */
void LEDTest (void) {
	
	testInit ();
	
	tid1	=	osThreadCreate(osThread(Task0), NULL);
	tid2	=	osThreadCreate(osThread(Task0), NULL);
	tid3	=	osThreadCreate(osThread(Task0), NULL);
}

void Task0 (void const *argument){
	
	osTimerId	 Tim_id0;
	
	Tim_id0 = osTimerCreate(osTimer(Tim0), osTimerPeriodic, &TTask0);
	
	osTimerStart(Tim_id0,3000);
	
	while(1){
	
		LED1_0;
		osDelay(200);
		LED1_1;
		osDelay(100);
	}
}

void Task1 (void const *argument){
	
	while(1){
	
		LED2_0;
		osDelay(200);
		LED2_1;
		osDelay(100);
	}
}
void TTask0(void const *argument){

	unsigned char cnt = 5;
	while(--cnt){
	
		LED2_0;
		osDelay(200);
		LED2_1;
		osDelay(100);
	}
	cnt = 5;
	while(--cnt){
	
		LED3_0;
		osDelay(100);
		LED3_1;
		osDelay(200);
	}
}

