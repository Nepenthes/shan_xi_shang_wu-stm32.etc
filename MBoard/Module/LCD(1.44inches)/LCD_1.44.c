#include "LCD_1.44.h"
  
//管理LCD重要参数
//默认为竖屏
_lcd144_dev lcd144dev;

osThreadId tid_LCD144Test_Thread;

osThreadDef(LCD144Disp_Thread,osPriorityNormal,1,2048);

//画笔颜色,背景颜色
uint16_t LCD144POINT_COLOR = 0x0000,LCD144BACK_COLOR = 0xFFFF;  
uint16_t DeviceCode;	 
/****************************************************************************
* 名    称：void  SPIv_WriteData(uint8_t Data)
* 功    能：STM32_模拟SPI写一个字节数据底层函数
* 入口参数：Data
* 出口参数：无
* 说    明：STM32_模拟SPI读写一个字节数据底层函数
****************************************************************************/
void  SPIv_WriteData(uint8_t Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80)	
	  LCD_1_44_SDA_SET; //输出数据
      else LCD_1_44_SDA_CLR;
	   
      LCD_1_44_SCL_CLR;       
      LCD_1_44_SCL_SET;
      Data<<=1; 
	}
}

/****************************************************************************
* 名    称：uint8_t SPI_WriteByte(SPI_TypeDef* SPIx,uint8_t Byte)
* 功    能：STM32_硬件SPI读写一个字节数据底层函数
* 入口参数：SPIx,Byte
* 出口参数：返回总线收到的数据
* 说    明：STM32_硬件SPI读写一个字节数据底层函数
****************************************************************************/
uint8_t SPI_WriteByte(SPI_TypeDef* SPIx,uint8_t Byte)
{
	while((SPIx->SR&SPI_I2S_FLAG_TXE)==RESET);		//等待发送区空	  
	SPIx->DR=Byte;	 	//发送一个byte   
	while((SPIx->SR&SPI_I2S_FLAG_RXNE)==RESET);//等待接收完一个byte  
	return SPIx->DR;          	     //返回收到的数据			
} 

/****************************************************************************
* 名    称：void SPI_SetSpeed(SPI_TypeDef* SPIx,uint8_t SpeedSet)
* 功    能：设置SPI的速度
* 入口参数：SPIx,SpeedSet
* 出口参数：无
* 说    明：SpeedSet:1,高速;0,低速;
****************************************************************************/
void SPI_SetSpeed(SPI_TypeDef* SPIx,uint8_t SpeedSet)
{
	SPIx->CR1&=0XFFC7;
	if(SpeedSet==1)//高速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_2;//Fsck=Fpclk/2	
	}
	else//低速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_32; //Fsck=Fpclk/32
	}
	SPIx->CR1|=1<<6; //SPI设备使能
} 

/****************************************************************************
* 名    称：SPI3_Init(void)
* 功    能：STM32_SPI3硬件配置初始化
* 入口参数：无
* 出口参数：无
* 说    明：STM32_SPI3硬件配置初始化
****************************************************************************/
void SPI3_Init(void)	
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	 
	//配置SPI3管脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//SPI3配置选项
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);
	   
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI3, &SPI_InitStructure);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE) ;

	//使能SPI3
	SPI_Cmd(SPI3, ENABLE);  
}

//******************************************************************
//函数名：  LCD_1_44_WR_REG
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    向液晶屏总线写入写16位指令
//输入参数：Reg:待写入的指令值
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WR_REG(uint16_t data)
{ 
   LCD_1_44_CS_CLR;
   LCD_1_44_RS_CLR;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI3,data);
#else
   SPIv_WriteData(data);
#endif 
   LCD_1_44_CS_SET;
}

//******************************************************************
//函数名：  LCD_1_44_WR_DATA
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    向液晶屏总线写入写8位数据
//输入参数：Data:待写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WR_DATA(uint8_t data)
{
	
   LCD_1_44_CS_CLR;
   LCD_1_44_RS_SET;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI3,data);
#else
   SPIv_WriteData(data);
#endif 
   LCD_1_44_CS_SET;

}
//******************************************************************
//函数名：  LCD_1_44_DrawPoint_16Bit
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    8位总线下如何写入一个16位数据
//输入参数：(x,y):光标坐标
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WR_DATA_16Bit(uint16_t data)
{	
   LCD_1_44_CS_CLR;
   LCD_1_44_RS_SET;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI3,data>>8);
   SPI_WriteByte(SPI3,data);
#else
   SPIv_WriteData(data>>8);
   SPIv_WriteData(data);
#endif 
   LCD_1_44_CS_SET;
}

//******************************************************************
//函数名：  LCD_1_44_WriteReg
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    写寄存器数据
//输入参数：LCD_1_44_Reg:寄存器地址
//			LCD_1_44_RegValue:要写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WriteReg(uint16_t LCD_1_44_Reg, uint16_t LCD_1_44_RegValue)
{	
	LCD_1_44_WR_REG(LCD_1_44_Reg);  
	LCD_1_44_WR_DATA(LCD_1_44_RegValue);	    		 
}	   
	 
//******************************************************************
//函数名：  LCD_1_44_WriteRAM_Prepare
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    开始写GRAM
//			在给液晶屏传送RGB数据前，应该发送写GRAM指令
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WriteRAM_Prepare(void)
{
	LCD_1_44_WR_REG(lcd144dev.wramcmd);
}	 

//******************************************************************
//函数名：  LCD_1_44_DrawPoint
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    在指定位置写入一个像素点数据
//输入参数：(x,y):光标坐标
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_1_44_SetCursor(x,y);//设置光标位置 
	LCD_1_44_WR_DATA_16Bit(LCD144POINT_COLOR);
}

//******************************************************************
//函数名：  LCD_1_44_GPIOInit
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    液晶屏IO初始化，液晶初始化前要调用此函数
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_GPIOInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	      
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	  	
}

//******************************************************************
//函数名：  LCD_1_44_Reset
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD复位函数，液晶初始化前要调用此函数
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_RESET(void)
{
	LCD_1_44_RST_CLR;
	osDelay(100);	
	LCD_1_44_RST_SET;
	osDelay(50);
}
 	 
//******************************************************************
//函数名：  LCD_1_44_Init
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD初始化
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD144_Init(void)
{  
#if USE_HARDWARE_SPI //使用硬件SPI
	SPI3_Init();
#else	
	LCD_1_44_GPIOInit();//使用模拟SPI
#endif  										 

 	LCD_1_44_RESET(); //液晶屏复位

	//************* Start Initial Sequence **********//		
  LCD_1_44_WR_REG(0x11); //Exit Sleep
	osDelay(20);
	LCD_1_44_WR_REG(0x26); //Set Default Gamma
	LCD_1_44_WR_DATA(0x04);
	LCD_1_44_WR_REG(0xB1);//Set Frame Rate
	LCD_1_44_WR_DATA(0x0e);
	LCD_1_44_WR_DATA(0x10);
	LCD_1_44_WR_REG(0xC0); //Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
	LCD_1_44_WR_DATA(0x08);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_REG(0xC1); //Set BT[2:0] for AVDD & VCL & VGH & VGL
	LCD_1_44_WR_DATA(0x05);
	LCD_1_44_WR_REG(0xC5); //Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	LCD_1_44_WR_DATA(0x38);
	LCD_1_44_WR_DATA(0x40);
	
	LCD_1_44_WR_REG(0x3a); //Set Color Format
	LCD_1_44_WR_DATA(0x05);
	LCD_1_44_WR_REG(0x36); //RGB
	LCD_1_44_WR_DATA(0x1C);   //1C//C8
	
	LCD_1_44_WR_REG(0x2A); //Set Column Address
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(0x7F);
	LCD_1_44_WR_REG(0x2B); //Set Page Address
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(32);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(127+32);
	
	LCD_1_44_WR_REG(0xB4); 
	LCD_1_44_WR_DATA(0x00);
	
	LCD_1_44_WR_REG(0xf2); //Enable Gamma bit
	LCD_1_44_WR_DATA(0x01);
	LCD_1_44_WR_REG(0xE0); 
	LCD_1_44_WR_DATA(0x3f);//p1
	LCD_1_44_WR_DATA(0x22);//p2
	LCD_1_44_WR_DATA(0x20);//p3
	LCD_1_44_WR_DATA(0x30);//p4
	LCD_1_44_WR_DATA(0x29);//p5
	LCD_1_44_WR_DATA(0x0c);//p6
	LCD_1_44_WR_DATA(0x4e);//p7
	LCD_1_44_WR_DATA(0xb7);//p8
	LCD_1_44_WR_DATA(0x3c);//p9
	LCD_1_44_WR_DATA(0x19);//p10
	LCD_1_44_WR_DATA(0x22);//p11
	LCD_1_44_WR_DATA(0x1e);//p12
	LCD_1_44_WR_DATA(0x02);//p13
	LCD_1_44_WR_DATA(0x01);//p14
	LCD_1_44_WR_DATA(0x00);//p15
	LCD_1_44_WR_REG(0xE1); 
	LCD_1_44_WR_DATA(0x00);//p1
	LCD_1_44_WR_DATA(0x1b);//p2
	LCD_1_44_WR_DATA(0x1f);//p3
	LCD_1_44_WR_DATA(0x0f);//p4
	LCD_1_44_WR_DATA(0x16);//p5
	LCD_1_44_WR_DATA(0x13);//p6
	LCD_1_44_WR_DATA(0x31);//p7
	LCD_1_44_WR_DATA(0x84);//p8
	LCD_1_44_WR_DATA(0x43);//p9
	LCD_1_44_WR_DATA(0x06);//p10
	LCD_1_44_WR_DATA(0x1d);//p11
	LCD_1_44_WR_DATA(0x21);//p12
	LCD_1_44_WR_DATA(0x3d);//p13
	LCD_1_44_WR_DATA(0x3e);//p14
	LCD_1_44_WR_DATA(0x3f);//p15
	
	LCD_1_44_WR_REG(0x29); // Display On
	LCD_1_44_WR_REG(0x2C);

	LCD_1_44_SetParam();//设置LCD参数	 
	///LCD_1_44_LED_SET;//点亮背光	 
	//LCD_1_44_Clear(WHITE);
}
//******************************************************************
//函数名：  LCD_1_44_Clear
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD全屏填充清屏函数
//输入参数：Color:要清屏的填充色
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_Clear(uint16_t Color)
{
	uint16_t i,j;      
	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);	  
	for(i=0;i<lcd144dev.width;i++)
	{
		for(j=0;j<lcd144dev.height;j++)
		LCD_1_44_WR_DATA_16Bit(Color);	//写入数据 	 
	}
} 

void LCD_1_44_ClearS(uint16_t Color,uint16_t x,uint16_t y,uint16_t xx,uint16_t yy)
{
	uint16_t i,j;      
	LCD_1_44_SetWindows(x,y,xx-1,yy-1);	  
	for(i=x;i<xx;i++)
	{
		for(j=y;j<yy;j++)
		LCD_1_44_WR_DATA_16Bit(Color);	//写入数据 	 
	}
}   	
/*************************************************
函数名：LCD_1_44_SetWindows
功能：设置lcd显示窗口，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void LCD_1_44_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{
#if USE_HORIZONTAL==1	//使用横屏

	LCD_1_44_WR_REG(lcd144dev.setxcmd);	
	LCD_1_44_WR_DATA(xStar>>8);
	LCD_1_44_WR_DATA(0x00FF&xStar);		
	LCD_1_44_WR_DATA(xEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&xEnd);

	LCD_1_44_WR_REG(lcd144dev.setycmd);	
	LCD_1_44_WR_DATA(yStar>>8);
	LCD_1_44_WR_DATA(0x00FF&yStar);		
	LCD_1_44_WR_DATA(yEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&yEnd);		
#else
	
	LCD_1_44_WR_REG(lcd144dev.setxcmd);	
	LCD_1_44_WR_DATA(xStar>>8);
	LCD_1_44_WR_DATA(0x00FF&xStar);		
	LCD_1_44_WR_DATA(xEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&xEnd);

	LCD_1_44_WR_REG(lcd144dev.setycmd);	
	LCD_1_44_WR_DATA(yStar>>8);
	LCD_1_44_WR_DATA(0x00FF&yStar+0);		
	LCD_1_44_WR_DATA(yEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&yEnd+0);	
#endif

	LCD_1_44_WriteRAM_Prepare();	//开始写入GRAM				
}   

/*************************************************
函数名：LCD_1_44_SetCursor
功能：设置光标位置
入口参数：xy坐标
返回值：无
*************************************************/
void LCD_1_44_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	  	    			
	LCD_1_44_SetWindows(Xpos,Ypos,Xpos,Ypos);
} 

//设置LCD参数
//方便进行横竖屏模式切换
void LCD_1_44_SetParam(void)
{ 	
	lcd144dev.wramcmd=0x2C;
#if USE_HORIZONTAL==1	//使用横屏	  
	lcd144dev.dir=1;//横屏
	lcd144dev.width=128+3;
	lcd144dev.height=128+2;
	lcd144dev.setxcmd=0x2A;
	lcd144dev.setycmd=0x2B;			
	LCD_1_44_WriteReg(0x36,0xA8);

#else//竖屏
	lcd144dev.dir=0;//竖屏				 	 		
	lcd144dev.width=128+2;
	lcd144dev.height=128+3;
	lcd144dev.setxcmd=0x2A;
	lcd144dev.setycmd=0x2B;	
	LCD_1_44_WriteReg(0x36,0xC8);
	//LCD_1_44_WriteReg(0x36,0x1C);//如其值使用0x1C则LCD_1_44_SetWindows函数中‘+32’偏移量应取0
#endif
}	

void LCD144Test_Thread(const void *argument){

	while(1)
	{
		LCD_1_44_Clear(BLACK); //清屏

		LCD144POINT_COLOR=GRAY; 

		Show_Str(32,5,BLUE,WHITE,"系统监控",16,0);

		Show_Str(5,25,RED,YELLOW,"温度     ℃",24,1);
		LCD_1_44_ShowNum2412(5+48,25,RED,YELLOW,":24",24,1);

		Show_Str(5,50,YELLOW,YELLOW,"湿度     ％",24,1);
		LCD_1_44_ShowNum2412(5+48,50,YELLOW,YELLOW,":32",24,1);

		Show_Str(5,75,WHITE,YELLOW,"电压      Ｖ",24,1);
		LCD_1_44_ShowNum2412(5+48,75,WHITE,YELLOW,":3.2",24,1);
			
		Show_Str(5,100,GREEN,YELLOW,"电流      Ａ",24,1);
		LCD_1_44_ShowNum2412(5+48,100,GREEN,YELLOW,":0.2",24,1);
		
		delay_ms(1500);
	}
}

void LCD144Disp_Thread(const void *argument){

	static char EXT_ID,MSG_ID,ZW_ADDR;
	u8 EXTID_Disp[5],MSGID_Disp[5],WZADDR_Disp[10];
	osStatus status;
	
	LCD_1_44_Clear(BLACK); //清屏
	
	Show_Str(70,3,LGRAYBLUE,BLACK,"MSG_type:",12,1);
	Show_Str(123,4,YELLOW,BLACK,"x",12,1);
	
	Show_Str(3,3,LGRAYBLUE,BLACK,"EXT_ID:",12,1);
	Show_Str(44,3,YELLOW,BLACK,"xxxx",12,1);
	
	Show_Str(3,13,LGRAYBLUE,BLACK,"EXD_ADDR:",12,1);
	Show_Str(56,14,YELLOW,BLACK,"independent",12,1);
	
	Show_Str(5,50,YELLOW,BLACK,"未检测到",24,1);
	Show_Str(25,75,YELLOW,BLACK,"扩展模块",24,1);
	
	for(;;){
		
		if(MSG_ID != Moudle_GTA.Wirless_ID){
		
			MSG_ID = Moudle_GTA.Wirless_ID;
			LCD_1_44_ClearS(BLACK,122,0,130,13);
			
			switch(Moudle_GTA.Wirless_ID){
			
				case MID_TRANS_Zigbee:	
					
						sprintf((char *)MSGID_Disp,"Z");
						Show_Str(123,4,BRED,BLACK,MSGID_Disp,12,1);
						break;
						
				case MID_TRANS_Wifi:	
					
						sprintf((char *)MSGID_Disp,"W");
						Show_Str(123,4,BRED,BLACK,MSGID_Disp,12,1);
						break;
				
				default:sprintf((char *)MSGID_Disp,"-");
						Show_Str(123,4,LGRAYBLUE,BLACK,MSGID_Disp,12,1);
						break;
			}
		}
		
		if(EXT_ID != Moudle_GTA.Extension_ID){		//检测到模块更改，基层界面更新
		
			EXT_ID = Moudle_GTA.Extension_ID;
			if(!EXT_ID){
			
				sprintf((char *)EXTID_Disp,"NONE");
				LCD_1_44_ClearS(BLACK,43,0,70,13);
				Show_Str(44,3,BRRED,BLACK,EXTID_Disp,12,1);
			}else{
				
				sprintf((char *)EXTID_Disp,"0x%02X",Moudle_GTA.Extension_ID);
				LCD_1_44_ClearS(BLACK,43,0,70,13);
				Show_Str(44,3,BRRED,BLACK,EXTID_Disp,12,1);
			}
			
			LCD_1_44_ClearS(BLACK,56,14,128,25);
			Show_Str(56,14,YELLOW,BLACK,"independent",12,1);
		
			LCD_1_44_ClearS(BLACK,5,25,127,127);
			
			osDelay(500);
			
			switch(Moudle_GTA.Extension_ID){		

				case MID_SENSOR_FIRE:	

						Show_Str(5,50,WHITE,BLACK,"火焰监测鑞",24,1);
						Show_Str(50,75,WHITE,BLACK,"X X",24,1);
						break;
				
				case MID_SENSOR_PYRO:		
					
						Show_Str(5,50,WHITE,BLACK,"人体侦测鑞",24,1);
						Show_Str(50,75,WHITE,BLACK,"X X",24,1);
						break;
				
				case MID_SENSOR_SMOKE:

						Show_Str(5,50,WHITE,BLACK,"烟雾监测鑞",24,1);
						Show_Str(50,75,WHITE,BLACK,"X X",24,1);
						break;
						
				case MID_SENSOR_GAS:	
							
						Show_Str(5,50,WHITE,BLACK,"燃气检测鑞",24,1);
						Show_Str(50,75,WHITE,BLACK,"X X",24,1);
						break;	
				
				case MID_SENSOR_TEMP:	

						Show_Str(5,25,WHITE,BLACK,"温度鑞",24,1);
						Show_Str(50,50,WHITE,BLACK,"X X",24,1);
						Show_Str(5,75,WHITE,BLACK,"湿度鑞",24,1);
						Show_Str(50,100,WHITE,BLACK,"X X",24,1);	
						break;
				
				case MID_SENSOR_LIGHT:	
					
						Show_Str(5,50,WHITE,BLACK,"亮度检测鑞",24,1);
						Show_Str(50,75,WHITE,BLACK,"X X",24,1);
						break;	
				
				case MID_SENSOR_ANALOG:
				
						Show_Str(5,25,WHITE,BLACK,"当前电压鑞",24,1);
						Show_Str(50,50,WHITE,BLACK,"X X",24,1);
						Show_Str(5,75,WHITE,BLACK,"当前电流鑞",24,1);
						Show_Str(50,100,WHITE,BLACK,"X X",24,1);	
						break;
				
				case MID_EGUARD:
					
						Show_Str(5,50,WHITE,BLACK,"密码输入",24,1);
						Show_Str(50,80,WHITE,BLACK,"X X",24,1);
						break;
				
				case MID_EXEC_DEVIFR:		
					
						Show_Str(5,25,WHITE,BLACK,"键值鑞",24,1);
						Show_Str(50,50,GREEN,BLACK,"X X",24,1);
						Show_Str(5,75,WHITE,BLACK,"按键状态鑞",24,1);
						Show_Str(50,100,GREEN,BLACK,"X X",24,1);
						break;
				
				case MID_EXEC_DEVPWM:
					
						Show_Str(5,25,WHITE,BLACK,"开关状态鑞",24,1);
						Show_Str(50,50,GREEN,BLACK,"X X",24,1);
						Show_Str(5,75,WHITE,BLACK,"占空比值鑞",24,1);
						Show_Str(50,100,GREEN,BLACK,"X X",24,1);
						break;
				
				case MID_EXEC_CURTAIN:
					
						Show_Str(5,50,WHITE,BLACK,"窗帘状态鑞",24,1);
						Show_Str(50,75,WHITE,BLACK,"X X",24,1);
						break;	
				
				case MID_EXEC_SOURCE:
					
						Show_Str(5,25,WHITE,BLACK,"电源状态鑞",24,1);
						Show_Str(50,50,GREEN,BLACK,"X X",24,1);
						Show_Str(5,75,WHITE,BLACK,"当前电流鑞",24,1);
						Show_Str(50,100,GREEN,BLACK,"X X",24,1);	
						break;
				
				case MID_EXEC_SPEAK:
					
						Show_Str(5,50,WHITE,BLACK,"报警状态鑞",24,1);
						Show_Str(50,75,WHITE,BLACK,"X X",24,1);
						break;	

				default:
						
						Show_Str(5,50,YELLOW,BLACK,"未检测到",24,1);
						Show_Str(25,75,YELLOW,BLACK,"扩展模块",24,1);
						break;
			}
		}
		
		switch(Moudle_GTA.Extension_ID){		

			case MID_SENSOR_FIRE:{
				
						fireMS_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						evt = osMessageGet(MsgBox_DPfireMS, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,75,127,127);
							if(rptr->VAL){
							
								sprintf(disp,"有火");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}else{
							
								sprintf(disp,"没火");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}

							do{status = osPoolFree(fireMS_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_SENSOR_PYRO:{
					
						pyroMS_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						evt = osMessageGet(MsgBox_DPpyroMS, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,75,127,127);
							if(rptr->VAL){
							
								sprintf(disp,"有人");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}else{
							
								sprintf(disp,"无人");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}
							
							do{status = osPoolFree(pyroMS_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_SENSOR_SMOKE:{
					
						smokeMS_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						evt = osMessageGet(MsgBox_DPsmokeMS, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,75,127,127);
							if(rptr->VAL){
							
								sprintf(disp,"报警");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}else{
							
								sprintf(disp,"正常");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}
							
							sprintf(disp,"anaVAL:%d%%",rptr->anaDAT);
							Show_Str(10,105,BLUE,BLACK,(uint8_t *)disp,24,1);
							
							do{status = osPoolFree(smokeMS_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
					
			case MID_SENSOR_GAS:{
					
						gasMS_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						evt = osMessageGet(MsgBox_DPgasMS, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,75,127,127);
							if(rptr->VAL){
							
								sprintf(disp,"报警");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}else{
							
								sprintf(disp,"正常");
								Show_Str(40,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							}
							
							sprintf(disp,"anaVAL:%d%%",rptr->anaDAT);
							Show_Str(10,105,BLUE,BLACK,(uint8_t *)disp,24,1);
							
							do{status = osPoolFree(gasMS_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_SENSOR_TEMP:{
					
						tempMS_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						evt = osMessageGet(MsgBox_DPtempMS, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,50,127,75);
							LCD_1_44_ClearS(BLACK,0,100,127,125);
							
							sprintf(disp,"%.2f",rptr->temp);
							LCD_1_44_ShowNum2412(20,50,GREEN,BLACK,(uint8_t *)disp,24,1);
							Show_Str(20 + strlen(disp) * 24,50,GREEN,BLACK,"℃",24,1);
							sprintf(disp,"%.2f",rptr->hum);
							LCD_1_44_ShowNum2412(20,100,GREEN,BLACK,(uint8_t *)disp,24,1);
							Show_Str(20 + strlen(disp) * 24,100,GREEN,BLACK,"％",24,1);
							
							do{status = osPoolFree(tempMS_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_SENSOR_LIGHT:{
				
						lightMS_MEAS *rptr;
						osEvent  evt;
						char disp[30];
						
						evt = osMessageGet(MsgBox_DPlightMS, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,75,127,127);
							sprintf(disp,"%d %%",rptr->illumination);
							Show_Str(20 + 8*(8 - strlen(disp)),85,GREEN,BLACK,(uint8_t *)disp,24,1);
							
							do{status = osPoolFree(lightMS_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_SENSOR_ANALOG:{
					
						analogMS_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						float CUR = 0.0;
						float VOL = 0.0;
				
						evt = osMessageGet(MsgBox_DPanalogMS, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							VOL = 420.0 / 4096.0 * (float)rptr->Ich1;		//电压
							CUR = 6.1 / 4096.0 * (float)rptr->Ich2;			//电流

							LCD_1_44_ClearS(BLACK,0,50,127,75);
							LCD_1_44_ClearS(BLACK,0,100,127,125);
							
							sprintf(disp,"%.2f V",VOL);
							Show_Str(35,55,GREEN,BLACK,(uint8_t *)disp,24,1);
							
							sprintf(disp,"%.2f A",CUR);
							Show_Str(35,105,GREEN,BLACK,(uint8_t *)disp,24,1);
							
							do{status = osPoolFree(analogMS_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_EGUARD:{
					
						EGUARD_MEAS *rptr;
						EGUARD_MEAS	data_old;
						osEvent  evt;
						char disp[30];
				
						static uint8_t DP_cnt;
						static bool DP_FLG = false;
				
						if(DP_FLG){		//是否界面自恢复
						
							if(DP_cnt)DP_cnt --;
							else{
							
								DP_FLG = false;
								LCD_1_44_ClearS(BLACK,0,40,128,128);
								
								Show_Str(5,50,WHITE,BLACK,"密码输入",24,1);
								
								if(!data_old.Psd[0])Show_Str(50,80,WHITE,BLACK,"X X",24,1);
								else Show_Str(64 - strlen((const char*)data_old.Psd) * 4,80,GREEN,BLACK,(uint8_t *)data_old.Psd,24,1);
							}
						}
				
						evt = osMessageGet(MsgBox_DPEGUD, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							switch(rptr->CMD){
								
								case FID_EXERES_TTIT:
									
										DP_FLG = true;
										DP_cnt = 50;
								
										LCD_1_44_ClearS(BLACK,0,40,128,128);
										Show_Str(5,50,WHITE,BLACK,"指纹输入",24,1);
										memset(disp,0,30 * sizeof(char));
										sprintf(disp,"ID: %d",rptr->DAT);			
										Show_Str(25,80,GREEN,BLACK,(uint8_t *)disp,24,1);
										break;
							
								case RFID_EXERES_TTIT:
										
										DP_FLG = true;
										DP_cnt = 50;
								
										LCD_1_44_ClearS(BLACK,0,40,128,128);
										Show_Str(5,50,WHITE,BLACK,"RFID输入",24,1);
										memset(disp,0,30 * sizeof(char));
										sprintf(disp,"%02X%02X%02X%02X",rptr->rfidDAT[0],rptr->rfidDAT[1],rptr->rfidDAT[2],rptr->rfidDAT[3]);			
										Show_Str(25,80,GREEN,BLACK,(uint8_t *)disp,24,1);
										break;
								
								case PSD_EXERES_TTIT:
									
										memcpy(data_old.Psd, rptr->Psd, 9);
										LCD_1_44_ClearS(BLACK,0,80,127,128);
										memset(disp,0,30 * sizeof(char));
										if(!rptr->Psd[0])Show_Str(50,80,WHITE,BLACK,"X X",24,1);
										else Show_Str(64 - strlen((const char*)rptr->Psd) * 4,80,GREEN,BLACK,(uint8_t *)rptr->Psd,24,1);
										break;
									
								case PSD_EXERES_LVMSG_DN:
									
										LCD_1_44_ClearS(BLACK,0,40,128,128);
										Show_Str(20,60,YELLOW,BLACK,"开始留言",24,1);
										break;
									
								case PSD_EXERES_LVMSG_UP:
									
										DP_FLG = true;
										DP_cnt = 20;
									
										LCD_1_44_ClearS(BLACK,0,40,128,128);
										Show_Str(20,60,YELLOW,BLACK,"留言完毕",24,1);
										break;
								
								case PSD_EXERES_CALL:
									
										DP_FLG = true;
										DP_cnt = 20;
								
										LCD_1_44_ClearS(BLACK,0,40,128,128);
										Show_Str(20,60,YELLOW,BLACK,"呼叫请求",24,1);
										break;
								
								case DLOCK_MSGCMD_LOCK:
										
										DP_FLG = true;
										DP_cnt = 20;
								
										switch(rptr->DAT){
										
											case CMD_DOOROPEN:
												
													LCD_1_44_ClearS(BLACK,0,40,128,128);
													Show_Str(20,60,YELLOW,BLACK,"门锁已开",24,1);
													break;
												
											case CMD_DOORCLOSE:
												
													LCD_1_44_ClearS(BLACK,0,40,128,128);
													Show_Str(20,60,YELLOW,BLACK,"门锁已关",24,1);
													break;
												
											default:break;
										}
										break;
								
								default:break;
							}
							
							do{status = osPoolFree(EGUD_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_EXEC_DEVIFR:{
					
						IFR_MEAS *rptr;
						IFR_MEAS data_old;
						osEvent  evt;
						char disp[30];
				
						static uint8_t DP_cnt;
						static bool DP_FLG = false;
				
						if(DP_FLG){		//是否界面自恢复
						
							if(DP_cnt)DP_cnt --;
							else{
							
								DP_FLG = false;
								LCD_1_44_ClearS(BLACK,5,25,128,128);
								
								Show_Str(5,25,WHITE,BLACK,"键值鑞",24,1);
								Show_Str(5,75,WHITE,BLACK,"按键状态鑞",24,1);
								
								LCD_1_44_ClearS(BLACK,0,50,127,75);
								sprintf(disp,"%d",data_old.VAL_KEY);
								LCD_1_44_ShowNum2412(50,50,GREEN,BLACK,(uint8_t *)disp,24,1);
								
								LCD_1_44_ClearS(BLACK,0,100,127,125);
								switch(data_old.STATUS){
								
									case kifrSTATUS_NONLRN:sprintf(disp,"   NULL");break;
										
									case kifrSTATUS_WAITK:sprintf(disp,"按键等待");break;
										
									case kifrSTATUS_WAITSG:sprintf(disp,"等待遥控");break;
										
									case kifrSTATUS_LRNOVR:sprintf(disp,"学习完毕");break;
										
									case kifrSTATUS_SGOUT:sprintf(disp,"信号输出");break;
									
									default:sprintf(disp,"   NULL");break;
								}
								Show_Str(20,100,GREEN,BLACK,(uint8_t *)disp,24,1);	
							}
						}
				
						evt = osMessageGet(MsgBox_DPIFR, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							data_old.speDPCMD = rptr->speDPCMD;
							data_old.STATUS	  = rptr->STATUS;
							
							switch(rptr->speDPCMD){
							
								case SPECMD_pwmDevModADDR_CHG:{
								
									DP_FLG = true;
									DP_cnt = 50;
									
									LCD_1_44_ClearS(BLACK,0,25,128,128);
									Show_Str(10,60,YELLOW,BLACK,"WIFI_EXD_ADDR",24,1);
									Show_Str(30,80,YELLOW,BLACK,"IS SETTING",24,1);
									
									LCD_1_44_ClearS(BLACK,56,14,128,25);
									switch(rptr->Mod_addr){
									
										case ifrDevMID_video:	Show_Str(56,14,YELLOW,BLACK,"ifr-video",12,1); 
																break;
											
										case ifrDevMID_audio:	Show_Str(56,14,YELLOW,BLACK,"ifr-audio",12,1); 
																break;
											
										default:break;
									}
								}break;
								
								case SPECMD_pwmDevDATS_CHG:{
								
									if(!DP_FLG){	//禁止本显示信号成为干扰项
									
										LCD_1_44_ClearS(BLACK,0,50,127,75);
										sprintf(disp,"%d",rptr->VAL_KEY);
										LCD_1_44_ShowNum2412(50,50,GREEN,BLACK,(uint8_t *)disp,24,1);
										
										LCD_1_44_ClearS(BLACK,0,100,127,125);
										switch(rptr->STATUS){
										
											case kifrSTATUS_NONLRN:sprintf(disp,"   NULL");break;
												
											case kifrSTATUS_WAITK:sprintf(disp,"按键等待");break;
												
											case kifrSTATUS_WAITSG:sprintf(disp,"等待遥控");break;
												
											case kifrSTATUS_LRNOVR:sprintf(disp,"学习完毕");break;
												
											case kifrSTATUS_SGOUT:sprintf(disp,"信号输出");break;
											
											default:sprintf(disp,"   NULL");break;
										}
										Show_Str(20,100,GREEN,BLACK,(uint8_t *)disp,24,1);	
									}
								}break;
								
								default:break;
							}
							
							do{status = osPoolFree(IFR_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_EXEC_DEVPWM:{
			
						pwmCM_MEAS *rptr;
						pwmCM_MEAS	data_old;
						osEvent  evt;
						char disp[30];
				
						static uint8_t DP_cnt;
						static bool DP_FLG = false;
				
						if(DP_FLG){		//是否界面自恢复
						
							if(DP_cnt)DP_cnt --;
							else{
							
								DP_FLG = false;
								
								LCD_1_44_ClearS(BLACK,5,25,128,128);
								
								Show_Str(5,25,WHITE,BLACK,"开关状态鑞",24,1);
								Show_Str(5,75,WHITE,BLACK,"占空比值鑞",24,1);
								
								LCD_1_44_ClearS(BLACK,0,50,127,75);
								switch(data_old.Switch){
								
									case true:	sprintf(disp,"开启"); break;
											
									case false:	sprintf(disp,"关闭"); break;
								}
								Show_Str(40,50,GREEN,BLACK,(uint8_t *)disp,24,1);
								
								LCD_1_44_ClearS(BLACK,0,100,127,125);
								sprintf(disp,"%d%%",data_old.pwmVAL);
								Show_Str(50,100,GREEN,BLACK,(uint8_t *)disp,24,1);
							}
						}
				
						evt = osMessageGet(MsgBox_DPpwmCM, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							switch(rptr->speDPCMD){
							
								case SPECMD_ifrDevModADDR_CHG:{
								
									DP_FLG = true;
									DP_cnt = 50;
									
									LCD_1_44_ClearS(BLACK,0,25,128,128);
									Show_Str(10,60,YELLOW,BLACK,"WIFI_EXD_ADDR",24,1);
									Show_Str(30,80,YELLOW,BLACK,"IS SETTING",24,1);
									
									LCD_1_44_ClearS(BLACK,56,14,128,25);
									switch(rptr->Mod_addr){
									
										case pwmDevMID_unvarLight:	Show_Str(56,14,YELLOW,BLACK,"sw-Light",12,1); 
																	break;
											
										case pwmDevMID_varLight:	Show_Str(56,14,YELLOW,BLACK,"pwm-Light",12,1); 
																	break;
										
										case pwmDevMID_varFan:		Show_Str(56,14,YELLOW,BLACK,"pwm-Fan",12,1); 
																	break;
											
										default:break;
									}
								}break;
								
								case SPECMD_ifrDevDATS_CHG:{
								
									if(!DP_FLG){	//禁止本显示信号成为干扰项
									
										data_old.Switch = rptr->Switch;
										data_old.pwmVAL = rptr->pwmVAL;
										
										LCD_1_44_ClearS(BLACK,0,50,127,75);
										switch(rptr->Switch){
										
											case true:	sprintf(disp,"开启"); break;
													
											case false:	sprintf(disp,"关闭"); break;
										}
										Show_Str(40,50,GREEN,BLACK,(uint8_t *)disp,24,1);
										
										LCD_1_44_ClearS(BLACK,0,100,127,125);
										sprintf(disp,"%d%%",rptr->pwmVAL);
										Show_Str(50,100,GREEN,BLACK,(uint8_t *)disp,24,1);
									}
								}break;
							}
							
							do{status = osPoolFree(pwmCM_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_EXEC_CURTAIN:{
					
						curtainCM_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						evt = osMessageGet(MsgBox_DPcurtainCM, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,75,127,127);
							switch(rptr->valACT){
							
								case CMD_CURTUP:	sprintf(disp,"上升");break;
								
								case CMD_CURTSTP:	sprintf(disp,"停止");break;
								
								case CMD_CURTDN:	sprintf(disp,"下降");break;
							}
							Show_Str(30,85,GREEN,BLACK,(uint8_t *)disp,24,1);
							
							do{status = osPoolFree(curtainCM_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_EXEC_SOURCE:{
					
						sourceCM_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						float CUR;
				
						evt = osMessageGet(MsgBox_DPsourceCM, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							CUR = 6.75 / 4096.0 * (float)rptr->anaVal;
							
							LCD_1_44_ClearS(BLACK,0,75,127,100);
							switch(rptr->Switch){
							
								case true:	sprintf(disp,"开启"); break;
										
								case false:	sprintf(disp,"关闭"); break;
							}
							Show_Str(40,50,GREEN,BLACK,(uint8_t *)disp,24,1);
							
							LCD_1_44_ClearS(BLACK,0,100,127,127);
							sprintf(disp,"%.2f A",CUR);
							Show_Str(60,100,GREEN,BLACK,(uint8_t *)disp,24,1);
							
							do{status = osPoolFree(sourceCM_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;
			
			case MID_EXEC_SPEAK:{
			
						speakCM_MEAS *rptr;
						osEvent  evt;
						char disp[30];
				
						evt = osMessageGet(MsgBox_DPspeakCM, 10);
						if (evt.status == osEventMessage){
							
							rptr = evt.value.p;
							/*显示部分程序↓↓↓↓↓↓↓*/
							
							LCD_1_44_ClearS(BLACK,0,75,127,100);
							switch(rptr->spk_num){
							
								case 0:	sprintf(disp,"烟雾报警");break;
									
								case 1:	sprintf(disp,"燃气报警");break;
									
								case 2:	sprintf(disp,"火焰报警");break;
									
								case 3:	sprintf(disp,"插座报警");break;
									
								case 4:	sprintf(disp,"窗帘开");break;
								
								case 5:	sprintf(disp,"窗帘关");break;
								
								case 6:	sprintf(disp,"灯光");break;
								
								case 7:	sprintf(disp,"灯光关");break;
								
								case 8:	sprintf(disp,"灯光调亮");break;
							
								case 9:sprintf(disp,"灯光调暗");break;
								
								case 10:sprintf(disp,"灯光最亮");break;
								
								case 11:sprintf(disp,"灯光最暗");break;
								
								case 12:sprintf(disp,"电视开");break;
								
								case 13:sprintf(disp,"电视关");break;
								
								default:sprintf(disp,"一切正常");break;
							}
							
							Show_Str(10,75,GREEN,BLACK,(uint8_t *)disp,24,1);
							
							do{status = osPoolFree(speakCM_pool, rptr);}while(status != osOK);	//内存释放
							rptr = NULL;
						}
					}break;

			default:break;
		}
		osDelay(20);
	}
}

void LCD144Disp_Active(void){
	
	LCD144_Init();
	tid_LCD144Test_Thread = osThreadCreate(osThread(LCD144Disp_Thread),NULL);
}

