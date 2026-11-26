/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canfuncscopes.c
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec read scope data.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "Parameter.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "canspec.h"
#include "user_mb_app.h"

/* ----------------------- Other includes ----------------------------------*/
#include "Scope.h"

/* ----------------------- Defines ------------------------------------------*/
#define CS_PDU_FUNC_SCOPE_ADDR_OFF           ( CS_PDU_FUNC_OFF + 1)
#define CS_PDU_FUNC_SCOPE_REGCNT_OFF         ( CS_PDU_FUNC_OFF + 3 )
#define CS_PDU_FUNC_SCOPE_SIZE               ( 5 )
#define CS_PDU_FUNC_SCOPE_REGCNT_MAX		 ( S_REG_SCOPE_NREGS )


#if CS_FUNC_READ_SCOPE_ENABLED > 0

eCSException eCSFuncReadScopeRegister(UCHAR * pucFrame, USHORT * usLen)
{
	USHORT usRegAddress;
	USHORT usRegCount;
	USHORT usRegLen;
	UCHAR * pucFrameCur;
	USHORT usTemp;

	eCSException eStatus = CS_EX_NONE;
	eCSErrorCode eRegStatus;

	if (*usLen == CS_PDU_FUNC_SCOPE_SIZE)
	{
		usRegAddress = (USHORT)(pucFrame[CS_PDU_FUNC_SCOPE_ADDR_OFF] << 8); 
		usRegAddress |= (USHORT)(pucFrame[CS_PDU_FUNC_SCOPE_ADDR_OFF + 1]);

		usRegCount = (USHORT)(pucFrame[CS_PDU_FUNC_SCOPE_REGCNT_OFF] << 8);
		usRegCount |= (USHORT)(pucFrame[CS_PDU_FUNC_SCOPE_REGCNT_OFF + 1]);

		/* Check if the number of registers to read is valid. If not
		 * return Modbus illegal data length exception.
		 */
		if ((usRegCount >= 1) && (usRegCount <= CS_PDU_FUNC_SCOPE_REGCNT_MAX))
		{
			/* First byte contains the function code. */
			pucFrameCur = &pucFrame[CS_PDU_FUNC_SCOPE_REGCNT_OFF];
			*usLen = 3;

			/* Second word in the response contain the number of bytes. */
			if (Scope_GetDataReadyFlag(usRegAddress))
			{
				usRegLen = usRegCount << 1; 
				*pucFrameCur++ = (UCHAR)(usRegLen >> 8);
				*pucFrameCur++ = (UCHAR)(usRegLen & 0xFF);
				*usLen += 2;
				
				
				if ((usRegAddress >= 0) && (usRegAddress <= CS_PDU_FUNC_SCOPE_REGCNT_MAX))
				{
					if (usRegAddress + usRegCount <= CS_PDU_FUNC_SCOPE_REGCNT_MAX)
					{
						SendFramePointer.Count = 2;
						SendFramePointer.Length[0] = 5; // Header
						SendFramePointer.Length[1] = usRegCount << 1; // Data

						SendFramePointer.Addr[0] = pucFrame;
						SendFramePointer.Addr[1] = &usSRegScopeBuf[usRegAddress];
					}
					else
					{
						SendFramePointer.Count = 3;
						SendFramePointer.Length[0] = 5; // Header
						SendFramePointer.Length[1] = (S_REG_SCOPE_NREGS - usRegAddress) << 1; // Data1
						SendFramePointer.Length[2] = (usRegAddress + usRegCount - S_REG_SCOPE_NREGS) << 1; // Data2

						SendFramePointer.Addr[0] = pucFrame;
						SendFramePointer.Addr[1] = &usSRegScopeBuf[usRegAddress];
						SendFramePointer.Addr[2] = &usSRegScopeBuf[0];
					}
				}
				else
				{
					eStatus = CS_EX_ILLEGAL_DATA_ADDRESS;
				}
			}
			else
			{
				usRegLen = 2;
				*pucFrameCur++ = (UCHAR)(usRegLen >> 8);
				*pucFrameCur++ = (UCHAR)(usRegLen & 0xFF);
				*usLen += 2;
				
				usTemp = usSRegInBuf[SCOPESTATUS];
				*pucFrameCur++ = (UCHAR)(usTemp >> 8);
				*pucFrameCur++ = (UCHAR)(usTemp & 0xFF);
				*usLen += 2;
				
				
				if ((usRegAddress >= 0) && (usRegAddress <= S_REG_SCOPE_NREGS))
				{
					SendFramePointer.Count = 1;
					SendFramePointer.Length[0] = 7;
					SendFramePointer.Addr[0] = pucFrame;
				}
				else
				{
					eStatus = CS_EX_ILLEGAL_DATA_ADDRESS;
				}
			}	
		}
		else
		{
			eStatus = CS_EX_ILLEGAL_DATA_LENGTH;
		}
	}
	else
	{
		/* Can't be a valid read input register request because the length
		 * is incorrect. */
		eStatus = CS_EX_ILLEGAL_DATA_VALUE;
	}
	return eStatus;
}

#endif // #if CS_FUNC_READ_SCOPE_ENABLED > 0