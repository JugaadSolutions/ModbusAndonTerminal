#include "app.h"
#include "communication.h"
#include "config.h"
#include "uart.h"

/*
*------------------------------------------------------------------------------
* Structures
*------------------------------------------------------------------------------
*/



typedef struct _EVENT_LOG
{
	UINT8 writeIndex;
	UINT8 readIndex;
	UINT8 logBuffer[MAX_ENTRIES][MAX_LENGTH_OF_ENTRY];
	UINT8 StatusBuf[MAX_ENTRIES * 2];
	UINT8 StatBufIndex;
	
}EVENT_LOG;


/*
*------------------------------------------------------------------------------
* Variables
*------------------------------------------------------------------------------
*/

#pragma idata LOG_DATA
EVENT_LOG log = {0};
#pragma idata


/*------------------------------------------------------------------------------
* Private Functions
*------------------------------------------------------------------------------
*/

void logWrite( far UINT8 **data, UINT8 length );
UINT8 logRead( far UINT8 **logBuff );
UINT8 APP_comCallBack( far UINT8 *rxPacket, far UINT8* txCode,far UINT8** txPacket);
/*
*------------------------------------------------------------------------------
* void APP-init(void)
*------------------------------------------------------------------------------
*/

void APP_init(void)
{
	UINT8 i;
	//Modbus
	UINT16 sbaudrate = 9600, saddress = 0x0A;
	eMBErrorCode    eStatus;

	//modbus configuration
	eStatus = eMBInit( MB_RTU, ( UCHAR )saddress, 0, sbaudrate, MB_PAR_NONE);
	eStatus = eMBEnable(  );	/* Enable the Modbus Protocol Stack. */

	COM_init(CMD_SOP , CMD_EOP ,RESP_SOP , RESP_EOP , APP_comCallBack);

	for(i = 0; i < MAX_ENTRIES * 2; i++)
		log.StatusBuf[i] = 0XFF;

}


/*
*------------------------------------------------------------------------------
* void APP_task(void)
*------------------------------------------------------------------------------
*/

void APP_task(void)
{
	UINT8 data = 0;

	if( UART_hasData() )
	{
		data = UART_read();
		UART_write( data );
	}


}


/*---------------------------------------------------------------------------------------------------------------
*	void updateLog(void)
*----------------------------------------------------------------------------------------------------------------
*/
void logWrite( far UINT8 **data, UINT8 length )
{
	UINT8 i, j = 0;
	UINT8 temp = 0;
	UINT8 tempData = 0;
 
	length = length * 2;   //length multiplied by number of bytes

	
	log.StatBufIndex = **data;
	(*data)++;
	tempData = **data;

	if(log.StatusBuf[log.StatBufIndex+1] != tempData)
	{
		(*data)--;
		//Store data in the buffer
		for( i = 0, j =0; i < length; i++ )
		{
			temp = **data ;
	
	
			log.logBuffer[log.writeIndex][j++] = (temp / 10) + '0';
			temp = **data ;
			log.logBuffer[log.writeIndex][j++] = (temp % 10) + '0';
			
			(*data)++;
		}
	
		log.logBuffer[log.writeIndex][j] = '\0'; //store null character to indicate end of the packet

		log.writeIndex++;
		if( log.writeIndex >= MAX_ENTRIES)
			log.writeIndex = 0;

		//store previous state
		log.StatusBuf[log.StatBufIndex+1] = tempData;
	}

}

/*---------------------------------------------------------------------------------------------------------------
*	UINT8 logRead(void)
*----------------------------------------------------------------------------------------------------------------
*/
UINT8 logRead( far UINT8** logBuff )
{
	UINT8 length = 0;

	if( log.writeIndex == log.readIndex )
	{
		*logBuff = 0;
	}

/*	if((log.logBuffer[log.readIndex][0] == '\0') || (log.writeIndex == log.readIndex))
		return *logBuff = 0; */
	else
	{
		*logBuff = log.logBuffer[log.readIndex];
		length = strlen( log.logBuffer[log.readIndex] );
		//length = 4;
		
		log.readIndex++;
		if( log.readIndex >= MAX_ENTRIES)
			log.readIndex = 0;	
	}

	return length;
			
}

/*---------------------------------------------------------------------------------------------------------------
*	Modbus Callback Functions
*----------------------------------------------------------------------------------------------------------------
*/
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{

    eMBErrorCode    eStatus = MB_ENOERR;

    int             iRegIndex;

//	LED0_IO ^= 1;
//	xMBPortEventPost( EV_READY );
    return eStatus;

}

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs,
                 eMBRegisterMode eMode )
{

	UINT8	starting_add = usAddress;
	UINT8	length	 = usNRegs;
	eMBErrorCode    eStatus = MB_ENOERR;

	logWrite( &pucRegBuffer, length );  // call log write to store the data


//    LED0_IO ^= 1;
//	xMBPortEventPost( EV_READY );

	switch(eMode)
	{
	case MB_REG_WRITE:



    break;

 	case MB_REG_READ: 


   	 break;
	}
	return eStatus;
  }


eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
               eMBRegisterMode eMode )
{
//	LED0_IO ^= 1;
//	xMBPortEventPost( EV_READY );
    return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
//	LED0_IO ^= 1;
//	xMBPortEventPost( EV_READY );
    return MB_ENOREG;
}


/*---------------------------------------------------------------------------------------------------------------
*	APP CALLBACK
*----------------------------------------------------------------------------------------------------------------
*/
UINT8 APP_comCallBack( far UINT8 *rxPacket, far UINT8* txCode,far UINT8** txPacket)
{

	UINT8 i;

	UINT8 rxCode = rxPacket[0];
	volatile UINT8 length = 0;

	switch( rxCode )
	{
		case CMD_GET_LOG_STATUS:
			length = (UINT8) logRead(txPacket);
			*txCode = CMD_GET_LOG_STATUS;
		break;

		default:
			length = 0;
			
		break;

	}

	return length;

}