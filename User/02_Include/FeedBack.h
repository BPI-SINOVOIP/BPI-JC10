/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : Feedback.h
 * Author        : wynn.wang
 * Date          : 2025-01-16
 * Description   : Full closed loop control Function
 *
 * Record        :
 * V1.0, 2025-01-16, wynn.wang: Created file
 */


/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __FEEDBACK_H__
#define __FEEDBACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fu7502.h"
#include "Math.h"
#include "Filter.h"


// OnLoadFlag
#define FEEDBACK_MOTOR				( 0 )	// 电机反馈
#define FEEDBACK_LOAD				( 1 )   // 负载反馈


// 按照外环的分辨率低于内环的分辨率确定的Q格式
// 否则Q格式的位数要修改
#define LOADCOEF                    ( 20 )
#define MOTORCOEF                   ( 10 )


/* ------ FeedBack Config Type ------ */
typedef struct
{
	const uint8* pMode;                   // 全闭环模式  0:内环 1:外环 2:内外环切换（功能码）
	const uint8* pPosLoopEnable;		  // 位置环使能标志位	
	const int16* pMixErrClrRpm;           // 混合偏差清0圈数（功能码）
	const uint32* pLoadFeedPulse;         // 电机旋转一圈外部编码器的脉冲数（功能码）
	const int32* pMixErrMaxValue;         // 内、外环偏差的差值超过该值时应该报警（功能码）
	const int32* pLoadActualAngle;        // 外环实时位置
	const int32* pMotorActualAngle;		  // 内环实时位置
	const int32* pTargetAngle;
	const int32* pTargetRef;

	int32 LoadEncRes;				// 外环分辨率
	int32 EncRes;					// 内环分辨率
	int16 PosCtrlFreq;				// 位置环控制频率
	int16 LPFFreq;					// 外环抑振滤波器

} FeedBackCfgTypedef;


/*************************************************************************************///External Function

/*---------------------------------------------------------------------------
 * Name		:	FeedBackOnLoad_Init
 * Input	:	cfg - 配置参数
 * Output	:	No
 * Description:	初始化全闭环控制
 *---------------------------------------------------------------------------*/
extern void FeedBackOnLoad_Init(const FeedBackCfgTypedef * cfg);


/*---------------------------------------------------------------------------
 * Name		:	FeedBackOnLoad_Clr
 * Input	:	No
 * Output	:	No
 * Description:	复位全闭环控制
 *---------------------------------------------------------------------------*/
extern void FeedBackOnLoad_Clr(void);


/*---------------------------------------------------------------------------
 * Name		:	FeedBackOnLoad_Update
 * Input	:	No
 * Output	:	No
 * Description:	全闭环控制变量实时更新
 *---------------------------------------------------------------------------*/
extern void FeedBackOnLoad_Update();


/*---------------------------------------------------------------------------
 * Name     :   PosErrCalc_realize
 * Input    :   No
 * Output   :   位置偏差
 * Description: 计算混合偏差，无论是否开启全闭环功能，都使用这个函数计算位置偏差
 *---------------------------------------------------------------------------*/
extern int32 PosErrCalc_realize();


/*---------------------------------------------------------------------------
 * Name     :   FeedBackMixErrCheck
 * Input    :   No
 * Output   :   混合偏差，该值仅作为故障保护使用，不作为闭环。
 * Description: 混合偏差监控，无论是否开启全闭环功能，都使用这个函数计算位置偏差
 *---------------------------------------------------------------------------*/
extern int32 FeedBackMixErrCheck();


/*---------------------------------------------------------------------------
 * Name     :   LoadPosFdbCalc_realize
 * Input    :   NO
 * Output   :   外环速度
 * Description: 计算外环位置反馈增量
 *---------------------------------------------------------------------------*/
extern int32 LoadPosFdbCalc_realize();


/*---------------------------------------------------------------------------
 * Name     :   Feedback_GetOnLoadFlag
 * Input    :   No
 * Output   :   工作环路标志位：1:工作在外环  0:工作在内环。
 * Description: 获取当前工作环路的标志位
 *---------------------------------------------------------------------------*/
extern uint8 Feedback_GetOnLoadFlag(void);


/*---------------------------------------------------------------------------
 * Name     :   Feedback_GetMixErrOverFlag
 * Input    :   No
 * Output   :   混合偏差过大报警标志位
 * Description: 获取混合偏差过大报警标志位
 *---------------------------------------------------------------------------*/
extern uint8 Feedback_GetMixErrOverFlag(void);


/*---------------------------------------------------------------------------
 * Name     :   Feedback_GetLoadToMotorCoef
 * Input    :   No
 * Output   :   外环向内环的转换系数 
 * Description: 获取外环向内环的转换系数
 *---------------------------------------------------------------------------*/
extern int64 Feedback_GetLoadToMotorCoef(void);


/*---------------------------------------------------------------------------
 * Name     :   Feedback_GetMotorToLoadCoef
 * Input    :   No
 * Output   :   内环向外环的转换系数
 * Description: 获取内环向外环的转换系数
 *---------------------------------------------------------------------------*/
extern int64 Feedback_GetMotorToLoadCoef(void);

#ifdef __cplusplus
}
#endif

#endif