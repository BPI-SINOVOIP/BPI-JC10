/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canfunccontrol.c
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec control frame handling.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

/* ----------------------- System includes ----------------------------------*/
#include "stdlib.h"
#include "string.h"

/* ----------------------- Platform includes --------------------------------*/
#include "parameter.h"
#include <Myproject.h>

/* ----------------------- Modbus includes ----------------------------------*/
#include "canspec.h"

#include "user_mb_app.h"


/* ----------------------- Defines ------------------------------------------*/
#define CS_PDU_FUNC_CTRL_REV_SIZE				( 8 )
#define CS_PDU_FUNC_CTRL_SEND_SIZE				( 16 )

// Receive data
#define CS_PDU_FUNC_CTRL_CONTROLWORD_OFF		( 0 )
#define CS_PDU_FUNC_CTRL_TARGETPOS_OFF			( 2 )

// Send data
#define CS_PDU_FUNC_CTRL_STATUSWORD_OFF			( 0 )
#define CS_PDU_FUNC_CTRL_ACTPOS_OFF				( 2 )
#define CS_PDU_FUNC_CTRL_ACTCUR_OFF				( 6 )
#define CS_PDU_FUNC_CTRL_ACTTEMP_OFF			( 8 )
#define CS_PDU_FUNC_CTRL_ERRCODE_OFF			( 10 )
#define CS_PDU_FUNC_CTRL_REV1_OFF				( 12 )


// control word definition
#define CONTROLWORD_BIT_ENABLE					( 0x0001 )/**< \brief Enable command*/
#define CONTROLWORD_BIT_DISABLE					( 0x0002 )/**< \brief Disable command*/
#define CONTROLWORD_BIT_CLEAR					( 0x0004 )/**< \brief Clear error command*/
#define CONTROLWORD_BIT_HALT					( 0x4000 )/**< \brief 1: Halt*/
#define CONTROLWORD_BIT_MODE					( 0x8000 )/**< \brief 1: CSP; 0: PP*/



/* ----------------------- Start implementation -----------------------------*/

#if CS_FUNC_CONTROL_FRAME_ENABLED > 0

eCSException eCSFuncControlFrame( UCHAR * pucFrame, USHORT * usLen )
{
    eCSException    eStatus = CS_EX_NONE;

    if (*usLen == CS_PDU_FUNC_CTRL_REV_SIZE)
    {
		SendFramePointer.Count = 1;
		SendFramePointer.Length[0] = CS_PDU_FUNC_CTRL_SEND_SIZE;
		SendFramePointer.Addr[0] = pucFrame;
	
#if (CANSPEC_ENDIAN == CANSPEC_BIG_ENDIAN)
		// Receive
		usSRegHoldBuf[DRIVECTRL] = (((USHORT) pucFrame[CS_PDU_FUNC_CTRL_CONTROLWORD_OFF]) << 8) | \
			((USHORT) pucFrame[CS_PDU_FUNC_CTRL_CONTROLWORD_OFF + 1]);

		CiA402_TargetPosition = (((ULONG) pucFrame[CS_PDU_FUNC_CTRL_TARGETPOS_OFF]) << 24) | \
			(((ULONG) pucFrame[CS_PDU_FUNC_CTRL_TARGETPOS_OFF + 1]) << 16) | \
			(((ULONG) pucFrame[CS_PDU_FUNC_CTRL_TARGETPOS_OFF + 2]) << 8) | \
			(((ULONG) pucFrame[CS_PDU_FUNC_CTRL_TARGETPOS_OFF + 3]));
		
		// Send
		pucFrame[CS_PDU_FUNC_CTRL_STATUSWORD_OFF] = usSRegInBuf[DRIVESTATUS] >> 8;
		pucFrame[CS_PDU_FUNC_CTRL_STATUSWORD_OFF + 1] = usSRegInBuf[DRIVESTATUS] & 0x03;

		pucFrame[CS_PDU_FUNC_CTRL_ACTPOS_OFF] = usSRegInBuf[ACTPOS_H] >> 8;
		pucFrame[CS_PDU_FUNC_CTRL_ACTPOS_OFF + 1] = usSRegInBuf[ACTPOS_H];
		pucFrame[CS_PDU_FUNC_CTRL_ACTPOS_OFF + 2] = usSRegInBuf[ACTPOS_L] >> 8;
		pucFrame[CS_PDU_FUNC_CTRL_ACTPOS_OFF + 3] = usSRegInBuf[ACTPOS_L];

		pucFrame[CS_PDU_FUNC_CTRL_ACTCUR_OFF] = usSRegInBuf[ACTCUR] >> 8;
		pucFrame[CS_PDU_FUNC_CTRL_ACTCUR_OFF + 1] = usSRegInBuf[ACTCUR];

		pucFrame[CS_PDU_FUNC_CTRL_ACTTEMP_OFF] = usSRegInBuf[DRIVETEMP] >> 8;
		pucFrame[CS_PDU_FUNC_CTRL_ACTTEMP_OFF + 1] = usSRegInBuf[DRIVETEMP];
		
		pucFrame[CS_PDU_FUNC_CTRL_ERRCODE_OFF] = usSRegInBuf[ERRORSTATUS0] >> 8;
		pucFrame[CS_PDU_FUNC_CTRL_ERRCODE_OFF + 1] = usSRegInBuf[ERRORSTATUS0];
#else
		// Receive
		usSRegInBuf[CANCONTROLWORD] = *((USHORT *) & pucFrame[CS_PDU_FUNC_CTRL_CONTROLWORD_OFF]);

		CiA402_TargetPosition = *((ULONG *) &pucFrame[CS_PDU_FUNC_CTRL_TARGETPOS_OFF]);

		// Send
		*((USHORT *) & pucFrame[CS_PDU_FUNC_CTRL_STATUSWORD_OFF]) = usSRegInBuf[DRIVESTATUS] & 0x03;

		*((ULONG *) & pucFrame[CS_PDU_FUNC_CTRL_ACTPOS_OFF]) = *((ULONG *) & usSRegInBuf[SFBACTPOS_L]);

		*((USHORT *) & pucFrame[CS_PDU_FUNC_CTRL_ACTCUR_OFF]) = usSRegInBuf[ACTCUR];

		*((USHORT *) & pucFrame[CS_PDU_FUNC_CTRL_ACTTEMP_OFF]) = usSRegInBuf[DRIVETEMP];

		*((USHORT *) & pucFrame[CS_PDU_FUNC_CTRL_ERRCODE_OFF]) = usSRegInBuf[ERRORSTATUS0];
		
		*((ULONG *) & pucFrame[CS_PDU_FUNC_CTRL_REV1_OFF]) = 0;
#endif
		
		*((ULONG *) & usSRegInBuf[PCMDFBRAW_L]) = CiA402_TargetPosition;
		
    }
    else
    {
        /* Can't be a valid request because the length is incorrect. */
        eStatus = CS_EX_ILLEGAL_DATA_VALUE;
    }
    return eStatus;
}


/*---------------------------------------------------------------------------*/
/* Name		:	void CANSpec_cycle_process(void)
/* Input	:	NO
/* Output	:	NO
/* Description:	Process control of CAN，每1ms调用一次
/*---------------------------------------------------------------------------*/
void CANSpec_cycle_process()
{
	static uint8 CtrlWordLatch = 0;
	int32 targetPos;
	
	if (mcRegParam.WorkMode == COMM_ECAT)
	{
		if (mcRegParam.WorkMode_Pre != COMM_ECAT)
		{
			usSRegInBuf[ECCOMMSTATE] = 0x05;
		}
		usSRegInBuf[FBSYNCACT] = usSRegHoldBuf[FBSYNCSET];
		
		if (GetReg(usSRegInBuf[CANCONTROLWORD], CONTROLWORD_BIT_ENABLE) == 0) // 去使能
		{
			SetBit(usSRegHoldBuf[DRIVECTRL], CTRL_DISABLE);
			ClrBit(usSRegHoldBuf[DRIVECTRL], CTRL_ENABLE);
		}
		else if (GetReg(usSRegInBuf[CANCONTROLWORD], CONTROLWORD_BIT_ENABLE) && 
			GetReg(CtrlWordLatch, CONTROLWORD_BIT_ENABLE) == 0) // 上升沿：上使能
		{
			ClrBit(usSRegHoldBuf[DRIVECTRL], CTRL_DISABLE);
			SetBit(usSRegHoldBuf[DRIVECTRL], CTRL_ENABLE);
		}
		else
		{
			ClrBit(usSRegHoldBuf[DRIVECTRL], CTRL_DISABLE);
			ClrBit(usSRegHoldBuf[DRIVECTRL], CTRL_ENABLE);
		}

		SetReg(usSRegHoldBuf[DRIVECTRL], CTRL_CLEARERR, usSRegInBuf[CANCONTROLWORD] & 0x04);
		if (GetReg(usSRegInBuf[CANCONTROLWORD], CONTROLWORD_BIT_MODE))
		{
			usSRegInBuf[FBOPMODE] = CYCLIC_SYNC_POSITION_MODE;
		}
		else
		{
			usSRegInBuf[FBOPMODE] = INTERPOLATION_POSITION_MODE;
		}

		if (usSRegInBuf[FBOPMODE] == INTERPOLATION_POSITION_MODE) // PP模式
		{
			if (GetReg(usSRegInBuf[DRIVESTATUS], STATUS_ENABLE))
			{
				if (GetReg(usSRegInBuf[CANCONTROLWORD], CONTROLWORD_BIT_HALT)) // 暂停/减速
				{
					DynTrap_Demo_Update(CiA402_TargetPosition, 0,
						*((uint32*) & usSRegHoldBuf[PROFILEMAXACC_L]));
				}
				else
				{
					DynTrap_Demo_Update(CiA402_TargetPosition, usSRegHoldBuf[PROFILEMAXVEL],
						*((uint32*) & usSRegHoldBuf[PROFILEMAXACC_L]));
				}
			}
			else
			{
#if FUNC_FEEDBACKONLOAD_ENABLED
				if (FEEDBACK_LOAD == Feedback_GetOnLoadFlag())
				{
					targetPos = ((int32)usSRegInBuf[SFBACTPOS_H] << 16) | (uint16)usSRegInBuf[SFBACTPOS_L];
				}
				else
#endif
				{
					targetPos = ((int32)usSRegInBuf[ACTPOS_H] << 16) | (uint16)usSRegInBuf[ACTPOS_L];
				}
				
				DynTrap_Demo_Init();
				DynTrap_Demo_Update(targetPos, usSRegHoldBuf[PROFILEMAXVEL],
					*((uint32*) & usSRegHoldBuf[PROFILEMAXACC_L]));
				
			}
		}

		
		CtrlWordLatch = usSRegInBuf[CANCONTROLWORD];
	}
	else
	{
		usSRegInBuf[ECCOMMSTATE] = 0xFFFF;
		usSRegInBuf[FBOPMODE] = NO_MODE;
	}
}

#endif // #if CS_FUNC_CONTROL_FRAME_ENABLED > 0