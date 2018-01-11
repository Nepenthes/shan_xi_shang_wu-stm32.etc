#ifndef _RFID_H_
#define _RFID_H_

#include "Eguard.h"
#include "rc522_config.h"

#define	  RFID_EXERES_TTIT		0xDD

#define   macRC522_DELAY()  delay_us(100)

#define   macDummy_Data  0x00

void             PcdReset                   ( void );                       //复位
void             M500PcdConfigISOType       ( u8 type );                    //工作方式
char             PcdRequest                 ( u8 req_code, u8 * pTagType ); //寻卡
char             PcdAnticoll                ( u8 * pSnr);                   //读卡号

void rfID_Thread(const void *argument);
void rfIDThread_Active(void);

#endif

