/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canrtu.h
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec send and receive data.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */


#ifndef _CAN_RTU_H
#define _CAN_RTU_H

#include "canspec.h"

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif
	
void eCSRTUInit(USHORT baudrate);
void eCSRTUStart(void);
void eCSRTUStop(void);
eCSErrorCode eCSRTUReceive(UCHAR * pucRcvCobId, UCHAR * pucRcvAddress, UCHAR * *pucFrame, USHORT * pusLength);
eCSErrorCode eCSRTUSend(UCHAR ucCobId, UCHAR pucAddress);
void xCSRTUReceiveFSM(void);
void xCSRTUTransmitFSM(void);


#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
