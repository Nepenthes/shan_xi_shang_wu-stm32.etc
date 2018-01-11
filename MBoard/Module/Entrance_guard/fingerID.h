#ifndef _FINGERID_H_
#define _FINGERID_H_

#include "Eguard.h"

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

#define FID_MSGCMD_FIDSAVE	0x0A
#define FID_MSGCMD_FIDDELE	0x0B
#define FID_MSGCMD_FIDIDEN	0x0C

#define FID_EXERES_SUCCESS	0x0A
#define FID_EXERES_FAIL		0x0B
#define FID_EXERES_TTIT		0xCC

void fingerID_Thread(const void *argument);
void fingerIDThread_Active(void);

#endif

