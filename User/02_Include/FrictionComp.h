/*
 * Copyright (c) 2024-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : FrictionComp.h
 * Author        : wynn.wang
 * Date          : 2024-08-01
 * Description   : 重力/摩擦力补偿
 *
 * Record        :
 * V1.0, 2024-08-01, wynn.wang: Created file
 */


/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __FRICTIONCOMP_H__
#define __FRICTIONCOMP_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "fu7502.h"
#include "Filter.h"
#include "Customer.h"

/*************************************************************************************///External Function

typedef struct
{
	int16 PosVelDeadBand;
	int16 NegVelDeadBand;
	int16 PosVelValue;
	int16 NegVelValue;
	int16 PosCmpIq;
	int16 NegCmpIq;

	uint8 FricMod;
	uint8 FricVSrc;
	uint16 FricLpfHz;

	uint16 MaxSpeed;
	uint16 CurFileFreq;

	const int32* pTargetReftoSpd;
	const int16* pCmdVel;
	const int16* pActVel;
	const uint8* pRunMod;
	
} FriCmpCfgTypedef;


/*---------------------------------------------------------------------------
 * Name		:	FricCmp_Init
 * Input	:	cfg - 配置参数
 * Output	:	No
 * Description:	初始化摩擦力补偿
 *---------------------------------------------------------------------------*/
extern void FricCmp_Init(FriCmpCfgTypedef* cfg);


/*---------------------------------------------------------------------------
 * Name		:	FricCmp_realize
 * Input	:	No
 * Output	:	摩擦力补偿电流
 * Description:	计算摩擦力补偿电流
 *---------------------------------------------------------------------------*/
extern int16 FricCmp_realize(void);


/*---------------------------------------------------------------------------
 * Name		:	FricCmp_Clr
 * Input	:	No
 * Output	:	No
 * Description:	复位摩擦力补偿
 *---------------------------------------------------------------------------*/
extern int16 FricCmp_Clr(void);

#ifdef __cplusplus
}
#endif

#endif