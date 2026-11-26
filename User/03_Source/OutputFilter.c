/*
 * Copyright (c) 2023-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : OutputFilter.c
 * Author        : Summer
 * Date          : 2023-08-23
 * Description   : 转矩滤波器(硬件化)
 *
 * Record        :
 * V1.0, 2023-08-23, Summer: Created file
 */


/********************************************************************************
 * Header Definition
 ********************************************************************************/
#include "MyProject.h"


/*---------------------------------------------------------------------------
 * Name		:	OutputFilter_Init
 * Input	:	No
 * Output	:	No
 * Description:	转矩滤波器初始化
 *---------------------------------------------------------------------------*/
void OutputFilter_Init(void)
{
	if (!(usSRegHoldBuf[OUTFILTCTRL] & OUTCTRL_FILTMASK4))
	{
		clr_csr(DRV1_FCR3, IQRF4_EN);
		if (!(usSRegHoldBuf[OUTFILTCTRL] & OUTCTRL_FILTMASK3))
		{
			clr_csr(DRV1_FCR3, IQRF3_EN);
			if (!(usSRegHoldBuf[OUTFILTCTRL] & OUTCTRL_FILTMASK2))
			{
				clr_csr(DRV1_FCR3, IQRF2_EN);
				set_csr(DRV1_FCR3, IQRF1_EN); // 至少需要开一个滤波器
			}
			else
			{
				set_csr(DRV1_FCR3, IQRF2_EN);
				set_csr(DRV1_FCR3, IQRF1_EN);
			}
		}
		else
		{
			set_csr(DRV1_FCR3, IQRF3_EN);
			set_csr(DRV1_FCR3, IQRF2_EN);
			set_csr(DRV1_FCR3, IQRF1_EN);
		}
	}
	else
	{
		set_csr(DRV1_FCR3, IQRF4_EN); // 滤波器4开了其他3级都得开
		set_csr(DRV1_FCR3, IQRF3_EN);
		set_csr(DRV1_FCR3, IQRF2_EN);
		set_csr(DRV1_FCR3, IQRF1_EN);
	}
}

/*---------------------------------------------------------------------------
 * Name		:	OutputFilter_RunModeUpdate
 * Input	:	NO
 * Output	:	NO
 * Description:	设置转矩滤波器参数，电机使能或者模式切换时调用
 *				Y(n) = B0*X(n) + B1*X(n-1) + B2*X(n-2) - A1*Y(n-1) - A2*Y(n-2)
 *				系数B0\B1\B2\A1\A2的值由上位机计算，电流环和速度环取不同的系数
 *---------------------------------------------------------------------------*/
void OutputFilter_RunModeUpdate(void)
{
	int16 i = 0, j = 0, k = 0, filtStartIndex = 0, qFacSel = 0, reg = 0;
	
#if FUNC_SOFTCTRL_ENABLED
	if ((mcFocCtrl.RunMod == CURMOD) ||
		((mcFocCtrl.RunMod == POS_CURMOD) && (FieldSoftControl_GetSwToPosFlag() != 1)))
#elif FUNC_MIT_ENABLED    // 使能时该函数不运行，在MIT_ModeUpdate中切换状态
    if (mcFocCtrl.RunMod == CURMOD || mcFocCtrl.RunMod == POS_SPD_CURMOD)
#else
	if (mcFocCtrl.RunMod == CURMOD) // 电流环
#endif
	{
		filtStartIndex = OUT2FILT1B0_L;
		qFacSel = (usSRegHoldBuf[OUTFILTQFAC] << 4) & 0x0F00;
		clr_csr(DRV1_FCR3, IQRF_SEL); // 0-电流环计算；1-速度环计算
	}
	else
	{
		filtStartIndex = OUT1FILT1B0_L;
		qFacSel = (usSRegHoldBuf[OUTFILTQFAC] << 8) & 0x0F00;
		set_csr(DRV1_FCR3, IQRF_SEL); // 0-电流环计算；1-速度环计算
	}

	// 系数选择Q29 or Q20
	reset_csr(DRV1_FCR3, QFAC_SEL4 | QFAC_SEL3 | QFAC_SEL2 | QFAC_SEL1, qFacSel); 

	// 设置滤波器系数B0\B1\B2\A1\A2
	for (j = 0; j < 10; j++)
	{
		*(&NFOC_F1LB0SH + j) = (int16)usSRegHoldBuf[filtStartIndex + j];
	}
	set_csr(DRV1_FCR5, IQRF_UPDATE);
	
	
	k = 0;
	for (i = 0; i < OUTPUTFILTER_FILTNUM; i++)
	{
		for (j = 0; j < 10; j++)
		{
			*(&NFOC_F1LB0 + k) = (int16)usSRegHoldBuf[filtStartIndex + i * 10 + j];
			k += 1;
		}
		k += 10;
	}
}

/*---------------------------------------------------------------------------
 * Name		:	OutputFilter_CoeUpdate
 * Input	:	No
 * Output	:	No
 * Description:	使能时滤波器1系数实时更新，方便调试；只有滤波器1支持，其他3个不支持
 *---------------------------------------------------------------------------*/
void OutputFilter_CoeUpdate(void)
{
	int16 filtStartIndex = 0, j = 0;
	uint8 bUpdate = 0;
	static uint8 bUpdatePre = 0;
	
	bUpdate = ReadBit(usSRegHoldBuf[OUTFILTQFAC], OUTFILT1UPDATE);
	
	if (bUpdate && !bUpdatePre) // 上升沿
	{
		if (mcFocCtrl.RunMod == CURMOD)
		{
			filtStartIndex = OUT2FILT1B0_L;
		}
		else
		{
			filtStartIndex = OUT1FILT1B0_L;
		}

		for (j = 0; j < 10; j++)
		{
			*(&NFOC_F1LB0SH + j) = (int16)usSRegHoldBuf[filtStartIndex + j];
		}

		set_csr(DRV1_FCR5, IQRF_UPDATE);
	}

	bUpdatePre = bUpdate;

}