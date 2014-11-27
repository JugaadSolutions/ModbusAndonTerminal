#ifndef APP_H
#define APP_H

#include "port.h"
#include "mbport.h"
#include "mb.h"



#define MAX_ENTRIES 32
#define	MAX_LENGTH_OF_ENTRY 9

enum
{
	CMD_GET_LOG_STATUS = 0X80
};

void APP_init(void);
void APP_task(void);

#endif