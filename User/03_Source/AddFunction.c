/*
 * Copyright (c) 2021-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : AddFunction.c
 * Author        : Summer
 * Date          : 2021-11-13
 * Description   : This file defines some basic function.
 *
 * Record        :
 * V1.0, 2021-11-13, Summer: Created file
 */

/********************************************************************************
* Header Definition
********************************************************************************/
#include <Myproject.h>


/********************************************************************************
* Macro & Structure Definition
*******************************************************************************/

#if TEST_SPIDEBUG_SW_ENABLED
unsigned short spidebug[4] = { 0 };		//< SPI debug 输出通道缓存，SPI调试器会将该变量值进行输出
#endif


uint16 TestVariable_1 = 0;
uint16 TestVariable_2 = 0;
uint16 TestVariable_3 = 0;
uint32 TestVariable_u32_1 = 0;
uint32 TestVariable_u32_2 = 0;
uint32 TestVariable_u32_3 = 0;


/********************************************************************************
* Internal Routine Prototypes
********************************************************************************/

/*---------------------------------------------------------------------------
 * Name		:	Test_Buff
 * Input	:	No
 * Output	:	No
 * Description:	将相关变量放在测试通道里，方便通过示波器来排查和调试代码的bug，
 *				也可以在代码里给单个测试通道幅值
 *---------------------------------------------------------------------------*/
void Test_Buff(void)
{
	usSRegInBuf[TESTVAR0] = TestVariable_1;
	usSRegInBuf[TESTVAR1] = TestVariable_2;
	usSRegInBuf[TESTVAR2] = TestVariable_3;
	usSRegInBuf[TESTVAR3_H] = TestVariable_u32_1 >> 16;
	usSRegInBuf[TESTVAR3_L] = TestVariable_u32_1;
	usSRegInBuf[TESTVAR4_H] = TestVariable_u32_2 >> 16;
	usSRegInBuf[TESTVAR4_L] = TestVariable_u32_2;
	usSRegInBuf[TESTVAR5_H] = TestVariable_u32_3 >> 16;
	usSRegInBuf[TESTVAR5_L] = TestVariable_u32_3;
}


/*---------------------------------------------------------------------------
 * Name          : IWDT_Init
 * Input         : None
 * Description   : 该函数为IDWT初始化操作函数, 用户仅需向该函数传入独立看门狗
 * 重载值即可, 独立看门狗的使能、重载值装载由该函数完成。
 * 要修改IWDT_KR 的值，必须先向IWDT_KR 寄存器中写入0x5555
 *---------------------------------------------------------------------------*/
void IWDT_Init(void)
{
	write_csr(IWDT_KR, IWDT_KRCHANGE);		//先写入0x5555，才能修改IWDT_KR寄存器的值
	write_csr(IWDT_KR, IWDT_OPEN);			//开启独立看门狗
	
	write_csr(IWDT_KR, IWDT_KRCHANGE);
	write_csr(IWDT_REL, IWDT_RELOAD);		//写入重载值
	
	write_csr(IWDT_KR, IWDT_KRCHANGE);
	write_csr(IWDT_KR, IWDT_FEED);			//开启写保护
}

/*---------------------------------------------------------------------------
 * Name			 : IWDT_Refresh
 * Input         : None
 * Description   : 该函数为IDWT喂狗操作函数,  独立看门狗的喂狗动作由该函数完成
 * 如果在计数器计数结束前，软件没有向键寄存器写入复位指令或关闭指令，则系统会产生独立看门狗复位。
 *---------------------------------------------------------------------------*/
void IWDT_Refresh(void)
{
	//独立看门狗喂狗
	write_csr(IWDT_KR, IWDT_KRCHANGE);
	write_csr(IWDT_KR, IWDT_FEED);
}

/*---------------------------------------------------------------------------
 * Name          : SPIDebugSet
 * Input         : None
 * Output        : None
 * Description   : 初始化SPI debug，支持硬件SPI debug(指定硬件寄存器地址即可)，
 *				    和软件SPI debug(将软件值放在spidebug变量中)，通过SPI debug
 *					工具输出相应信号，可通过电子示波器来查看。
 *---------------------------------------------------------------------------*/
void SPIDebugSet(void)
{
#if TEST_SPIDEBUG_HW_ENABLED || TEST_SPIDEBUG_SW_ENABLED
//	SPI_Init();
#endif
	write_csr(SPIDBG_DMA_LEN, 7);
	write_csr(SPIDBG_DMA_CR, DMAEN | DMA_MON_Send);

#if TEST_SPIDEBUG_HW_ENABLED
	write_csr(SPIDBG_DMA_BA, HARD_SPIDATA);
#elif TEST_SPIDEBUG_SW_ENABLED
	write_csr(SPIDBG_DMA_BA, spidebug);
#endif
	// 00：PA9复用为DataMonitor；01 : PA0复用为DataMonitor；1x : PB0复用为DataMonitor
	clr_csr(PH_SEL, MONCT1);  //DataMonitor管脚使能1  
	clr_csr(PH_SEL, MONCT0);  //DataMonitor管脚使能0

	set_csr(SPIDBG_DMA_CR, DMABSY);
	set_csr(SPIDBG_DMA_CR, DMACONEN);
}
	
/*---------------------------------------------------------------------------
 * Name          : SignalDebugSet
 * Input         : None
 * Output        : None
 * Description   : 设置相关引脚输出FOC的测试信号，或者CMPx_OUT的采样结果
 *---------------------------------------------------------------------------*/
void SignalDebugSet(void)
{
	/*Observation signal Select*/
	reset_csr(CMP_DBR, DBG1SEL0 | DBG1SEL1 | DBG1SEL2 | DBG1SEL3 | DBG1SEL4, GPA1_DBG_Conf);
	reset_csr(CMP_DBR, DBG2SEL0 | DBG2SEL1 | DBG2SEL2 | DBG2SEL3 | DBG2SEL4, GPB0_DBG_Conf);

	clr_csr(PH_SEL, CMPCT);		// 比较器信号输出引脚选择	0：PA1	1：PA3
	set_csr(PH_SEL, DBGCT);		// DBG信号输出引脚选择	0：PA1	1：PA9
	reset_csr(CMP_DBR, CMPSEL0 | CMPSEL1 | CMPSEL2 | CMPSEL3 | CMPSEL4, GPAx_DBG_Conf);
}

/*---------------------------------------------------------------------------
 * Name     :   void Motor_Charge(void)
 * Input    :   NO
 * Output   :   NO
 * Description: 预充电，当一直处于预充电状态下，不接电机，可用于验证IPM或者Mos。
 * 预充电分三步，第一步是对U相进行预充电，第二步是对U,V两相进行预充电，第三步是对U、V、W三相进行预充电。
 *---------------------------------------------------------------------------*/
void Motor_Charge(void)
{
	if (McStaSet.SetFlag.ChargeSetFlag == 0)
	{
		McStaSet.SetFlag.ChargeSetFlag = 1;
		set_csr(DRV1_CR, DRVEN);			 /* 计数器使能... */
		write_csr(DRV1_DR, (unsigned short)((PWM_VALUE_LOAD) >> 1)); /* 下桥臂50% duty... */
		write_csr(DRV1_CMR, 0x0000);     /* 预充电配置与FOC正常配置不一致... */
		set_csr(DRV1_OUT, MOE);			 /* 打开MOE... */
		set_csr(DRV1_CR, DRVOE);			// Driver输出使能	0-->Disable		1-->Enable
		mcFocCtrl.ChargeStep = 0;
		mcFocCtrl.State_Count = CHARGE_TIME;
		mcFocCtrl.ChargeFinish = 0;
	}

	if ((mcFocCtrl.State_Count < CHARGE_TIME) && (mcFocCtrl.ChargeStep == 0))
	{
		mcFocCtrl.ChargeStep = 1;
		reset_csr(DRV1_CMR, ULE, ULE);  /* U相下桥臂通... */

		set_csr(DRV1_OUT, MOE);
		set_csr(DRV1_CR, DRVOE);		// Driver输出使能	0-->Disable		1-->Enable
	}
	if ((mcFocCtrl.State_Count <= (CHARGE_TIME << 1) / 3) && (mcFocCtrl.ChargeStep == 1))
	{
		mcFocCtrl.ChargeStep = 2;
		reset_csr(DRV1_CMR, VLE, VLE); /* V相下桥臂通... */
	}
	if ((mcFocCtrl.State_Count <= CHARGE_TIME / 3) && (mcFocCtrl.ChargeStep == 2))
	{
		mcFocCtrl.ChargeStep = 3;
		reset_csr(DRV1_CMR, WLE, WLE); /* W相下桥臂通... */
	}

	if ((mcFocCtrl.State_Count == 0) && (mcFocCtrl.ChargeStep == 3))
	{
		mcFocCtrl.ChargeFinish = 1;
	}

}