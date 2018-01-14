#ifndef _DOORLOCK_H_
#define _DOORLOCK_H_

#include "Eguard.h"

#define DLOCK_CON	PAout(0)

#define CMD_DOOROPEN	0x01
#define CMD_DOORCLOSE	0x02

#define DLOCK_EXERES_TTIT	0xFF

#define DLOCK_MSGCMD_LOCK	0x1A

void doorLock_Init(void);
void doorLock_Thread(const void *argument);
void doorLockThread_Active(void);

#endif

