/*
 * Copyright (c) 2023-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : DeadTimeCompensation.h
 * Author        : ThinkPad
 * Date          : 2023-6-4
 * Description   : This file contains Dead Time Compensation.
 *
 * Record        :
 * V1.0, 2023-6-4, Summer: Created file
 */

#ifndef __DEADTIMECOMPENSATION_H__
#define __DEADTIMECOMPENSATION_H__

#include "fu7502.h"
#include "Customer.h"



/*---------------------------------------------------------------------------
 * Name		:	DeadTimeCompensationInit
 * Input	:	DeadCompCurr - 启动死区补偿的电流阈值
 * Output	:	No
 * Description:	初始化死区补偿
 *---------------------------------------------------------------------------*/
extern void DeadTimeCompensationInit(int16 DeadCompCurr);


/*---------------------------------------------------------------------------
 * Name		:	DeadTimeCompensationController
 * Input	:	Theta - 当前电角度值
 *				Id - 当前D轴电流
 * 				Iq - 当前Q轴电流
 * Output	:	No
 * Description:	计算死区补偿的值
 *---------------------------------------------------------------------------*/
extern void DeadTimeCompensationController(int16 Theta, int16 Id, int16 Iq);

#endif


