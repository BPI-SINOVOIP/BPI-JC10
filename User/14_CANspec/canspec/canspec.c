/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : CANspec.c
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec main function.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

#include <Myproject.h>
/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- CANspec includes ----------------------------------*/

#include "canspec.h"
#include "canfunc.h"

#include "canport.h"

#if CS_SLAVE_SDO_ENABLED > 0
#include "canrtu.h"
#endif

#include "Parameter.h"

#if COMM_CAN_ENABLED > 0
/* ----------------------- Static variables ---------------------------------*/

static UCHAR ucCSAddress;

static enum
{
	STATE_NOT_INITIALIZED,
	STATE_ENABLED,
	STATE_DISABLED,
} eCSState = STATE_NOT_INITIALIZED;



/* An array of Modbus functions handlers which associates Modbus function
 * codes with implementing functions.
 */
static const xCSFunctionHandler xCSFuncHandlers[CS_FUNC_HANDLERS_MAX] = {
#if CS_FUNC_READ_HOLDING_ENABLED > 0
	{ CS_FUNC_READ_HOLDING_REGISTER, eCSFuncReadHoldingRegister },
#endif
#if CS_FUNC_WRITE_HOLDING_ENABLED > 0
	{ CS_FUNC_WRITE_REGISTER, eCSFuncWriteHoldingRegister },
#endif
#if CS_FUNC_READ_SCOPE_ENABLED > 0
	{ CS_FUNC_READ_SCOPE_REGISTER, eCSFuncReadScopeRegister },
#endif
};

/* ----------------------- Start implementation -----------------------------*/
eCSErrorCode eCSInit(USHORT baudrate)
{
	eCSErrorCode eStatus = CS_ENOERR;

	eCSRTUInit(baudrate);
		
	xCSPortEventInit();
		
	eCSState = STATE_DISABLED;

	return eStatus;
}



eCSErrorCode eCSEnable(void)
{
	eCSErrorCode eStatus = CS_ENOERR;

	if (eCSState == STATE_DISABLED)
	{
		/* Activate the protocol stack. */
		eCSRTUStart();
		eCSState = STATE_ENABLED;
	}
	else
	{
		eStatus = CS_EILLSTATE;
	}
	return eStatus;
}

eCSErrorCode eCSDisable(void)
{
	eCSErrorCode eStatus;

	if (eCSState == STATE_ENABLED)
	{
		eCSRTUStop();
		eCSState = STATE_DISABLED;
		eStatus = CS_ENOERR;
	}
	else if (eCSState == STATE_DISABLED)
	{
		eStatus = CS_ENOERR;
	}
	else
	{
		eStatus = CS_EILLSTATE;
	}
	return eStatus;
}

eCSErrorCode eCSPoll(void)
{
	static UCHAR * ucCSFrame;
	static UCHAR ucRcvAddress;
	static UCHAR ucRcvCobId;
	static UCHAR ucFunctionCode;
	static USHORT usLength;
	static eCSException eException;

	int i;
	eCSErrorCode eStatus = CS_ENOERR;
	eCSEventType eEvent;

	/* Check if the protocol stack is ready. */
	if (eCSState != STATE_ENABLED)
	{
		return CS_EILLSTATE;
	}
	
	/* Check if SlaveAddress is legal. */
	ucCSAddress = usSRegHoldBuf[COMMADDR];
	if ((ucCSAddress != CS_ADDRESS_BROADCAST) &&
		(ucCSAddress < CS_ADDRESS_MIN) || (ucCSAddress > CS_ADDRESS_MAX))
	{
		ucCSAddress = CS_ADDRESS_BROADCAST;
//		return CS_EINVAL;
	}

	/* Check if there is a event available. If not return control to caller.
	 * Otherwise we will handle the event. */
	if (xCSPortEventGet(&eEvent) == TRUE)
	{
		switch (eEvent)
		{
		case CS_FRAME_RECEIVED:
			// Handle received frame
			eStatus = eCSRTUReceive(&ucRcvCobId, &ucRcvAddress, &ucCSFrame, &usLength);
			/* Check if the frame is for us. If not ignore the frame. */
			if ((ucRcvAddress == ucCSAddress) || (ucRcvAddress == CS_ADDRESS_BROADCAST))
			{
				if (eStatus == CS_ENOERR)
				{
#if CS_SLAVE_SDO_ENABLED > 0
					if (ucRcvCobId == CS_CODEID_SDO)
					{
						ucFunctionCode = ucCSFrame[CS_PDU_FUNC_OFF];
						eException = CS_EX_ILLEGAL_FUNCTION;
						for (i = 0; i < CS_FUNC_HANDLERS_MAX; i++)
						{
							if (xCSFuncHandlers[i].ucFunctionCode == ucFunctionCode)
							{
								eException = xCSFuncHandlers[i].pxHandler(ucCSFrame, &usLength);
								break;
							}
						}
					}
					else 
#endif // #if CS_SLAVE_SDO_ENABLED > 0
#if CS_SLAVE_PDO_ENABLED > 0
					if (ucRcvCobId == CS_CODEID_PDO)
					{
						eException = eCSFuncControlFrame(ucCSFrame, &usLength);
					}
					else
#endif // #if CS_SLAVE_PDO_ENABLED > 0
					{
						eException = CS_EX_ERROR_CODID;
					}
				}
				else
				{
					eException = CS_EX_ERROR_NODEID;
				}
			
				/* If the request was not sent to the broadcast address we
				 * return a reply. */
				//			if (ucRcvAddress != CS_ADDRESS_BROADCAST)
				{
					if (eException != CS_EX_NONE)
					{
						/* An exception occured. Build an error frame. */
						usLength = 2;
						ucCSFrame[0] = (UCHAR)(ucFunctionCode | CS_FUNC_ERROR);
						ucCSFrame[1] = eException;

						// set SendFramePointer
						SendFramePointer.Count = 1;
						SendFramePointer.Length[0] = 2;
						SendFramePointer.Addr[0] = &ucCSFrame[0];
					}
					eStatus = eCSRTUSend(ucRcvCobId, ucRcvAddress);
					if (eStatus == CS_ENOERR) //no error
					{
						CanspecFT.mode = 1;
						xCSRTUTransmitFSM();
					}
				}
			}
			break;

		default:
			break;
		}
	}
	return CS_ENOERR;
}
#endif // #if COMM_CAN_ENABLED > 0