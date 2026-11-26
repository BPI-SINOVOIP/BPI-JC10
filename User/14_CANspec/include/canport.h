/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canport.h
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec port definition.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

#ifndef _CAN_PORT_H
#define _CAN_PORT_H

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif
             
#include "CAN.h"

	/* ----------------------- Defines ------------------------------------------*/
#ifndef BOOL
#define BOOL                                      unsigned char
#endif

#ifndef UCHAR
#define UCHAR                                     unsigned char
#endif

#ifndef CHAR
#define CHAR									  char
#endif

#ifndef USHORT
#define USHORT									  unsigned short
#endif

#ifndef SHORT
#define SHORT									  short
#endif

#ifndef ULONG
#define ULONG									  unsigned long
#endif

#ifndef LONG
#define LONG									  long
#endif

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif



	/* ----------------------- Type definitions ---------------------------------*/

typedef enum
{
	CS_READY = 1 << 0,         /*!< Startup finished. */
	CS_FRAME_RECEIVED = 1 << 1,         /*!< Frame received. */
	CS_EXECUTE = 1 << 2,         /*!< Execute function. */
	CS_FRAME_SENT = 1 << 3          /*!< Frame sent. */
} eCSEventType;

/* Transmit/Receive message object */
typedef struct {
	CAN_TxRxHeaderTypeDef header;
	UCHAR fdf; // 1:CANFD; 0:CAN
	UCHAR mode; // 1:CANopen; 0:CANspec
	UCHAR mail; // send mail box number
	UCHAR data[64];
} CANspecTypeDef;

typedef struct
{
	BOOL RxEnable;
	BOOL TxEnable;
} PortCANTypeDef;


extern PortCANTypeDef PortCAN;

extern CANspecTypeDef CanspecFT;

/* ----------------------- Supporting functions -----------------------------*/
extern void xCSPortEventInit(void);

extern void xCSPortEventPost(eCSEventType eEvent);

extern BOOL xCSPortEventGet( eCSEventType* eEvent);


/* ----------------------- Serial port functions ----------------------------*/

extern void xCSPortCANInit(USHORT baudrate);

extern void vCSPortCANStart(void);

extern void vCSPortCANClose(void);

extern void vCSPortCANEnable(BOOL xRxEnable, BOOL xTxEnable);

/* ----------------------- Callback for the protocol stack ------------------*/

extern void xCSPortTxStart(UCHAR* ulAddr, UCHAR cLen);


#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
