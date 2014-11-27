/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- static functions ---------------------------------*/
 void prvvUARTTxReadyISR( void );
void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    /* If xRXEnable enable serial receive interrupts. If xTxENable enable
     * transmitter empty interrupts.
     */
	if( xRxEnable == FALSE )
	{
		RCSTAbits.CREN = 0;
		
	}
	else
	{
		RCSTAbits.CREN = 1;
	}

	if( xTxEnable == FALSE )
	{
	//	PIE1bits.TXIE = 0;
	}
	else
	{	
		pxMBFrameCBTransmitterEmpty(  );
	}

}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	unsigned char uartConfig= USART_TX_INT_OFF &
				USART_RX_INT_ON &
				USART_ASYNCH_MODE &
				USART_EIGHT_BIT &
				USART_CONT_RX &
				USART_BRGH_HIGH;
// configure USART

	unsigned char bdValue;
	switch(ulBaudRate)
	{
		case 2400:
			bdValue = 207;	//2400bps
		break;

		case 9600:
			bdValue = 162;	//9600bps
		break;

		case 19200				:
			bdValue = 25;	//19200bps
		break;

		case 57600:
			bdValue = 8;	//57600bps
		break;
		default:
			bdValue = 162;		//default baudrate = 9600
		break;
	}
#if defined (__18F8722) || defined (__18F67J60)
	Open1USART( uartConfig ,
				bdValue); 
#else
	OpenUSART( uartConfig ,
				bdValue); 
#endif		
		
  	IPR1bits.RC1IP = 1;	// Make receive interrupt high priority
//	IPR1bits.TXIP = 1;	// Make transmit interrupt high priority

#ifdef __18F67J60
	RCSTA1bits.SPEN = 1;
	PIE1bits.RCIE = 1;
	PIR1bits.RCIF = 0;
#endif

    return TRUE;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    /* Put a byte in the UARTs transmit buffer. This function is called
     * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
     * called. */
#if defined (__18F8722) || defined (__18F67J60)
	TX1_EN = 1;
#else	
	TX_EN = 1;
#endif
	Delay10us(10);
	TXREG1 = ucByte;	//transmit
#if defined (__18F8722) || defined (__18F67J60)
	while(Busy1USART());
	TX1_EN = 0;
#else
	while(BusyUSART());
	TX_EN = 0;
#endif
	Delay10us(10);
    return TRUE;

}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    /* Return the byte in the UARTs receive buffer. This function is called
     * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
     */
	volatile unsigned char data;
#if defined (__18F8722) || defined (__18F67J60)
	data = Read1USART();
#else
	data = ReadUSART();
#endif

	*pucByte = data;

	
    PIR1bits.RC1IF = 0;	// Clear the interrupt flag
    return TRUE;
}

/* Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */

#pragma interrupt prvvUARTTxReadyISR
void prvvUARTTxReadyISR( void )
{
    pxMBFrameCBTransmitterEmpty(  );
}

/* Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */

#pragma interrupt prvvUARTRxISR
void prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}
