/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canport.c
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec port definition.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */


/* ----------------------- CANspec includes ----------------------------------*/
#include "canspec.h"
#include "canport.h"
#include "CAN.h"
#include "Parameter.h"

#if COMM_CAN_ENABLED > 0 || EXCTRL_CANOPEN_ENABLED > 0
CANspecTypeDef CanspecFT;
#endif

#if COMM_CAN_ENABLED > 0

/* ----------------------- Defines ------------------------------------------*/
PortCANTypeDef PortCAN;

/* ----------------------- static functions ---------------------------------*/


/* ----------------------- Start implementation -----------------------------*/
/*---------------------------------------------------------------------------*/
/* Name		:	void CANSpecInit(void)
/* Input	:	nodeID, baudrate
/* Output	:	NO
/* Description:	Initial CANSpec
/*---------------------------------------------------------------------------*/
void xCSPortCANInit(USHORT baudrate)
{  
	memset(&CanspecFT, 0, sizeof(CANspecTypeDef));
	CAN_Setting(baudrate);
}

void vCSPortCANStart(void)
{
	CAN_Enable(); 
}

void vCSPortCANClose(void)
{ 
	CAN_Disable();
}


void vCSPortCANEnable(BOOL xRxEnable, BOOL xTxEnable)
{
	PortCAN.RxEnable = xRxEnable;
	PortCAN.TxEnable = xTxEnable;
}

void xCSPortTxStart(UCHAR* ulAddr, UCHAR cLen)
{
	CanspecFT.header.length = cLen;
	CanspecFT.mail = CAN_Send(&CanspecFT.header, ulAddr);
}

#endif // #if COMM_CAN_ENABLED > 0