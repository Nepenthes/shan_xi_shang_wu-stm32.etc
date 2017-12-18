#ifndef __LCD_144_H
#define __LCD_144_H	

#define osObjectsPublic                     // define objects in main module

#include "stdlib.h"
#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "GUI_1.44.h"
#include "stdio.h"
#include "Driver_USART.h"
#include "string.h"

#include "delay.h"

//LCD重要参数集
typedef struct  
{										    
	uint16_t width;			//LCD 宽度
	uint16_t height;			//LCD 高度
	uint16_t id;				//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	 wramcmd;		//开始写gram指令
	uint16_t  setxcmd;		//设置x坐标指令
	uint16_t  setycmd;		//设置y坐标指令	 
}_lcd144_dev; 	

//LCD参数
extern _lcd144_dev lcd144dev;	//管理LCD重要参数
/////////////////////////////////////用户配置区///////////////////////////////////	 
//支持横竖屏快速定义切换，支持8/16位模式切换
#define USE_HORIZONTAL  	0	//定义是否使用横屏 		0,不使用.1,使用.
//使用硬件SPI 还是模拟SPI作为测试
#define USE_HARDWARE_SPI  1  //1:Enable Hardware SPI;0:USE Soft SPI
//////////////////////////////////////////////////////////////////////////////////	  
//定义LCD的尺寸
#if USE_HORIZONTAL==1	//使用横屏
#define LCD_1_44_W 320
#define LCD_1_44_H 240
#else
#define LCD_1_44_W 240
#define LCD_1_44_H 320
#endif

//TFTLCD部分外要调用的函数		   
extern uint16_t  POINT_COLOR;//默认红色    
extern uint16_t  BACK_COLOR; //背景颜色.默认为白色

////////////////////////////////////////////////////////////////////
//-----------------LCD端口定义---------------- 
//QDtech全系列模块采用了三极管控制背光亮灭，用户也可以接PWM调节背光亮度
//#define LCD_1_44_LED        	GPIO_Pin_9  //PB9 连接至TFT -LED
//接口定义在Lcd_Driver.h内定义，请根据接线修改并修改相应IO初始化LCD_1_44_GPIO_Init()

//#define LCD_1_44_CTRL   	  	GPIOB		//定义TFT数据端口
//#define LCD_1_44_RS         	GPIO_Pin_10	//PB10连接至TFT --RS
//#define LCD_1_44_CS        	GPIO_Pin_11 //PB11 连接至TFT --CS
//#define LCD_1_44_RST     	GPIO_Pin_12	//PB12连接至TFT --RST
//#define LCD_1_44_SCL        	GPIO_Pin_13	//PB13连接至TFT -- CLK
//#define LCD_1_44_SDA        	GPIO_Pin_15	//PB15连接至TFT - SDI

//////////////////////////////////////////////////////////////////////
//液晶控制口置1操作语句宏定义
#define	LCD_1_44_CS_SET  		GPIOA->BSRR = GPIO_Pin_15 
#define	LCD_1_44_RS_SET  		GPIOC->BSRR = GPIO_Pin_5  
#define	LCD_1_44_SDA_SET  	GPIOB->BSRR = GPIO_Pin_5 
#define	LCD_1_44_SCL_SET  	GPIOB->BSRR = GPIO_Pin_3  
#define	LCD_1_44_RST_SET  	GPIOC->BSRR = GPIO_Pin_4
//#define	LCD_LED_SET  	LCD_CTRL->BSRR=LCD_LED   

//液晶控制口置0操作语句宏定义
#define	LCD_1_44_CS_CLR  		GPIOA->BRR = GPIO_Pin_15  
#define	LCD_1_44_RS_CLR  		GPIOC->BRR = GPIO_Pin_5    
#define	LCD_1_44_SDA_CLR  	GPIOB->BRR = GPIO_Pin_5    
#define	LCD_1_44_SCL_CLR  	GPIOB->BRR = GPIO_Pin_3    
#define	LCD_1_44_RST_CLR  	GPIOC->BRR = GPIO_Pin_4 
//#define	LCD_LED_CLR  	LCD_CTRL->BRR=LCD_LED 

//画笔颜色
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 		  0XFFE0
#define GBLUE			0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 //棕色
#define BRRED 			0XFC07 //棕红色
#define GRAY  			0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	0X841F //浅绿色
//#define LIGHTGRAY     0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 		0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE      	0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE          0X2B12 //浅棕蓝色(选择条目的反色)
	    															  
extern uint16_t BACK_COLOR, POINT_COLOR ;  

void LCD144_Init(void);
void LCD_1_44_DisplayOn(void);
void LCD_1_44_DisplayOff(void);
void LCD_1_44_Clear(uint16_t Color);	
void LCD_1_44_ClearS(uint16_t Color,uint16_t x,uint16_t y,uint16_t xx,uint16_t yy);
void LCD_1_44_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_1_44_DrawPoint(uint16_t x,uint16_t y);//画点
uint16_t  LCD_1_44_ReadPoint(uint16_t x,uint16_t y); //读点
void LCD_1_44_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_1_44_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   
void LCD_1_44_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);
void LCD_1_44_DrawPoint_16Bit(uint16_t color);
uint16_t LCD_1_44_RD_DATA(void);//读取LCD数据									    
void LCD_1_44_WriteReg(uint16_t LCD_1_44_Reg, uint16_t LCD_1_44_RegValue);
void LCD_1_44_WR_DATA(uint8_t data);
void LCD_1_44_WR_DATA_16Bit(uint16_t data);
uint16_t LCD_1_44_ReadReg(uint8_t LCD_1_44_Reg);
void LCD_1_44_WriteRAM_Prepare(void);
void LCD_1_44_WriteRAM(uint16_t RGB_Code);
uint16_t LCD_1_44_ReadRAM(void);		   
uint16_t LCD_1_44_BGR2RGB(uint16_t c);
void LCD_1_44_SetParam(void);

void LCD144Test_Thread(const void *argument);
void LCD144_Thread(const void *argument);

void LCD144_test(void);
						  		 
#endif  
	 
	 



