/*
 * Copyright (c) 2026-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : MIT.h
 * Author        : Gabriel
 * Date          : 2026-03-10
 * Description   : MIT 控制.
 *
 * Record        :
 * V1.0, 2026-03-10, Summer: Created file
 */



/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __MIT_H__
#define __MIT_H__

#ifdef __cplusplus
extern "C"{
#endif
/*--------------------Includes----------------------------------------------------*/	
#include <Myproject.h>

#define MIT_MODE_OFF			( 0 )
#define MIT_MODE_CUR			( 1 )
#define MIT_MODE_VEL			( 2 )
#define MIT_MODE_POS			( 3 )


typedef struct
{
	int16 VelLimMax;
	int16 VelLimMin;
	int16 CurLimMax;
	int16 CurLimMin;

	uint16 ForceConst;
	uint16 CurCoe;
	
	uint16 CurCtrlFreq;
	uint16 VelCtrlFreq;
	uint16 PosCtrlFreq;

	const int32* pTargetReftoSpd;
	const int16* pActVel;
	const int16* pActCur;
	
	const int16* pWorkMode;
	const uint8* pProfileFlag;
	
	const uint16* pMKp;
	const uint16* pMKd;
	
	const int16* pSpdCmd;
	const int16* pCurCmd;

} MITCfgTypedef;

/*---------------------------------------------------------------------------*/
/* Name     :MIT_Init   
/* Input    :   NO
/* Output   :   NO
/* Description: 参数初始化
/*---------------------------------------------------------------------------*/
extern void MIT_Init(MITCfgTypedef* cfg);

/*---------------------------------------------------------------------------*/
/* Name     :MIT_CmdParaUpdate
/* Input    :   NO
/* Output   :   NO
/* Description: MIT指令参数更新
/*---------------------------------------------------------------------------*/
extern void MIT_CmdParaUpdate(void);

/*---------------------------------------------------------------------------*/
/* Name     :MIT_ModeUpdate
/* Input    :   NO
/* Output   :   NO
/* Description: MIT工作模式更新
/*---------------------------------------------------------------------------*/
extern void MIT_ModeUpdate(void);

/*---------------------------------------------------------------------------*/
/* Name     :MIT_CurComp
/* Input    :   NO
/* Output   :   NO
/* Description: MIT位置/速度模式电流补偿
/*---------------------------------------------------------------------------*/
extern void MIT_CurComp(void);

/*---------------------------------------------------------------------------*/
/* Name     :MIT_Clr
/* Input    :   NO
/* Output   :   NO
/* Description: 动态参数和滤波器清零
/*---------------------------------------------------------------------------*/
extern void MIT_Clr(void);

/*---------------------------------------------------------------------------*/
/* Name     :MIT_Pos_realize
/* Input    :   NO
/* Output   :
/* Description: 位置环调度
/*---------------------------------------------------------------------------*/
extern void MIT_Pos_realize(int32 ErrValue);   

/*---------------------------------------------------------------------------*/
/* Name     :MIT_Spd_realize
/* Input    :   NO
/* Output   :
/* Description: 速度环调度
/*---------------------------------------------------------------------------*/
extern int16 MIT_Spd_realize(void);

/*---------------------------------------------------------------------------*/
/* Name     : MIT_TrefCalc  
/* Input    :   NO
/* Output   :
/* Description: 电流环调度
/*---------------------------------------------------------------------------*/
extern int16 MIT_TrefCalc(void);

/*---------------------------------------------------------------------------
 * Name		:	MIT_GetDynamicWorkMode
 * Input	:	No
 * Output	:	No
 * Description:	获取当前的MIT工作模式
 *---------------------------------------------------------------------------*/
extern uint8 MIT_GetDynamicWorkMode(void);

#ifdef __cplusplus
}
#endif


#endif