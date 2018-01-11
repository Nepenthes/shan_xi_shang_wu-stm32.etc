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

uint8_t keyBDscan(void){ 	
	
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

uint8_t valKB_get(void)
{   
	KEY_RInit();
	GPIO1_KB=0;GPIO2_KB=0;GPIO3_KB=0;GPIO4_KB=0;
	GPIO5_KB=1;GPIO6_KB=1;GPIO7_KB=1;GPIO8_KB=1; 
	delay_us(10);
	
	if((COL1==0)|(COL2==0)|(COL3==0)|(COL4==0))
	{
		return keyBDscan();	
	}else return valKB_NULL;
}

u8 test(void){

	u8 key_val;
	static u8 val_old = valKB_NULL;
	
	key_val = valKB_get();
	if(key_val != val_old){
		
		if(key_val != valKB_NULL){
		
			 osDelay(50);
			 val_old = key_val;
			 return key_val;
		}else{
		
			 val_old = valKB_NULL;
		}
	}
	
	return valKB_NULL;
}

void kBoard_Init(void){

	;
}

void kBoard_Thread(const void *argument){

	char disp[30];
	u8 key_val;
	
	for(;;){
	
		key_val = test();
		
		sprintf(disp,"%d\r\n",key_val);
		Driver_USART1.Send(disp,strlen(disp));
		
		osDelay(100);
	}
}

void kBoardThread_Active(void){

	kBoard_Init();
	tid_kBoard_Thread = osThreadCreate(osThread(kBoard_Thread),NULL);
}
