#include "infraTrans.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static IFR_MEAS ifrCM_Attr;
static uint8_t  ifrDevMOUDLE_ID;

osThreadId tid_keyIFR_Thread;
osThreadId tid_keyIFR_Thread_umdScan;
osThreadDef(keyIFR_Thread,osPriorityAboveNormal,1,3072);
osThreadDef(keyIFR_Thread_umdScan,osPriorityAboveNormal,1,512);

osPoolId  IFR_pool;								 
osPoolDef(IFR_pool, 10, IFR_MEAS);                  // 内存池定义
osMessageQId  MsgBox_IFR;
osMessageQDef(MsgBox_IFR, 2, &IFR_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTIFR;
osMessageQDef(MsgBox_MTIFR, 2, &IFR_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPIFR;
osMessageQDef(MsgBox_DPIFR, 2, &IFR_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

osPoolId  memID_IFRsigK_pool;								 
osPoolDef(IFRsigK_pool, 4, IFR_kMSG);              // 按键消息内存池定义
osMessageQId  MsgBox_IFRsigK;
osMessageQDef(MsgBox_IFRsigK, 2, &IFR_kMSG);        // 消息队列定义，用于底板按键信号读取

const uint8_t IFR_PER  = 2;		//电平采样分辨率 2us
const uint8_t KEY_NUM = 11;
bool measure_en = true;

uint8_t tabHp,tabLp;		
uint16_t HTtab[Tab_size];
uint16_t LTtab[Tab_size];	//按键ifr载波信息缓存

uint16_t ifrKB_HTtab[KEY_NUM][Tab_size];	
uint16_t ifrKB_LTtab[KEY_NUM][Tab_size];
uint8_t  ifrKB_tabHp[KEY_NUM],ifrKB_tabLp[KEY_NUM];	//按键ifr载波信息缓存组

typedef void (* funkeyThread)(funKeyInit key_Init,Obj_keyStatus *orgKeyStatus,funKeyScan key_Scan,Obj_eventKey keyEvent,const char *Tips_head);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

void keyIFR_ADCInit(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟

    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		//模拟输入引脚
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器

    ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1

    ADC_ResetCalibration(ADC1);	//使能复位校准

    while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束

    ADC_StartCalibration(ADC1);	 //开启AD校准

    while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束

//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}

void Remote_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO,ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC,GPIO_Pin_6);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	PBout(15) = 0;
	PBout(13) = 1;

	EXTI_ClearITPendingBit(EXTI_Line6);
	
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(2);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);
}

u16 HW_ReceiveTime(void)
{
    u16 t=0;
	const uint16_t MAX = 12000 / IFR_PER;

    while(PCin(6))
    {
        t++;
        delay_us(IFR_PER);
		
        if(t >= MAX) return t;
    }

    return t;
}

u16 LW_ReceiveTime(void)
{
    u16 t=0;
	const uint16_t MAX = 12000 / IFR_PER;

    while(!PCin(6))
    {
        t++;
        delay_us(IFR_PER);
		
        if(t >= MAX) return t;
    }

    return t;
}


void IFR_Send(uint16_t HTab[],uint8_t Hp,uint16_t LTab[],uint8_t Lp){

	const uint16_t MAX = 6000 / IFR_PER; //长时间保持信号补偿标准
	uint8_t loop;
	uint16_t temp;
	
	char disp[20];							//对应数据位信号长度输出测试
	sprintf(disp,"%d",HTab[30]);
	Driver_USART1.Send(disp,strlen(disp));
	
	PBout(15) = 0;
	for(loop = 0;loop < Lp;loop ++){
	
		if(LTab[loop] < MAX){
		
			temp = (LTab[loop] + LTab[loop] / 14) / 12;	//载波重现 14为时间补偿，12(调试值) = 13-1(13 为 26 / IFR_PER) ， 26为38k载波单周期
		}	
		else{
		
			temp = (LTab[loop] + LTab[loop] / 14) / 12 + 60UL; //长时间保持信号补偿
		}
		while(temp --){			//38k载波单周期调制
		
			PBout(15) = 1;		
			delay_us(11);
			PBout(15) = 0;
			delay_us(12);			
		}
		
		PBout(15) = 0;		    //低电平信号互补
		temp = HTab[loop] + HTab[loop] / 15;
		while(temp --)delay_us(IFR_PER);
	}
	
	PBout(15) = 0;
}

//获得ADC值
//ch:通道值 0~3
uint16_t keyIFRGet_Adc(uint8_t ch)
{
    //设置指定ADC的规则组通道，一个序列，采样时间
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);	//ADC1,ADC通道,采样时间为239.5周期

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

    return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

uint16_t keyIFRGet_Adc_Average(uint8_t ch,uint8_t times)
{
    u32 temp_val = 0;
    uint8_t t;

    for(t=0; t<times; t++)
    {
        temp_val += keyIFRGet_Adc(ch);
        delay_ms(5);
    }
    return temp_val / times;
}

uint8_t keyIFR_Scan(void) {

    uint16_t keyAnalog =  keyIFRGet_Adc_Average(4,3);
	
	if(keyAnalog < 200)return ifrvalK_NULL;
	else return (uint8_t)(10 - (keyAnalog / 400));
}

void keyAnalog_Test(void) {

    char disp[30];
    uint16_t keyAnalog;

    keyIFR_ADCInit();

    for(;;) {

        keyAnalog = keyIFRGet_Adc(4);

        sprintf(disp,"valAnalog : %d\n\r", keyAnalog);
        Driver_USART1.Send(disp,strlen(disp));
        osDelay(500);
    }
}

void KB_ifrDats_Save(u8 num){

//	memset(ifrKB_HTtab[num], 0, Tab_size * sizeof(uint16_t));
//	memset(ifrKB_LTtab[num], 0, Tab_size * sizeof(uint16_t));
//	
//	memcpy(ifrKB_HTtab[num],(uint16_t *)HTtab,tabHp * 2);	//数据类型为 u16,而memcpy以字节为单位，倍乘2
//	memcpy(ifrKB_LTtab[num],(uint16_t *)LTtab,tabLp * 2);
//	
//	ifrKB_tabHp[num] = tabHp;
//	ifrKB_tabLp[num] = tabLp;
	
	STMFLASH_Write(MODULE_IFRdatsHpn_DATADDR + num,(uint16_t *)&tabHp,1);
	STMFLASH_Write(MODULE_IFRdatsLpn_DATADDR + num,(uint16_t *)&tabLp,1);
	STMFLASH_Write(MODULE_IFRdatsHp_DATADDR + (num * 256),(uint16_t *)HTtab,tabHp);
	STMFLASH_Write(MODULE_IFRdatsLp_DATADDR + (num * 256),(uint16_t *)LTtab,tabLp);
}

void usr_sigin(void){

	uint16_t Ktemp;
	uint8_t	 cnt;
	
	beeps(2);
	
	cnt = 5;
	while(cnt --){	//tips 开始学习
	
		IFRLRN_STATUS = !IFRLRN_STATUS;
		osDelay(150);
	}IFRLRN_STATUS = 0;
	
	ifrCM_Attr.VAL_KEY = ifrvalK_NULL;
	ifrCM_Attr.STATUS = kifrSTATUS_WAITK;
	
	for(;;){	//tips 等待存储目标按键
		
		Ktemp = ifrCM_Attr.VAL_KEY;
		
		if(ifrCM_Attr.VAL_KEY == ifrvalK_NULL){
		
			IFRLRN_STATUS = 0;
			osDelay(20);
		}else break;
	}measure_en = true;
	
	ifrCM_Attr.VAL_KEY = ifrvalK_NULL;
	
	beeps(1);
	
	ifrCM_Attr.STATUS = kifrSTATUS_WAITSG;
	
	while(measure_en){	//等待遥控信号
	
		IFRLRN_STATUS = !IFRLRN_STATUS;
		osDelay(500);
	}IFRLRN_STATUS = 1;
	
	KB_ifrDats_Save(Ktemp);	//目标按键存储ifr载波信息
	
	cnt = 5;
	while(cnt --){	//tips	存储学习完毕
	
		IFRLRN_STATUS = !IFRLRN_STATUS;
		osDelay(150);
	}IFRLRN_STATUS = 1;
	
	beeps(0);
	
	ifrCM_Attr.STATUS = kifrSTATUS_LRNOVR;
	osDelay(500);
}

void keyIFR_Thread_umdScan(const void *argument){

	osEvent  evt;
    osStatus status;	
	
	IFR_MEAS *mptr = NULL;
	IFR_MEAS *rptr = NULL;
	
	uint8_t ifrKey_temp;
	
	static IFR_MEAS ifrCM_Attr_tempDP;
	
	for(;;){
	
		evt = osMessageGet(MsgBox_MTIFR, 100);
		if (evt.status == osEventMessage){		//等待消息指令
			
			rptr = evt.value.p;
			/*自定义本地线程接收数据处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			
			if(rptr->Mod_addr == ifrDevMOUDLE_ID)
				ifrCM_Attr.VAL_KEY = rptr->VAL_KEY;

			do{status = osPoolFree(IFR_pool, rptr);}while(status != osOK);	//内存释放
			rptr = NULL;
		}
		
		ifrKey_temp = keyIFR_Scan();
		if(ifrvalK_NULL != ifrKey_temp){ifrCM_Attr.VAL_KEY = ifrKey_temp; osDelay(500);}
		
		if(ifrCM_Attr_tempDP.VAL_KEY != ifrCM_Attr.VAL_KEY||
		   ifrCM_Attr_tempDP.STATUS  != ifrCM_Attr.STATUS){
			   
			    ifrCM_Attr_tempDP.VAL_KEY = ifrCM_Attr.VAL_KEY;
			    ifrCM_Attr_tempDP.STATUS  = ifrCM_Attr.STATUS;
		   
				do{mptr = (IFR_MEAS *)osPoolCAlloc(IFR_pool);}while(mptr == NULL);	//1.44寸液晶显示消息推送
				mptr->speDPCMD = SPECMD_ifrDevDATS_CHG;
				mptr->VAL_KEY  = ifrCM_Attr.VAL_KEY;
				mptr->STATUS   = ifrCM_Attr.STATUS;
				osMessagePut(MsgBox_DPIFR, (uint32_t)mptr, 100);
				osDelay(10);
		}
		   
		osDelay(10);
	}
}

void keyIFR_Thread(const void *argument){
	
	osEvent  evt;
    osStatus status;
	
//	char disp[30];
	pwmCM_kMSG *rptr_sigK = NULL;
	
	ifrCM_Attr.VAL_KEY = ifrvalK_NULL;
	
	STMFLASH_Read(MODULE_IFRid_DATADDR,(uint16_t *)&ifrDevMOUDLE_ID,1);
	if((ifrDevMOUDLE_ID != ifrDevMID_video)&&\
	   (ifrDevMOUDLE_ID != ifrDevMID_audio))ifrDevMOUDLE_ID = ifrDevMID_audio;
	
	for(;;){
		
	/********************************底板按键线程数据接收******************************************/
		evt = osMessageGet(MsgBox_IFRsigK, 100);
		if (evt.status == osEventMessage){
			
			IFR_MEAS *mptr = NULL;
		
			rptr_sigK = evt.value.p;
			/*底板按键消息处理↓↓↓↓↓↓↓↓↓↓↓↓*/
			
			ifrDevMOUDLE_ID = rptr_sigK->mADDR;
			STMFLASH_Write(MODULE_IFRid_DATADDR,(uint16_t *)&ifrDevMOUDLE_ID,1);
			
			do{mptr = (IFR_MEAS *)osPoolCAlloc(IFR_pool);}while(mptr == NULL);	//底板按键事件送显
			mptr->speDPCMD = SPECMD_ifrDevModADDR_CHG;
			osMessagePut(MsgBox_DPIFR, (uint32_t)mptr, 100);
			beeps(9);
			
			do{status = osPoolFree(memID_IFRsigK_pool, rptr_sigK);}while(status != osOK);	//内存释放
			rptr_sigK = NULL;
		}
		
	/************************************常线程***************************************************/
		ifrCM_Attr.STATUS = kifrSTATUS_NONLRN;
		
		if(ifrCM_Attr.VAL_KEY != ifrvalK_NULL){
		
			if(ifrCM_Attr.VAL_KEY == 1){usr_sigin(); ifrCM_Attr.VAL_KEY = ifrvalK_NULL;}
			else{
					
				ifrCM_Attr.STATUS = kifrSTATUS_SGOUT;
			
				memset(HTtab, 0, Tab_size * sizeof(uint16_t));
				memset(LTtab, 0, Tab_size * sizeof(uint16_t));
				
				STMFLASH_Read(MODULE_IFRdatsHpn_DATADDR + ifrCM_Attr.VAL_KEY,(uint16_t *)&tabHp,1);
				STMFLASH_Read(MODULE_IFRdatsLpn_DATADDR + ifrCM_Attr.VAL_KEY,(uint16_t *)&tabLp,1);
				
				STMFLASH_Read(MODULE_IFRdatsHp_DATADDR + (ifrCM_Attr.VAL_KEY * 256),(uint16_t *)HTtab,tabHp);
				STMFLASH_Read(MODULE_IFRdatsLp_DATADDR + (ifrCM_Attr.VAL_KEY * 256),(uint16_t *)LTtab,tabLp);
				
				if((HTtab[0] + HTtab[1] + HTtab[2] + LTtab[0] + LTtab[1] + LTtab[2]) > 65000){tips_beep(6,100,1); ifrCM_Attr.VAL_KEY = ifrvalK_NULL; continue;} //取样，读到非法溢出值（未经存储过的值），无法使用

				IFR_Send(HTtab,tabHp,LTtab,tabLp);
				osDelay(20);
				IFR_Send(HTtab,tabHp,LTtab,tabLp);
				
//				IFR_Send((uint16_t *)ifrKB_HTtab[ifrCM_Attr.VAL_KEY],ifrKB_tabHp[ifrCM_Attr.VAL_KEY],(uint16_t *)ifrKB_LTtab[ifrCM_Attr.VAL_KEY],ifrKB_tabLp[ifrCM_Attr.VAL_KEY]);
//				osDelay(20);
//				IFR_Send((uint16_t *)ifrKB_HTtab[ifrCM_Attr.VAL_KEY],ifrKB_tabHp[ifrCM_Attr.VAL_KEY],(uint16_t *)ifrKB_LTtab[ifrCM_Attr.VAL_KEY],ifrKB_tabLp[ifrCM_Attr.VAL_KEY]);				
				
				tips_beep(1,100,5);
				osDelay(100);
				ifrCM_Attr.VAL_KEY = ifrvalK_NULL;
			}
		}
	
//		sprintf(disp,"%d\n",ifrCM_Attr.VAL_KEY);	/*调试语句_检测键值*/
//		Driver_USART1.Send(disp,strlen(disp));
//		if(ifrCM_Attr.VAL_KEY != ifrvalK_NULL)ifrCM_Attr.VAL_KEY = ifrvalK_NULL;
		
		osDelay(50);
	}
	
//	for(;;){					/*调试语句_检测键值*/
//	
//		keyAnalog_Test();
//	}
}

void keyIFR_Terminate(void){

	osThreadTerminate(tid_keyIFR_Thread); 
	osThreadTerminate(tid_keyIFR_Thread_umdScan); 
}

void keyIFRActive(void){

	static bool memInit_flg = false;
	
	Remote_Init();
	keyIFR_ADCInit();
	
	tid_keyIFR_Thread = osThreadCreate(osThread(keyIFR_Thread),NULL);
	tid_keyIFR_Thread_umdScan = osThreadCreate(osThread(keyIFR_Thread_umdScan),NULL);
	
	if(!memInit_flg){

		IFR_pool     = osPoolCreate(osPool(IFR_pool));	//创建内存池
		MsgBox_IFR 	 = osMessageCreate(osMessageQ(MsgBox_IFR), NULL);	//创建消息队列
		MsgBox_MTIFR = osMessageCreate(osMessageQ(MsgBox_MTIFR), NULL);	//创建消息队列
		MsgBox_DPIFR = osMessageCreate(osMessageQ(MsgBox_DPIFR), NULL);	//创建消息队列
		
		memID_IFRsigK_pool = osPoolCreate(osPool(IFRsigK_pool));
		MsgBox_IFRsigK = osMessageCreate(osMessageQ(MsgBox_IFRsigK), NULL);
		
		memInit_flg = true;
	}
}

