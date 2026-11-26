/*
 * Copyright (c) 2023-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : InertiaFrictionIdentify.h
 * Author        : wynn.wang
 * Date          : 2023-07-26
 * Description   : 惯量识别
 *
 * Record        :
 * V1.0, 2023-07-26, wynn.wang: Created file
 */


/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __INERTIAFRICTIONIDENTIFY_H_
#define __INERTIAFRICTIONIDENTIFY_H_


#include "fu7502.h"
#include "Math.h"
#include "Customer.h"


typedef struct
{
	uint8  EstCtrl;
	uint16 SpeedStart;
	uint16 SpeedDelta;
	uint16 SpeedHigh;
	uint16 SpeedMin;
	uint16 Cur0_1;
	uint16 MaxLenSamp;
	uint32 Coff_J;
	const int16* pActVel;
	const int16* pCmdCur;

} InFricIdCfgTypedef;


/*---------------------------------------------------------------------------
 * Name		:	InFricId_Init
 * Input	:	cfg - 配置参数
 * Output	:	No
 * Description:	初始化惯量辨识
 *---------------------------------------------------------------------------*/
extern void InFricId_Init(InFricIdCfgTypedef* cfg);


/*---------------------------------------------------------------------------
 * Name		:	InFricId_realize
 * Input	:	*pJm - 惯量辨识结果
 * Output	:	惯量辨识的状态
 * Description:	执行惯量辨识，放在位置环中断里
 *---------------------------------------------------------------------------*/
extern uint8 InFricId_realize(uint32* pJm);

#endif

