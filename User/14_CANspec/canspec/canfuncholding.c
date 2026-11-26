/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canfuncholding.c
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec read and write register.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "parameter.h"
#include "Customer.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "canspec.h"

#include "user_mb_app.h"
#include "Scope.h"

/* ----------------------- Defines ------------------------------------------*/
#define CS_PDU_FUNC_READ_ADDR_OFF               ( CS_PDU_FUNC_OFF + 1 )
#define CS_PDU_FUNC_READ_REGCNT_OFF             ( CS_PDU_FUNC_OFF + 3 )
#define CS_PDU_FUNC_READ_SIZE                   ( 4 )
#define CS_PDU_FUNC_READ_REGCNT_MAX				( 30 )


#define CS_PDU_FUNC_WRITE_ADDR_OFF				( CS_PDU_FUNC_OFF + 1 )
#define CS_PDU_FUNC_WRITE_REGCNT_OFF			( CS_PDU_FUNC_OFF + 3 )
#define CS_PDU_FUNC_WRITE_VALUES_OFF			( CS_PDU_FUNC_OFF + 4 )
#define CS_PDU_FUNC_WRITE_SIZE_MIN				( 4 )
#define CS_PDU_FUNC_WRITE_REGCNT_MAX			( 30 )



/* ----------------------- Start implementation -----------------------------*/

#if CS_FUNC_WRITE_HOLDING_ENABLED > 0

eCSException eCSFuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
{
    eCSException    eStatus = CS_EX_NONE;
    USHORT usAddressH, usAddressL;
    USHORT iRegIndex;
    USHORT usData, usRegCount, i;
    UCHAR * pucFrameCur;

    if (*usLen >= (CS_PDU_FUNC_WRITE_SIZE_MIN + CS_PDU_SIZE_MIN))
    {
#if (CANSPEC_ENDIAN == CANSPEC_BIG_ENDIAN)
    	usAddressH = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF];
    	usAddressL = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF + 1];
#else
    	usAddressH = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF + 1];
    	usAddressL = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF];
#endif
    	
        if (usAddressH < S_REG_HOLDING_INDEX_MAX)
        {
        	iRegIndex = (USHORT)(usSRegHoldStartArray[usAddressH] + usAddressL);
        	if (iRegIndex + 1 <= usSRegHoldStartArray[usAddressH + 1])
        	{
        		SendFramePointer.Count = 1;
        		SendFramePointer.Length[0] = CS_PDU_FUNC_WRITE_SIZE_MIN;
        		SendFramePointer.Addr[0] = pucFrame;

        		usRegCount = (USHORT) pucFrame[CS_PDU_FUNC_WRITE_REGCNT_OFF];

        		if ((usRegCount >= 1) &&
        			(usRegCount <= CS_PDU_FUNC_WRITE_REGCNT_MAX))
        		{
        			pucFrame[CS_PDU_FUNC_WRITE_REGCNT_OFF] = usRegCount << 1;
        			*usLen = CS_PDU_FUNC_WRITE_SIZE_MIN;
        			i = CS_PDU_FUNC_WRITE_VALUES_OFF;
        			while (usRegCount-- > 0)
        			{
#if (CANSPEC_ENDIAN == CANSPEC_BIG_ENDIAN)
        				usData = (USHORT) pucFrame[i++] << 8;
        				usData |= pucFrame[i++];
#else
        				usData = pucFrame[i++];
        				usData |= (USHORT) pucFrame[i++] << 8;
#endif

        				UpdateCSRegHoldBuf(usData, iRegIndex++);
        			}	
        		}
        		else
        		{
        			eStatus = CS_EX_ILLEGAL_DATA_LENGTH;
        		}
        	}
        	else
        	{
        		eStatus = CS_EX_ILLEGAL_DATA_ADDRESS;
        	}
        }
        else
        {
        	eStatus = CS_EX_ILLEGAL_DATA_ADDRESS;
        }
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = CS_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}


/*---------------------------------------------------------------------------*/
/* Name     :   void UpdateRegHoldBuf(USHORT usData, USHORT iIndex)
/* Input    :   usData
/* Output   :   void
/* Description: 个别参数需要先缓存到影子寄存器，然后再更新到参数表
/*---------------------------------------------------------------------------*/
void UpdateCSRegHoldBuf(USHORT usData, USHORT iIndex)
{
	static USHORT flagL = 0;
	static USHORT usDataL = 0;
	static USHORT iIndexL = 0;
	
	if (flagL == 0)
	{
		if (iIndex == POSLIMPOS_L || iIndex == POSLIMNEG_L)
		{
			flagL = 1;
			usDataL = usData;
			iIndexL = iIndex;
		}
		else
		{
			usSRegHoldBuf[iIndex] = usData;
		}
	}
	else
	{
		flagL = 0;
		usSRegHoldBuf[iIndex] = usData; // if (iIndex == iIndexL + 1)
		usSRegHoldBuf[iIndexL] = usDataL;
	}
}
#endif // #if CS_FUNC_WRITE_HOLDING_ENABLED > 0


#if CS_FUNC_READ_HOLDING_ENABLED > 0

eCSException eCSFuncReadHoldingRegister(UCHAR * pucFrame, USHORT * usLen)
{
	USHORT usAddressH, usAddressL;
	USHORT usRegCount;
	USHORT iRegIndex;
	UCHAR * pucFrameCur;
	UCHAR usIndexMax = S_REG_HOLDING_INDEX_MAX;
	USHORT * pusStartArray = usSRegHoldStartArray;
	USHORT * pusRegBuf = usSRegHoldBuf;

	eCSException eStatus = CS_EX_NONE;

	if (*usLen == CS_PDU_FUNC_READ_SIZE)
	{
#if (CANSPEC_ENDIAN == CANSPEC_BIG_ENDIAN)
		usAddressH = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF];
		usAddressL = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF + 1];
#else
		usAddressH = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF + 1];
		usAddressL = pucFrame[CS_PDU_FUNC_WRITE_ADDR_OFF];
#endif

		usRegCount = (USHORT)pucFrame[CS_PDU_FUNC_READ_REGCNT_OFF];


		/* Check if the number of registers to read is valid. If not
		 * return Modbus illegal data length exception.
		 */
		if ((usRegCount >= 1) && (usRegCount <= CS_PDU_FUNC_READ_REGCNT_MAX))
		{
			/* Second byte in the response contain the number of bytes. */
			pucFrameCur = &pucFrame[CS_PDU_FUNC_READ_REGCNT_OFF];
			*pucFrameCur++ = (UCHAR)(usRegCount << 1);
			*usLen = CS_PDU_FUNC_READ_SIZE + (usRegCount << 1);

			if (usAddressH & 0x80) // input register
			{
				usAddressH &= 0x7F;
				usIndexMax = S_REG_INPUT_INDEX_MAX;
				pusStartArray = usSRegInStartArray;
				pusRegBuf = usSRegInBuf;
			}
			if (usAddressH < usIndexMax)
			{
				iRegIndex = (USHORT)(*(pusStartArray + usAddressH) + usAddressL);
				if (iRegIndex + usRegCount <= *(pusStartArray + usAddressH + 1))
				{
					SendFramePointer.Count = 1;
					SendFramePointer.Length[0] = *usLen;
					SendFramePointer.Addr[0] = pucFrame;

					while (usRegCount > 0)
					{
#if (CANSPEC_ENDIAN == CANSPEC_BIG_ENDIAN)
						*pucFrameCur++ = (UCHAR)(*(pusRegBuf + iRegIndex) >> 8);
						*pucFrameCur++ = (UCHAR)(*(pusRegBuf + iRegIndex) & 0xFF);
#else
						*pucFrameCur++ = (UCHAR)(*(pusRegBuf + iRegIndex) & 0xFF);
						*pucFrameCur++ = (UCHAR)(*(pusRegBuf + iRegIndex) >> 8);
#endif
						iRegIndex++;
						usRegCount--;
					}
				}
				else
				{
					eStatus = CS_EX_ILLEGAL_DATA_ADDRESS;
				}
			}
			else
			{
				eStatus = CS_EX_ILLEGAL_DATA_ADDRESS;
			}
		}
		else
		{
			eStatus = CS_EX_ILLEGAL_DATA_LENGTH;
		}
	}
	else
	{
		/* Can't be a valid request because the length is incorrect. */
		eStatus = CS_EX_ILLEGAL_DATA_VALUE;
	}
	return eStatus;
}
#endif // #if CS_FUNC_READ_HOLDING_ENABLED > 0