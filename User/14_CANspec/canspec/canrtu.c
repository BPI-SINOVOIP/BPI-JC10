/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canrtu.c
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec send and receive data.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */


/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include <Myproject.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "canspec.h"
#include "canrtu.h"

#include "canport.h"

#if COMM_CAN_ENABLED > 0

/* ----------------------- Defines ------------------------------------------*/
#define CS_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define CS_SER_PDU_SIZE_MAX     22      /*!< Maximum size of a Modbus RTU frame. */

#define CS_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */

#define CS_SER_PDU_PDU_OFF      0       /*!< Offset of Modbus-PDU in Ser-PDU. */
#define CS_SER_PDU_FUNC_OFF     0       /*!< Offset of Modbus-Func in Ser-PDU. */


#define CS_RCV_BUFFFER_SIZE     64       /*!< Size of DMA receive buffer. */

/* ----------------------- Type definitions ---------------------------------*/
typedef enum
{
	STATE_RX_INIT,              /*!< Receiver is in initial state. */
	STATE_RX_IDLE,              /*!< Receiver is in idle state. */
	STATE_RX_RCV,               /*!< Frame is beeing received. */
	STATE_RX_ERROR              /*!< If the frame is invalid. */
} eCSRcvState;

typedef enum
{
	STATE_TX_IDLE,              /*!< Transmitter is in idle state. */
	STATE_TX_XMIT               /*!< Transmitter is in transfer state. */
} eCSSndState;

/* ----------------------- Static variables ---------------------------------*/
static volatile eCSSndState eSndState;
static volatile eCSRcvState eRcvState;

volatile UCHAR* pucRTUBuf = CanspecFT.data;

static volatile USHORT csSndAddrCount = 0;
static volatile USHORT csSndOffsetCount = 0;


/* ----------------------- Start implementation -----------------------------*/
void eCSRTUInit(USHORT baudrate)
{
	/* Modbus RTU uses 8 Databits. */ 
	xCSPortCANInit(baudrate);
}

void eCSRTUStart(void)
{
	/* Initially the receiver is in the state STATE_RX_INIT. we start
	 * the timer and if no character is received within t3.5 we change
	 * to STATE_RX_IDLE. This makes sure that we delay startup of the
	 * modbus protocol stack until the bus is free.
	 */
	eRcvState = STATE_RX_INIT;
	vCSPortCANStart();
	vCSPortCANEnable(TRUE, FALSE);
}

void eCSRTUStop(void)
{
	vCSPortSerialEnable(FALSE, FALSE);
}

eCSErrorCode eCSRTUReceive(UCHAR * pucRcvCobId, UCHAR * pucRcvAddress, 
	UCHAR * *pucFrame, USHORT * pusLength)
{
	eCSErrorCode eStatus = CS_ENOERR;
	USHORT ident = CanspecFT.header.ident;

	if (((ident & 0xF00) >> 8) == (ident & 0x00F))
	{
		*pucRcvCobId = ident & 0xF0;
			
		/* Save the address field. All frames are passed to the upper layed
		 * and the decision if a frame is used is done there.
		 */
		*pucRcvAddress = ident & 0x00F;

		/* Total length of Modbus-PDU is Modbus-Serial-Line-PDU minus
		 * size of address field and CRC checksum.
		 */
		*pusLength = CanspecFT.header.length;

		/* Return the start of the Modbus PDU to the caller. */
		*pucFrame = (UCHAR *) & CanspecFT.data[CS_SER_PDU_PDU_OFF];
	}
	else
	{
		eStatus = CS_EIO;
	}

	return eStatus;
}

eCSErrorCode eCSRTUSend(UCHAR ucCobId, UCHAR pucAddress)
{
	eCSErrorCode eStatus = CS_ENOERR;

	/* Check if the receiver is still in idle state. If not we where to
	 * slow with processing the received frame and the master sent another
	 * frame on the network. We have to abort sending the frame.
	 */
	if (eRcvState == STATE_RX_IDLE)
	{
		/* Activate the transmitter. */
		CanspecFT.header.ident = ((USHORT)pucAddress << 8) + pucAddress + ucCobId + 0x10;
		eSndState = STATE_TX_XMIT;
		vCSPortCANEnable(FALSE, TRUE);
	}
	else
	{
		eStatus = CS_EIO;
	}

	return eStatus;
}


void xCSRTUReceiveFSM(void)
{
	xCSPortEventPost(EV_FRAME_RECEIVED);
	eRcvState = STATE_RX_IDLE;
}


void xCSRTUTransmitFSM(void)
{
	USHORT IncLen = 0;
	UCHAR maxLen = (CanspecFT.fdf == 0x01 ? CANFD_MAXLEN : CAN_MAXLEN);

	switch (eSndState)
	{
	case STATE_TX_XMIT:
		if (csSndOffsetCount >= SendFramePointer.Length[csSndAddrCount])
		{
			csSndOffsetCount = 0;
			csSndAddrCount++;
		}
		if (csSndAddrCount < SendFramePointer.Count)
		{
			IncLen = SendFramePointer.Length[csSndAddrCount] - csSndOffsetCount;
			if (IncLen > maxLen)
				IncLen = maxLen;

			csSndOffsetCount += IncLen;
			xCSPortTxStart(SendFramePointer.Addr[csSndAddrCount] + csSndOffsetCount - IncLen, IncLen);
		}
		else
		{
			csSndAddrCount = 0;
			csSndOffsetCount = 0;
			CanspecFT.mode = 0;
//			xCSPortEventPost(EV_FRAME_SENT);
			/* Disable transmitter. This prevents another transmit buffer
			 * empty interrupt. */
			vCSPortCANEnable(TRUE, FALSE);
			eSndState = STATE_TX_IDLE;
		}
		break;
		/* We should not get a transmitter event if the transmitter is in idle state.  */
	default: // case STATE_TX_IDLE:
		/* enable receiver/disable transmitter. */
		csSndAddrCount = 0;
		csSndOffsetCount = 0;
		CanspecFT.mode = 0;
		vCSPortCANEnable(TRUE, FALSE);
		break;
	}
}

#endif // #if COMM_CAN_ENABLED > 0