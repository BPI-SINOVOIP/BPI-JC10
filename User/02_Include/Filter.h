/*
 * Copyright (c) 2023-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : Filter.h
 * Author        : wynn.wang
 * Date          : 2023-08-11
 * Description   : 通用滤波器
 *
 * Record        :
 * V1.0, 2023-08-11, wynn.wang: Created file
 */


/* Define to prevent recursive inclusion -------------------------------------------------------- */
#ifndef __FILTER_H__
#define __FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fu7502.h"
#include "math.h"


/*************************************************************************************///External Function
// 1st low pass filter
#define INLPF_Q_SHIFT						(15)
#define pi_Q15								(102944)


// 分配的内存大小(字节)
#define LPFILT_MEM_SIZE						( 44 )  // LowPassFiltTypedef内存大小
#define POSLPFILT_MEM_SIZE					( 36 )  // PosLowPassFiltTypedef内存大小
#define AVGFILT_MEM_SIZE					( 526 ) // AvgFiltTypedef内存大小



typedef struct LowPassFiltTypedef_t LowPassFiltTypedef;
typedef struct PosLowPassFiltTypedef_t PosLowPassFiltTypedef;
typedef struct AvgFiltTypedef_t AvgFiltTypedef;


/*---------------------------------------------------------------------------
 * Name		:	LowPassFilt_Init
 * Input	:	memory_buffer - 外部预先分配的LowPassFiltTypedef大小内存
 *				freq - 滤波器截止频率
 *				sampFreq - 滤波器更新频率
 * Output	:	指向已配置完成的 LowPassFiltTypedef 实例的指针
 * Description:	初始化低通滤波器: y(n) = b0 * x(n) + b1 * x(n-1) - a1 * y(n-1)
 *---------------------------------------------------------------------------*/
extern LowPassFiltTypedef* LowPassFilt_Init(void* memory_buffer, int32 freq, int32 sampFreq);


/*---------------------------------------------------------------------------
 * Name		:	LowPassFilt_realize
 * Input	:	p - LowPassFilt_Init 成功返回的低通滤波器实例指针
 *				Input - 滤波器输入值x(n)
 * Output	:	滤波器输出值y(n)
 * Description: 计算下一个时刻的滤波器输出值，固定时间间隔调用该函数	
 *				y(n) = b0 * x(n) + b1 * x(n-1) - a1 * y(n-1)
 *---------------------------------------------------------------------------*/
extern int32 LowPassFilt_realize(LowPassFiltTypedef* p, int32 Input);


/*---------------------------------------------------------------------------
 * Name		:	LowPassFilt_Clr
 * Input	:	p - LowPassFilt_Init 成功返回的低通滤波器实例指针
 * Output	:	无
 * Description: 复位滤波器
 *---------------------------------------------------------------------------*/
extern void LowPassFilt_Clr(LowPassFiltTypedef* p);


/*---------------------------------------------------------------------------
 * Name		:	PosLowPassFilt_Init
 * Input	:	memory_buffer - 外部预先分配的PosLowPassFiltTypedef大小内存
 *				freq - 滤波器截止频率
 *				sampFreq - 滤波器更新频率
 * Output	:	指向已配置完成的 PosLowPassFiltTypedef 实例的指针
 * Description:	初始化低通滤波器.
 *---------------------------------------------------------------------------*/
extern PosLowPassFiltTypedef* PosLowPassFilt_Init(void* memory_buffer, int32 freq, int32 sampFreq);


/*---------------------------------------------------------------------------
 * Name		:	PosLowPassFilt_realize
 * Input	:	p - PosLowPassFilt_Init 成功返回的低通滤波器实例指针
 *				Input - 滤波器输入值x(n)
 * Output	:	滤波器输出值y(n)
 * Description: 计算下一个时刻的滤波器输出值，固定时间间隔调用该函数
 *				PosLowPassFilt会比LowPassFilt的计算时间更长一些
 *				y(n) = ( x(n) - y(n-1) ) / K + y(n-1)
 *---------------------------------------------------------------------------*/
extern int32 PosLowPassFilt_realize(PosLowPassFiltTypedef* p, int32 Input);


/*---------------------------------------------------------------------------
 * Name		:	PosLowPassFilt_Clr
 * Input	:	p - PosLowPassFilt_Init 成功返回的低通滤波器实例指针
 * Output	:	无
 * Description: 复位滤波器
 *---------------------------------------------------------------------------*/
extern void PosLowPassFilt_Clr(PosLowPassFiltTypedef* p);


/*---------------------------------------------------------------------------
 * Name		:	AvgFilt_Init
 * Input	:	memory_buffer - 外部预先分配的AvgFiltTypedef大小内存
 *				filtSize - 滤波器的深度为(1 << filtSize)
 * Output	:	指向已配置完成的 AvgFiltTypedef 实例的指针
 * Description:	初始化滑动平均滤波器.
 *---------------------------------------------------------------------------*/
extern AvgFiltTypedef* AvgFilt_Init(void* memory_buffer, uint16 filtSize);


/*---------------------------------------------------------------------------
 * Name		:	AvgFilt_realize
 * Input	:	p - AvgFilt_Init 成功返回的低通滤波器实例指针
 *				Input - 滤波器输入值x(n)
 * Output	:	滤波器输出值y(n)
 * Description: 计算下一个时刻的滤波器输出值，固定时间间隔调用该函数
 *				y(n) = [x(n)+x(n−1)+x(n−2)+...+x(n−N+1)] / N
 *---------------------------------------------------------------------------*/
extern int16 AvgFilt_realize(AvgFiltTypedef* p, int16 Input);


/*---------------------------------------------------------------------------
 * Name		:	AvgFilt_Clr
 * Input	:	p - AvgFilt_Init 成功返回的平滑滤波器实例指针
 * Output	:	无
 * Description: 复位滤波器
 *---------------------------------------------------------------------------*/
extern void AvgFilt_Clr(AvgFiltTypedef* p);

#ifdef __cplusplus
}
#endif

#endif
