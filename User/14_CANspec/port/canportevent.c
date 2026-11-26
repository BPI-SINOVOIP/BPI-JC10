/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : canportevent.c
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec handle receive event.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

/* ----------------------- Modbus includes ----------------------------------*/
#include "canspec.h"
#include "canport.h"
#include "Parameter.h"

#if COMM_CAN_ENABLED > 0

/* ----------------------- Variables ----------------------------------------*/

static eCSEventType eCSQueuedEvent;
static BOOL     xCSEventInQueue;

/* ----------------------- Start implementation -----------------------------*/
void xCSPortEventInit( void )
{
    xCSEventInQueue = FALSE;
}

void xCSPortEventPost( eCSEventType eEvent )
{
    eCSQueuedEvent = eEvent;
    xCSEventInQueue = TRUE;
}

BOOL xCSPortEventGet( eCSEventType * eEvent )
{
    BOOL xEventHappened = FALSE;

    if(xCSEventInQueue)
    { 
    	xCSEventInQueue = FALSE;
        xEventHappened = TRUE;
        *eEvent = eCSQueuedEvent;
    }
    return xEventHappened;

}
#endif // #if COMM_CAN_ENABLED > 0