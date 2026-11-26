/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canfunc.h
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec read and write function definition.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

#ifndef _CAN_FUNC_H
#define _CAN_FUNC_H

#include "canspec.h"

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif


#if CS_FUNC_READ_HOLDING_ENABLED > 0
eCSException    eCSFuncReadHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
#endif

#if CS_FUNC_WRITE_HOLDING_ENABLED > 0
eCSException    eCSFuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen );
void UpdateCSRegHoldBuf(USHORT usData, USHORT iIndex);
#endif

#if CS_FUNC_READ_SCOPE_ENABLED > 0
eCSException	eCSFuncReadScopeRegister(UCHAR * pucFrame, USHORT * usLen);
#endif

#if CS_FUNC_CONTROL_FRAME_ENABLED > 0
eCSException	eCSFuncControlFrame(UCHAR * pucFrame, USHORT * usLen);
void CANSpec_cycle_process();
#endif

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
