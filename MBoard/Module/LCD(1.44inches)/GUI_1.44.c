#include "LCD_1.44.h"
#include "FONT_1.44.h" 
#include "string.h"
//******************************************************************
//函数名：  LCD144_LCD144GUI_DrawPoint
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    GUI描绘一个点
//输入参数：x:光标位置x坐标
//        	y:光标位置y坐标
//			color:要填充的颜色
//返回值：  无
//修改记录：无
//******************************************************************

extern uint16_t LCD144POINT_COLOR,LCD144BACK_COLOR;

void LCD144_LCD144GUI_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_1_44_SetCursor(x,y);//设置光标位置 
	LCD_1_44_WR_DATA_16Bit(color); 
}

//******************************************************************
//函数名：  LCD_1_44_Fill
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    在指定区域内填充颜色
//输入参数：sx:指定区域开始点x坐标
//        	sy:指定区域开始点y坐标
//			ex:指定区域结束点x坐标
//			ey:指定区域结束点y坐标
//        	color:要填充的颜色
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{  	

	uint16_t i,j;			
	uint16_t width=ex-sx+1; 		//得到填充的宽度
	uint16_t height=ey-sy+1;		//高度
	LCD_1_44_SetWindows(sx,sy,ex-1,ey-1);//设置显示窗口
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		LCD_1_44_WR_DATA_16Bit(color);	//写入数据 	 
	}

	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);//恢复窗口设置为全屏
}

//******************************************************************
//函数名：  LCD_1_44_DrawLine
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    GUI画线
//输入参数：x1,y1:起点坐标
//        	x2,y2:终点坐标 
//返回值：  无
//修改记录：无
//****************************************************************** 
void LCD_1_44_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_1_44_DrawPoint(uRow,uCol);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

//******************************************************************
//函数名：  LCD_1_44_DrawRectangle
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    GUI画矩形(非填充)
//输入参数：(x1,y1),(x2,y2):矩形的对角坐标
//返回值：  无
//修改记录：无
//******************************************************************  
void LCD_1_44_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_1_44_DrawLine(x1,y1,x2,y1);
	LCD_1_44_DrawLine(x1,y1,x1,y2);
	LCD_1_44_DrawLine(x1,y2,x2,y2);
	LCD_1_44_DrawLine(x2,y1,x2,y2);
}  

//******************************************************************
//函数名：  LCD_1_44_DrawFillRectangle
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    GUI画矩形(填充)
//输入参数：(x1,y1),(x2,y2):矩形的对角坐标
//返回值：  无
//修改记录：无
//******************************************************************   
void LCD_1_44_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_1_44_Fill(x1,y1,x2,y2,LCD144POINT_COLOR);

}
 
//******************************************************************
//函数名：  _draw_circle_8
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    8对称性画圆算法(内部调用)
//输入参数：(xc,yc) :圆中心坐标
// 			(x,y):光标相对于圆心的坐标
//         	c:填充的颜色
//返回值：  无
//修改记录：无
//******************************************************************  
void _draw_circle_8(int xc, int yc, int x, int y, uint16_t c)
{
	LCD144_LCD144GUI_DrawPoint(xc + x, yc + y, c);

	LCD144_LCD144GUI_DrawPoint(xc - x, yc + y, c);

	LCD144_LCD144GUI_DrawPoint(xc + x, yc - y, c);

	LCD144_LCD144GUI_DrawPoint(xc - x, yc - y, c);

	LCD144_LCD144GUI_DrawPoint(xc + y, yc + x, c);

	LCD144_LCD144GUI_DrawPoint(xc - y, yc + x, c);

	LCD144_LCD144GUI_DrawPoint(xc + y, yc - x, c);

	LCD144_LCD144GUI_DrawPoint(xc - y, yc - x, c);
}

//******************************************************************
//函数名：  gui_circle
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    在指定位置画一个指定大小的圆(填充)
//输入参数：(xc,yc) :圆中心坐标
//         	c:填充的颜色
//		 	r:圆半径
//		 	fill:填充判断标志，1-填充，0-不填充
//返回值：  无
//修改记录：无
//******************************************************************  
void gui_circle(int xc, int yc,uint16_t c,int r, int fill)
{
	int x = 0, y = r, yi, d;

	d = 3 - 2 * r;


	if (fill) 
	{
		// 如果填充（画实心圆）
		while (x <= y) {
			for (yi = x; yi <= y; yi++)
				_draw_circle_8(xc, yc, x, yi, c);

			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	} else 
	{
		// 如果不填充（画空心圆）
		while (x <= y) {
			_draw_circle_8(xc, yc, x, y, c);
			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
}

//******************************************************************
//函数名：  LCD_1_44_ShowChar
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示单个英文字符
//输入参数：(x,y):字符显示位置起始坐标
//        	fc:前置画笔颜色
//			bc:背景颜色
//			num:数值（0-94）
//			size:字体大小
//			mode:模式  0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//******************************************************************  
void LCD_1_44_ShowChar(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode)
{  
    uint8_t temp;
    uint8_t pos,t;
	uint16_t colortemp=LCD144POINT_COLOR;      
		   
	num=num-' ';//得到偏移后的值
	LCD_1_44_SetWindows(x,y,x+size/2-1,y+size-1);//设置单个文字显示窗口
	if(!mode) //非叠加方式
	{
		
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=LCD144asc2_1206[num][pos];//调用1206字体
			else temp=LCD144asc2_1608[num][pos];		 //调用1608字体
			for(t=0;t<size/2;t++)
		    {                 
		        if(temp&0x01)LCD_1_44_WR_DATA(fc); 
				else LCD_1_44_WR_DATA(bc); 
				temp>>=1; 
				
		    }
			
		}	
	}else//叠加方式
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=LCD144asc2_1206[num][pos];//调用1206字体
			else temp=LCD144asc2_1608[num][pos];		 //调用1608字体
			for(t=0;t<size/2;t++)
		    {   
				LCD144POINT_COLOR=fc;              
		        if(temp&0x01)LCD_1_44_DrawPoint(x+t,y+pos);//画一个点    
		        temp>>=1; 
		    }
		}
	}
	LCD144POINT_COLOR=colortemp;	
	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);//恢复窗口为全屏    	   	 	  
} 

//******************************************************************
//函数名：  LCD_1_44_ShowChar
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示单个英文字符
//输入参数：(x,y):字符显示位置起始坐标
//        	fc:前置画笔颜色
//			bc:背景颜色
//			num:数值（0-94）
//			size:字体大小
//			mode:模式  0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//******************************************************************  
void LCD_1_44_ShowNum2412(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc,uint8_t *p ,uint8_t size,uint8_t mode)
{  
    uint16_t temp;
    uint8_t pos,t;
	uint16_t colortemp=LCD144POINT_COLOR;      
	uint16_t x0=x;
	uint16_t y0=y; 
	uint8_t num=0;
	

    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {   

		if(x>(lcd144dev.width-1)||y>(lcd144dev.height-1)) 
		return;   
		num=*p;
		if(':'==num) //特殊字符":"
		num=10;	
		else if('.'==num)//特殊字符"."
		num=11;	
		else if('-'==num)//特殊字符"."
		num=12;	
		else  //纯数字   	
		num=num-'0';   
		x0=x;
		    
		for(pos=0;pos<48;pos++)
		{
			temp=LCD144asc2_2412[num][pos];
			for(t=0;t<8;t++)
		    {   
				LCD144POINT_COLOR=fc;              
		        if(temp&0x80)LCD_1_44_DrawPoint(x,y);//画一个点  
				//else LCD_1_44_WR_DATA_16Bit(bc);   
		        temp<<=1; 
				x++;
				if((x-x0)==12)
				{
					x=x0;
					y++;
					break;
				}
		    }
		}
	if(num<10)
	x+=16;	//人为控制字距，使得排版更好看，原值为12
	else
	x+=8; //人为控制字距，使得排版更好看，原值为12

	y=y0;
    p++;
    }  
	LCD144POINT_COLOR=colortemp;	 	 	  
} 


//******************************************************************
//函数名：  LCD_1_44_ShowString
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示英文字符串
//输入参数：x,y :起点坐标	 
//			size:字体大小
//			*p:字符串起始地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//******************************************************************  	  
void LCD_1_44_ShowString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p,uint8_t mode)
{         
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {   
		if(x>(lcd144dev.width-1)||y>(lcd144dev.height-1)) 
		return;     
        LCD_1_44_ShowChar(x,y,LCD144POINT_COLOR,LCD144BACK_COLOR,*p,size,mode);
        x+=size/2;
        p++;
    }  
} 

//******************************************************************
//函数名：  mypow
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    求m的n次方(gui内部调用)
//输入参数：m:乘数
//	        n:幂
//返回值：  m的n次方
//修改记录：无
//******************************************************************  
u32 mypow(uint8_t m,uint8_t n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

//******************************************************************
//函数名：  LCD_1_44_ShowNum
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示单个数字变量值
//输入参数：x,y :起点坐标	 
//			len :指定显示数字的位数
//			size:字体大小(12,16)
//			color:颜色
//			num:数值(0~4294967295)
//返回值：  无
//修改记录：无
//******************************************************************  			 
void LCD_1_44_ShowNum(uint16_t x,uint16_t y,u32 num,uint8_t len,uint8_t size)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_1_44_ShowChar(x+(size/2)*t,y,LCD144POINT_COLOR,LCD144BACK_COLOR,' ',size,1);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_1_44_ShowChar(x+(size/2)*t,y,LCD144POINT_COLOR,LCD144BACK_COLOR,temp+'0',size,1); 
	}
} 

//******************************************************************
//函数名：  LCD144GUI_DrawFont16
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示单个16X16中文字体
//输入参数：x,y :起点坐标
//			fc:前置画笔颜色
//			bc:背景颜色	 
//			s:字符串地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//******************************************************************
void LCD144GUI_DrawFont16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode)
{
	uint8_t i,j;
	uint16_t k;
	uint16_t HZnum;
	uint16_t x0=x;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//自动统计汉字数目
	
			
	for (k=0;k<HZnum;k++) 
	{
	  if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
	  { 	LCD_1_44_SetWindows(x,y,x+16-1,y+16-1);
		    for(i=0;i<16*2;i++)
		    {
				for(j=0;j<8;j++)
		    	{	
					if(!mode) //非叠加方式
					{
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_1_44_WR_DATA_16Bit(fc);
						else LCD_1_44_WR_DATA_16Bit(bc);
					}
					else
					{
						LCD144POINT_COLOR=fc;
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_1_44_DrawPoint(x,y);//画一个点
						x++;
						if((x-x0)==16)
						{
							x=x0;
							y++;
							break;
						}
					}

				}
				
			}
			
			
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}

	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);//恢复窗口为全屏  
} 

//******************************************************************
//函数名：  LCD144GUI_DrawFont24
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示单个24X24中文字体
//输入参数：x,y :起点坐标
//			fc:前置画笔颜色
//			bc:背景颜色	 
//			s:字符串地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//******************************************************************
void LCD144GUI_DrawFont24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode)
{
	uint8_t i,j;
	uint16_t k;
	uint16_t HZnum;
	uint16_t x0=x;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//自动统计汉字数目
		
			for (k=0;k<HZnum;k++) 
			{
			  if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
			  { 	LCD_1_44_SetWindows(x,y,x+24-1,y+24-1);
				    for(i=0;i<24*3;i++)
				    {
							for(j=0;j<8;j++)
							{
								if(!mode) //非叠加方式
								{
									if(tfont24[k].Msk[i]&(0x80>>j))	LCD_1_44_WR_DATA_16Bit(fc);
									else LCD_1_44_WR_DATA_16Bit(bc);
								}
							else
							{
								LCD144POINT_COLOR=fc;
								if(tfont24[k].Msk[i]&(0x80>>j))	LCD_1_44_DrawPoint(x,y);//画一个点
								x++;
								if((x-x0)==24)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
			}

	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);//恢复窗口为全屏  
}

//******************************************************************
//函数名：  LCD144GUI_DrawFont32
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示单个32X32中文字体
//输入参数：x,y :起点坐标
//			fc:前置画笔颜色
//			bc:背景颜色	 
//			s:字符串地址
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//****************************************************************** 
void LCD144GUI_DrawFont32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode)
{
	uint8_t i,j;
	uint16_t k;
	uint16_t HZnum;
	uint16_t x0=x;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//自动统计汉字数目
	for (k=0;k<HZnum;k++) 
			{
			  if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
			  { 	LCD_1_44_SetWindows(x,y,x+32-1,y+32-1);
				    for(i=0;i<32*4;i++)
				    {
						for(j=0;j<8;j++)
				    	{
							if(!mode) //非叠加方式
							{
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_1_44_WR_DATA_16Bit(fc);
								else LCD_1_44_WR_DATA_16Bit(bc);
							}
							else
							{
								LCD144POINT_COLOR=fc;
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_1_44_DrawPoint(x,y);//画一个点
								x++;
								if((x-x0)==32)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
			}
	
	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);//恢复窗口为全屏  
} 

//******************************************************************
//函数名：  Show_Str
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示一个字符串,包含中英文显示
//输入参数：x,y :起点坐标
// 			fc:前置画笔颜色
//			bc:背景颜色
//			str :字符串	 
//			size:字体大小
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//******************************************************************    	   		   
void Show_Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode)
{					
	uint16_t x0=x;							  	  
  	uint8_t bHz=0;     //字符或者中文 
    while(*str!=0)//数据未结束
    { 
        if(!bHz)
        {
			if(x>(lcd144dev.width-size/2)||y>(lcd144dev.height-size)) 
			return; 
	        if(*str>0x80)bHz=1;//中文 
	        else              //字符
	        {          
		        if(*str==0x0D)//换行符号
		        {         
		            y+=size;
						x=x0;
		            str++; 
		        }  
		        else
					{
					if(size==12||size==16)
					{  
					LCD_1_44_ShowChar(x,y,fc,bc,*str,size,mode);
					x+=size/2; //字符,为全字的一半 
					}
					else//字库中没有集成16X32的英文字体,用8X16代替
					{
					 	LCD_1_44_ShowChar(x,y,fc,bc,*str,16,mode);
						x+=8; //字符,为全字的一半 
					}
				} 
				str++; 
		        
	        }
        }else//中文 
        {   
			if(x>(lcd144dev.width-size)||y>(lcd144dev.height-size)) 
			return;  
            bHz=0;//有汉字库    
			if(size==32)
			LCD144GUI_DrawFont32(x,y,fc,bc,str,mode);	 	
			else if(size==24)
			LCD144GUI_DrawFont24(x,y,fc,bc,str,mode);	
			else
			LCD144GUI_DrawFont16(x,y,fc,bc,str,mode);
				
	        str+=2; 
	        x+=size;//下一个汉字偏移	    
        }						 
    }   
}

//******************************************************************
//函数名：  Gui_StrCenter
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    居中显示一个字符串,包含中英文显示
//输入参数：x,y :起点坐标
// 			fc:前置画笔颜色
//			bc:背景颜色
//			str :字符串	 
//			size:字体大小
//			mode:模式	0,填充模式;1,叠加模式
//返回值：  无
//修改记录：无
//******************************************************************   
void Gui_StrCenter(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode)
{
	uint16_t len=strlen((const char *)str);
	uint16_t x1=(lcd144dev.width-len*8)/2;
	Show_Str(x+x1,y,fc,bc,str,size,mode);
} 
 
//******************************************************************
//函数名：  Gui_Drawbmp16
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    显示一副16位BMP图像
//输入参数：x,y :起点坐标
// 			*p :图像数组起始地址
//返回值：  无
//修改记录：无
//******************************************************************  
void Gui_Drawbmp16(uint16_t x,uint16_t y,const unsigned char *p) //显示40*40 QQ图片
{
  	int i; 
	unsigned char picH,picL; 
	LCD_1_44_SetWindows(x,y,x+40-1,y+40-1);//窗口设置
    for(i=0;i<40*40;i++)
	{	
	 	picL=*(p+i*2);	//数据低位在前
		picH=*(p+i*2+1);				
		LCD_1_44_WR_DATA_16Bit(picH<<8|picL);  						
	}	
	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);//恢复显示窗口为全屏	
}
