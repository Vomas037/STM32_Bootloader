#ifndef __BACKUP_H
#define __BACKUP_H

#include "rtc.h"


void BKP_EnableAccess(void);
void BKP_WriteFlag(uint16_t flag);
uint16_t BKP_ReadFlag(void);


#endif
