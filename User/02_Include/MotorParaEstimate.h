/*
 * Copyright (c) 2024-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : MotorParaEstimate.h
 * Author        : wynn.wang
 * Date          : 2024-12-02
 * Description   : 电机参数辨识
 *
 * Record        :
 * V1.0, 2024-12-02, wynn.wang: Created file
 */


/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __MOTORPARAESTIMATE_H_
#define __MOTORPARAESTIMATE_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "fu7502.h"
#include "Math.h"


typedef struct
{
	int16 Mode;
	int16 IAmp;
	uint16 RTime;
	uint16 LTime;
	int16 UMin;
	int16 UMax;
	int16 UDelta;
	int16 FreqMax; // 初始频率
	int16 FreqMin;
	int16 FreqDelta;
	int16 IMinRatio;
	int16 CVMode;
	uint16 CurFileFreq;

	uint8* pFocThetaFlag;
	int16* pStatus;
	int16* pIdRef;
	int16* pIqRef;

	int16* pId;
	int16* pUd;
	int16* pUq;
	
} MotorEstCfgTypeDef;



/*---------------------------------------------------------------------------
 * Name		:	Motor_Identify_ParamInit
 * Input	:	cfg - 配置参数
 * Output	:	No
 * Description:	初始化电机参数辨识
 *---------------------------------------------------------------------------*/
extern void Motor_Identify_ParamInit(MotorEstCfgTypeDef* cfg);


/*---------------------------------------------------------------------------
 * Name		:	Motor_Identify_realize
 * Input	:	No
 * Output	:	No
 * Description:	执行电机参数辨识，在1ms里运行
 *---------------------------------------------------------------------------*/
extern void Motor_Identify_realize(void);


/*---------------------------------------------------------------------------
 * Name		:	Motor_Resistance_Identify_OpenLoop_realize
 * Input	:	No
 * Output	:	No
 * Description:	电阻参数辨识（开环）
 *---------------------------------------------------------------------------*/
extern void Motor_Resistance_Identify_OpenLoop_realize(void);


/*---------------------------------------------------------------------------
 * Name		:	Motor_Resistance_Identify_ClosedLoop_realize
 * Input	:	No
 * Output	:	No
 * Description:	电阻参数辨识（闭环）
 *---------------------------------------------------------------------------*/
extern void Motor_Resistance_Identify_ClosedLoop_realize(void);


/*---------------------------------------------------------------------------
 * Name		:	Motor_Inductance_Identify_realize
 * Input	:	No
 * Output	:	No
 * Description:	电感参数辨识
 *---------------------------------------------------------------------------*/
extern void Motor_Inductance_Identify_realize(void);


/*---------------------------------------------------------------------------
 * Name		:	Motor_Identify_realize_Isr
 * Input	:	No
 * Output	:	No
 * Description:	执行电机参数辨识，放在载波中断里
 *---------------------------------------------------------------------------*/
extern void Motor_Identify_realize_Isr(void);


/*---------------------------------------------------------------------------
 * Name		:	Motor_Identify_GetFinishFlag
 * Input	:	No
 * Output	:	No
 * Description:	返回辨识完成标志位
 *---------------------------------------------------------------------------*/
extern uint8 Motor_Identify_GetFinishFlag(void);

#ifdef __cplusplus
}
#endif

#endif

