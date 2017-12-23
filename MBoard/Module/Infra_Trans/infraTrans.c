#include "infraTrans.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

osThreadId tid_keyIFR_Thread;
osThreadDef(keyIFR_Thread,osPriorityAboveNormal,1,4096);

osPoolId  IFR_pool;								 
osPoolDef(IFR_pool, 10, IFR_MEAS);                  // 内存池定义
osMessageQId  MsgBox_IFR;
osMessageQDef(MsgBox_IFR, 2, &IFR_MEAS);            // 消息队列定义，用于模块线程向无线通讯线程
osMessageQId  MsgBox_MTIFR;
osMessageQDef(MsgBox_MTIFR, 2, &IFR_MEAS);          // 消息队列定义,用于无线通讯线程向模块线程
osMessageQId  MsgBox_DPIFR;
osMessageQDef(MsgBox_DPIFR, 2, &IFR_MEAS);          // 消息队列定义，用于模块线程向显示模块线程

const uint8_t Tab_size = 255;	//信号采样表
const uint8_t IFR_PER  = 2;		//电平采样分辨率 2us
bool measure_en = true;
uint8_t tabHp,tabLp;
volatile uint16_t HTtab[Tab_size];
volatile uint16_t LTtab[Tab_size];

typedef void (* funkeyThread)(funKeyInit key_Init,Obj_keyStatus *orgKeyStatus,funKeyScan key_Scan,Obj_eventKey keyEvent,const char *Tips_head);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

void keyIFR_ADCInit(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟

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

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO,ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_14);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	PBout(15) = 0;

	EXTI_ClearITPendingBit(EXTI_Line14);
	
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(2);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);
}

u16 HW_ReceiveTime(void)
{
    u16 t=0;
	const uint16_t MAX = 12000 / IFR_PER;

    while(PBin(14))
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

    while(!PBin(14))
    {
        t++;
        delay_us(IFR_PER);
		
        if(t >= MAX) return t;
    }

    return t;
}

void EXTI15_10_IRQHandler(void) {

	const uint16_t SIGLEN_MAX = 10000 / IFR_PER;  //一个电平信号维持最长时间限制
	const uint16_t SIGLEN_MIN = 4000  / IFR_PER;  //一个电平信号维持最短时间限制
	
	static uint8_t Tab_Hp,Tab_Lp;
	static uint16_t HT_Tab[Tab_size] = {0};
	static uint16_t LT_Tab[Tab_size] = {0};
	uint16_t time = 0;
	
	Tab_Hp = Tab_Lp = 0;

    if(measure_en && EXTI_GetITStatus(EXTI_Line14) == SET)
    {
		
		Tab_Hp = Tab_Lp = 0;
		memset(HT_Tab, 0, Tab_size * sizeof(uint16_t));
		memset(LT_Tab, 0, Tab_size * sizeof(uint16_t));
		while(1){
		
			if(!PBin(14)){
			
				time = LW_ReceiveTime();
				
				if(time > SIGLEN_MIN && Tab_Lp > 155){  //缓存不够，周期截波跳出
				
					//Driver_USART1.Send(&Tab_Lp,1);   //单周期高电平信号总长输出测试
					memcpy((uint16_t *)LTtab,LT_Tab,Tab_Lp * 2);	//数据类型为 u16,而memcpy以字节为单位，倍乘2
					memcpy((uint16_t *)HTtab,HT_Tab,Tab_Hp * 2);
					tabLp = Tab_Lp;
					tabHp = Tab_Hp;
					measure_en = false;
					break;
				}else{
					
					LT_Tab[Tab_Lp ++] = time;
				}
			}
			if(PBin(14)){
			
				time = HW_ReceiveTime();
					
				if((time > SIGLEN_MAX) || (time > SIGLEN_MIN && Tab_Hp > 155)){
					
					//Driver_USART1.Send(&Tab_Hp,1);	//单周期低电平信号总长输出测试
					memcpy((uint16_t *)LTtab,LT_Tab,Tab_Lp * 2);	//数据类型为 u16,而memcpy以字节为单位，倍乘2
					memcpy((uint16_t *)HTtab,HT_Tab,Tab_Hp * 2);
					tabLp = Tab_Lp;
					tabHp = Tab_Hp;
					measure_en = false;
					break;
				}else{

					HT_Tab[Tab_Hp ++] = time;
				}
			}
		}
    }
	
	EXTI_ClearITPendingBit(EXTI_Line14);
	EXTI_ClearFlag(EXTI_Line14);
}

void IFR_Send(uint16_t HTab[],uint8_t Hp,uint16_t LTab[],uint8_t Lp){

	const uint16_t MAX = 6000 / IFR_PER; //长时间保持信号补偿标准
	uint8_t loop;
	uint16_t temp;
	
//	char disp[20];							//对应数据位信号长度输出测试
//	sprintf(disp,"%d",LTab[69]);
//	Driver_USART1.Send(disp,strlen(disp));
	
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

/***按键检测状态机***/
static uint16_t getKey2(Obj_keyStatus *orgKeyStatus,funKeyScan keyScan) {

    static	uint16_t s_u16KeyState 		= KEY_STATE_INIT;		//状态机检测状态，初始化状态
    static	uint16_t	s_u16LastKey		= KEY_NULL;			//保留历史按键键值
    static	uint8_t	KeyKeepComfirm		= 0;					//长按后确认保持 确认所用时长计时
    static	uint16_t	s_u16KeyTimeCount	= 0;						//长按时长定义（用来对KEY_TICK进行计数，根据这个计数值来确认是否属于长按）
    uint16_t keyTemp				= KEY_NULL;					/*十六进制第一位：按键状态；	第二位：保持计数值；		第三位：键值；		第四位：连按计数值*/

    static	uint32_t osTick_last			= 0xffff0000;		//对osTick进行记录，用于与下一次osTick进行对比获取间隔（此间隔用于判断按键是否属于连续按下）

    keyTemp = keyScan();		//获取键值

    switch(s_u16KeyState) {		//获取状态机状态

    case KEY_STATE_INIT:		//初始化状态

        if(orgKeyStatus->keyCTflg) {	//检测上一次是否为连按

            if((osKernelSysTick() - osTick_last) > KEY_CONTINUE_PERIOD) {	//上一次是连按则检测本次是否继续连按

                keyTemp	= s_u16LastKey & 0x00f0;	//本次不是连按，处理返回值为连按结束状态，同时连按标志位清零
                keyTemp |= KEY_CTOVER;
                orgKeyStatus->keyCTflg = 0;
            }
        }

        if(KEY_NULL != keyTemp)s_u16KeyState = KEY_STATE_WOBBLE;	//检测到有按键，切换状态到抖动检测
        break;

    case KEY_STATE_WOBBLE:	//消抖状态检测

        s_u16KeyState = KEY_STATE_PRESS;	//确认按键，切换状态到短按检测
        break;

    case KEY_STATE_PRESS:	//短按状态检测

        if(KEY_NULL != keyTemp) {		//按键是否弹起？

            s_u16LastKey 	= keyTemp;	//存储按键键值
            keyTemp 		  |= KEY_DOWN;	//按键状态确认为按下
            s_u16KeyState	= KEY_STATE_LONG;	//按键依然未弹起，切换状态到长按检测
        } else {

            s_u16KeyState	= KEY_STATE_INIT;	//检测为按键抖动，检测状态机返回初始化状态
        }
        break;

    case KEY_STATE_LONG:		//长按状态检测

        if(KEY_NULL != keyTemp) {	//按键是否弹起？

            if(++s_u16KeyTimeCount > KEY_LONG_PERIOD) {	//若按键依然未弹起，则根据长按时长进行计数继续确认是否为长按

                s_u16KeyTimeCount	= 0;			//确认长按，长按计数值清零
                orgKeyStatus->sKeyKeep				= 0;			//检测状态机切换状态前，对保持确认状态所需计数值提前清零，本计数值为长按后保持计数步长
                KeyKeepComfirm		= 0;			//检测状态机切换状态前，对保持确认状态所需计数值提前清零，本数值用于定义长按后多久进行保持计数
                keyTemp			  |= KEY_LONG;	//按键状态确认为长按
                s_u16KeyState		= KEY_STATE_KEEP;	//按键依然未弹起，切换状态到长按保持检测

                orgKeyStatus->keyOverFlg			= KEY_OVER_LONG;	//长按
            } else	orgKeyStatus->keyOverFlg	= KEY_OVER_SHORT;		//短按
        } else {

            s_u16KeyState	= KEY_STATE_RELEASE;	//检测确认为长按后按键弹起，切换状态到弹起检测
        }
        break;

    case KEY_STATE_KEEP:		//长按后保持状态检测

        if(KEY_NULL != keyTemp) {		//按键是否弹起？

            if(++s_u16KeyTimeCount > KEY_KEEP_PERIOD) {	//若按键依然未弹起，则根据长按时长进行计数继续确认是否为长按后继续保持

                s_u16KeyTimeCount	= 0;			//确认长按后继续保持，长按计数值清零
                if(KeyKeepComfirm < (KEY_COMFIRM + 3))KeyKeepComfirm++;			//检测是否到允许计数时刻
                if(orgKeyStatus->sKeyKeep < 15 && KeyKeepComfirm > KEY_COMFIRM)orgKeyStatus->sKeyKeep++; 	//检测到允许计数，执行长按后保持计数
                if(orgKeyStatus->sKeyKeep) {		//检测到长按后保持计数值不为零，即确认按键状态为长按后继续保持，对返回值进行相应确认处理

                    orgKeyStatus->keyOverFlg	 = KEY_OVER_KEEP;	//状态确认为长按后为保持
                    keyTemp	|= orgKeyStatus->sKeyKeep << 8;		//保持计数数据左移8位放在十六进制keyTemp第二位
                    keyTemp	|= KEY_KEEP;			//按键状态确认为长按后继续保持
                }
            }
        } else {

            s_u16KeyState	= KEY_STATE_RELEASE;	//按键状态确认为长按后继续保持之后弹起，切换状态到弹起检测
        }
        break;

    case KEY_STATE_RELEASE:	//弹起状态检测

        s_u16LastKey |= KEY_UP;	//存储按键状态
        keyTemp		  = s_u16LastKey;	//按键状态确认为弹起
        s_u16KeyState = KEY_STATE_RECORD;	//切换状态到按键连按记录
        break;

    case KEY_STATE_RECORD:	//按键连按记录状态检测

        if((osKernelSysTick() - osTick_last) < KEY_CONTINUE_PERIOD) {	//若两次按键弹起时间间隔小于规定值，则判断为连按

            orgKeyStatus->sKeyCount++;	//连按计数
        } else {

            orgKeyStatus->sKeyCount = 0;	//连按断开，计数清零
        }

        if(orgKeyStatus->sKeyCount) {		//若连按次数不为零，即确认为按键连按，对返回值进行相应处理

            orgKeyStatus->keyCTflg	= 1;	//打开连按标志
            keyTemp	= s_u16LastKey & 0x00f0;	//提取键值
            keyTemp	|=	KEY_CONTINUE;				//确认为按键连按
            if(orgKeyStatus->sKeyCount < 15)keyTemp += orgKeyStatus->sKeyCount;	//连按计数数据放在十六进制keyTemp第四位（最低位）
        }

        s_u16KeyState	= KEY_STATE_INIT;		//检测状态机返回初始状态
        osTick_last	 	= osKernelSysTick();	//记录osTick，留作下次连按检测对比
        break;

    default:
        break;
    }
    return keyTemp;	//返回按键状态和键值
}

/*按键初始化函数，按键状态缓存结构体，按键扫描函数，按键触发事件函数表，按键提示信息头*/
void key_Thread2(funKeyInit key_Init,
                 Obj_keyStatus *orgKeyStatus,
                 funKeyScan key_Scan,Obj_eventKey keyEvent,
                 const char *Tips_head) {

    /***按键调试（串口1反馈调试信息）****/
    static uint16_t keyVal;						//按键状态事件
    static uint8_t	key_temp;					//按键键值缓冲
    static uint8_t	kCount;						//按键计数值变量，长按保持计数和连按计数使用同一个变量，因为两个状态不会同时发生
    static uint8_t	kCount_rec;			//历史计数值保存

    static osThreadId ID_Temp;					//当前线程ID缓存
    static osEvent evt;
    static uint8_t KEY_DEBUG_FLG = 0;

    const	 uint8_t	tipsLen = 100;		//Tips打印字符串长度
    static char	key_tempDisp;
    static char	kCountDisp;
    static char	kCount_recDisp;
    static char	tips[tipsLen];					//Tips字符串

    key_Init();

    for(;;) {

        keyVal = getKey2(orgKeyStatus,key_Scan);    //获取键值

        ID_Temp = osThreadGetId();
        evt = osSignalWait (KEY_DEBUG_OFF, 1);		 //获取Debug_log输出权限信号
        if (evt.value.signals == KEY_DEBUG_OFF) {

            KEY_DEBUG_FLG = 0;
            osSignalClear(ID_Temp,KEY_DEBUG_OFF);
        } else {

            evt = osSignalWait (KEY_DEBUG_ON, 1);
            if (evt.value.signals == KEY_DEBUG_ON) {

                KEY_DEBUG_FLG = 1;
                osSignalClear(ID_Temp,KEY_DEBUG_ON);
            }
        }

        if(KEY_DEBUG_FLG) {

            memset(tips,0,tipsLen*sizeof(char));	//每轮Tips打印后清空
            strcat(tips,"Tips:");						//Tips标识
            strcat(tips,Tips_head);
            strcat(tips,"-");
        }
        /*------------------------------------------------------------------------------------------------------------------------------*/
        switch(keyVal & 0xf000) {

        case KEY_LONG		:

            key_temp = (uint8_t)((keyVal & 0x00f0) >> 4);
            if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                strcat(tips,"按键");
                key_tempDisp = key_temp + '0';
                strcat(tips,(const char*)&key_tempDisp);
                strcat(tips,"长按\r\n");
                Driver_USART1.Send(tips,strlen(tips));
            }/***/
            break;

        case KEY_KEEP		:

            kCount		= (uint8_t)((keyVal & 0x0f00) >> 8);  //获取计数值
            kCount_rec	= kCount;
            if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                strcat(tips,"按键");
                key_tempDisp = key_temp + '0';
                strcat(tips,(const char*)&key_tempDisp);
                strcat(tips,"长按保持，保持计数：");
                kCountDisp = kCount + '0';
                strcat(tips,(const char*)&kCountDisp);
                strcat(tips,"\r\n");
                Driver_USART1.Send(tips,strlen(tips));
            }/***/
            break;

        case KEY_DOWN		:

            key_temp = (uint8_t)((keyVal & 0x00f0) >> 4);
            if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                strcat(tips,"按键");
                key_tempDisp = key_temp + '0';
                strcat(tips,(const char*)&key_tempDisp);
                strcat(tips,"按下\r\n");
                Driver_USART1.Send(tips,strlen(tips));
            }/***/
            break;

        case KEY_UP			:
            if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                strcat(tips,"按键");
                key_tempDisp = key_temp + '0';
                strcat(tips,(const char*)&key_tempDisp);
            }/***/
            switch(orgKeyStatus->keyOverFlg) {

            case KEY_OVER_SHORT		:

                if(keyEvent.funKeySHORT[key_temp])keyEvent.funKeySHORT[key_temp]();		//按键事件触发，先检测触发事件是否创建，没创建则不进行触发
                if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                    strcat(tips,"短按后弹起\r\n");
                    Driver_USART1.Send(tips,strlen(tips));
                    orgKeyStatus->keyOverFlg = 0;
                }/***/
                break;

            case KEY_OVER_LONG		:

                if(keyEvent.funKeyLONG[key_temp])keyEvent.funKeyLONG[key_temp]();
                if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                    strcat(tips,"长按后弹起\r\n");
                    Driver_USART1.Send(tips,strlen(tips));
                    orgKeyStatus->keyOverFlg = 0;
                }/***/
                break;

            case KEY_OVER_KEEP		:

                if(keyEvent.funKeyKEEP[key_temp][kCount_rec])keyEvent.funKeyKEEP[key_temp][kCount_rec]();
                if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                    strcat(tips,"长按后保持");
                    kCount_recDisp = kCount_rec + '0';
                    strcat(tips,(const char*)&kCount_recDisp);
                    strcat(tips,"次计数后结束\r\n");
                    Driver_USART1.Send(tips,strlen(tips));
                    kCount_rec = 0;
                }/***/
                break;
            default:
                break;
            }
            break;

        case KEY_CONTINUE	:

            kCount 		= (uint8_t)((keyVal & 0x000f) >> 0);	//获取计数值
            kCount_rec	= kCount + 1;
            if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                strcat(tips,"按键");
                key_tempDisp = key_temp + '0';
                strcat(tips,(const char*)&key_tempDisp);
                strcat(tips,"连按，连按计数：");
                kCountDisp = kCount + '0';
                strcat(tips,(const char*)&kCountDisp);
                strcat(tips,"\r\n");
                Driver_USART1.Send(tips,strlen(tips));
            }/***/
            break;

        case KEY_CTOVER	:

            if(keyEvent.funKeyCONTINUE[key_temp][kCount_rec])keyEvent.funKeyCONTINUE[key_temp][kCount_rec]();
            if(KEY_DEBUG_FLG) { /*Debug_log输出使能*/
                strcat(tips,"按键");
                key_tempDisp = key_temp + '0';
                strcat(tips,(const char*)&key_tempDisp);
                strcat(tips,"连按");
                kCount_recDisp = kCount_rec + '0';
                strcat(tips,(const char*)&kCount_recDisp);
                strcat(tips,"次后结束\r\n");
                Driver_USART1.Send(tips,strlen(tips));
            }/***/
            kCount_rec = 0;
            break;

        default:
            break;
        }
        osDelay(20);
    }
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

uint16_t keyIFR_Scan(void) {

    uint16_t keyAnalog = keyIFRGet_Adc(4);
	
	if(keyAnalog < 150)return KEY_NULL;
	else return (10 - (keyAnalog / 400)) << 4;
}

void test_s10(void) {

    Driver_USART1.Send("abcd",4);
    osDelay(20);
}

void usr_sigin(void){

	measure_en = true;
}

void usr_sigout(void){

	IFR_Send((uint16_t *)HTtab,tabHp,(uint16_t *)LTtab,tabLp);
}

void keyIFR_Thread(const void *argument) {

    const char *Tips_Head = "红外转发扩展板按键";
    static Obj_eventKey myKeyIFREvent = {0};	//按键触发事件表，先建立空表，需要哪种触发事件，直接创建对应函数即可，空白处自动判断不会触发
    static Obj_keyStatus myKeyStatus = {0};		//按键判断所需标志初始化
    static funkeyThread key_ThreadIFR = key_Thread2;

    myKeyIFREvent.funKeySHORT[10] = test_s10;			//k10短按触发
	
	myKeyIFREvent.funKeySHORT[7]  = usr_sigin;			//k7短按触发
	myKeyIFREvent.funKeySHORT[9]  = usr_sigout;			//k9短按触发

    key_ThreadIFR(keyIFR_ADCInit,&myKeyStatus,keyIFR_Scan,myKeyIFREvent,Tips_Head);
}

void keyIFRActive(void) {

	Remote_Init();
	keyIFR_ADCInit();
    tid_keyIFR_Thread = osThreadCreate(osThread(keyIFR_Thread),NULL);
}

