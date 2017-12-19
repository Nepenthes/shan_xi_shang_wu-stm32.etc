#ifndef	_FINGERID_H_
#define	_FINGERID_H_

#include "IO_Map.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"
#include "delay.h"

#define FID_IDENT_CMD	0x01
#define FID_IDENT_NXT	0x02
#define FID_IDENT_END	0x08

#define FID_IDENT_ANS	0x07

#define FID_CFM_OK		0x00
#define FID_CFM_ERR		0x01
#define FID_CFM_NFG		0x02

#define FID_CMD_GETI	0x01
#define FID_CMD_GENC	0x02
#define FID_CMD_MATC	0x03
#define FID_CMD_SRCH	0x04
#define FID_CMD_REGM	0x05
#define FID_CMD_STOC	0x06
#define FID_CMD_DELM	0x0C

#define FID_MSGCMD_FIDSAVE	0x10
#define FID_MSGCMD_FIDDELE	0x20
#define FID_MSGCMD_FIDIDEN	0x30

typedef struct{

	u8 CMD;
	u8 DAT;
}FID_MEAS;

extern osPoolId  	 FIDpool;
extern osMessageQId  MsgBox;

void fingerID_Thread(const void *argument);

void fingerID_Active(void);

#endif

