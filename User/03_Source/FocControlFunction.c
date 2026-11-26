/*
 * Copyright (c) 2022-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : FocControlFunction.c
 * Author        : Fortior Application Team
 * Date          : 2022-09-21
 * Description   : define motor contorl parameter
 *
 * Record        :
 * V1.0, 2022-09-21, Fortior Application Team: Created file
 */


/********************************************************************************
* Header Definition
********************************************************************************/
#include <Myproject.h>
#include "FocControlFunction.h"


/********************************************************************************
* Macro & Structure Definition
*******************************************************************************/
FOCCTRL				mcFocCtrl = { 0 };

RegParamTypeDef		mcRegParam __attribute__((section(".pram.data.mcRegParam"))) = { 0 };
AnInTypeDef			mcAnalogInput __attribute__((section(".pram.data.mcAnalogInput"))) = { 0 };

volatile uint8 TimerFlag_1ms = 0;
uint8 SYNC0_Flag = 0;
uint8 EscIntStep = 0;

/********************************************************************************
* Internal Routine Prototypes
********************************************************************************/

/*---------------------------------------------------------------------------
 * Name		:	FOC_Init
 * Input	:	No
 * Output	:	No
 * Description:	FOC初始化，对FOC的相关寄存器进行配置
 *---------------------------------------------------------------------------*/
void FOC_Init(void)
{
	uint16 reg;
	
	memset(NFOC_BASE + FOC_REG_CLEAR_OFFSET, 0, 1024 - FOC_REG_CLEAR_OFFSET); //FOC1相关寄存器清零
	
	write_csr(DRV1_FCR0, 0x00);		//FOC1相关寄存器清零
	write_csr(DRV1_FCR1, 0x00);		//FOC1相关寄存器清零
	
	NFOC_ARR = PWM_VALUE_LOAD; // FOC 载波频率， 须与 DRVx_ARR 的值保持一致

	set_csr(DRV1_FCR1, ANGMAMD);	//使能后NFOC_THETA不再来源于ETHETA
	
	set_csr(DRV1_FCR0, FDIS_MD2);	//跳过估算器到角度的计算，从电流环开始计算，用户填NFOC_THETA
	clr_csr(DRV1_FCR0, FDIS_MD1);
	clr_csr(DRV1_FCR0, FDIS_MD0);
	
	set_csr(DRV1_FCR5, LUT_EN); // 0:LUT, 1:Cordic
	clr_csr(DRV1_FCR5, DTC_EN); // 死区补偿使能, 0:不使能, 1:使能

	
	// 电角度跟ActPos方向
	if (mcFocCtrl.EAngDir)
	{
		set_csr(DRV1_FCR4, IQREF_INV); // IQREF取反
	}
	else
	{
		clr_csr(DRV1_FCR4, IQREF_INV); // IQREF不取反
	}
	

	/*外环环路选择*/
	clr_csr(DRV1_FCR1, WPIMD0 | WPIMD1);  //用户手动填写WFDB

    /*外环初始化*/
	NFOC_WKP = 0;
	NFOC_WKI = 0;
	NFOC_WMAX = usSRegHoldBuf[VOUTMAX];
	NFOC_WMIN = usSRegHoldBuf[VOUTMIN];
	
	// 外环计算周期选择(DRV_FCR1[wpi_auto]须配11)  00:每1个MMT计算周期 01:每2个MMT计算周期
	// 外环计算周期选择(DRV_FCR1[wpi_auto]须配11)  10:每4个MMT计算周期 11:每8个MMT计算周期	
#if DRV32K_ENABLE_SWITCH
	set_csr(DRV1_FCR4, WPI_CAL_CYCLE1);
	clr_csr(DRV1_FCR4, WPI_CAL_CYCLE0);
#else
	clr_csr(DRV1_FCR4, WPI_CAL_CYCLE1);
	set_csr(DRV1_FCR4, WPI_CAL_CYCLE0);
#endif	

	/*功率计算配置*/
#if (IT_POW_MD == DIS_IT_POW)
	{
		clr_csr(DRV1_FCR0, ITPOWMD0 | ITPOWMD1);
	}
#elif (IT_POW_MD == EX_IT_POW)
	{
		set_csr(DRV1_FCR0, ITPOWMD0);
		clr_csr(DRV1_FCR0, ITPOWMD1);
	}
#elif (IT_POW_MD == IN_IT_POW)
	{
		clr_csr(DRV1_FCR0, ITPOWMD0);
		set_csr(DRV1_FCR0, ITPOWMD1);
	}
#elif (IT_POW_MD == IS_IT_POW)
	{
		set_csr(DRV1_FCR0, ITPOWMD0);
		set_csr(DRV1_FCR0, ITPOWMD1);
	}	
#endif

	/*电流采样模式*/
#if (HW_ADC_RESISTOR_MODE == SINGLE_RESISTOR)
#if (Single_Resistor_ignoreEnable == 1)
	set_csr(DRV1_FCR0, 0x08);        // DRV1_FCR0(bit1-bit3): 电流采样模式配置                                     //旧单电阻忽略模式
#endif
	NFOC_TS = PWM_TS_LOAD;									//单电阻最小采样窗口
	NFOC_TDTC = 0;
	NFOC_NTS = 0;
	NFOC_TDLY = 0x0A;          // 提前采样设置0x0fff，滞后采样设置0x0005;

#elif (HW_ADC_RESISTOR_MODE == DOUBLE_RESISTOR)
	
#if DOUBLESAMP_ENABLE_SWITCH
	set_csr(DRV1_FCR0, NCSAMMD3);	// 双电阻两次采样(1001)，同时CT_SEL必须置一
	clr_csr(DRV1_FCR0, NCSAMMD2);
	clr_csr(DRV1_FCR0, NCSAMMD1);
	set_csr(DRV1_FCR0, NCSAMMD0);
	NFOC_CSAM = 0x09; // 采样模式初值
#else
	clr_csr(DRV1_FCR0, NCSAMMD3);	// 双电阻单次采样(0001)
	clr_csr(DRV1_FCR0, NCSAMMD2);
	clr_csr(DRV1_FCR0, NCSAMMD1);
	set_csr(DRV1_FCR0, NCSAMMD0);
	NFOC_CSAM = 0x01;
#endif
	
	set_csr(DRV1_FCR1, CT_SEL);      // 计算FOC时机 0:电流采样后下次上溢点计算FOC  1:电流采样后立刻计算FOC
	
#if (HW_ADC_SYNC == ADCSAM_SYNC)
	set_csr(ME_CR, TRIG_MD);			// 开启双ADC同步采样
#endif
	
	NFOC_TS = usSRegHoldBuf[MINPWMWIDTH]; //PWM_DLOWL_TIME;
	NFOC_TDTC = PWM_DT_LOAD;
//	NFOC_NTS = 0; // omitted
	NFOC_TDLY = ((uint32)usSRegHoldBuf[ISAMDELAY] * MCU_CLOCK) >> 10; // ISAM_DELAY;
	write_csr(DRV1_DTR, ((uint32)usSRegHoldBuf[DEADTIME] * MCU_CLOCK) >> 10); // PWM_LOAD_DEADTIME
	
	/*五段式或七段式选择*/
#if (HW_SVPWM_MODE == SVPWM_7_SEGMENT)
	{
		clr_csr(DRV1_FCR2, NSEG5);							// 7段式
	}
#elif (HW_SVPWM_MODE == SVPWM_5_SEGMENT)
	{
		set_csr(DRV1_FCR2, NSEG5);							// 5段式
	}
#endif

#elif (HW_ADC_RESISTOR_MODE == THREE_RESISTOR)
	set_csr(DRV1_FCR0, 0x04);                                                // 三电阻模式
#if (Three_Resistor_switcheEnable == 1)
	set_csr(DRV1_FCR0, 0x0C);                                            // 新三电阻模式
#endif
	NFOC_TS = usSRegHoldBuf[MINPWMWIDTH]; //PWM_DLOWL_TIME;
	NFOC_TDTC = PWM_DT_LOAD;
	NFOC_NTS = 0;
	NFOC_TDLY = 0x0006;          // 提前采样设置0x0805，滞后采样设置0x0005;

#elif (HW_ADC_RESISTOR_MODE == SINGLE_RESISTOR_NEW)
	set_csr(DRV1_FCR0, 0x06);                                                // 新单电阻模式
	NFOC_TS = 0;
	NFOC_TDTC = 0;
	NFOC_NTS = 30;              // 单电阻采样窗口
	NFOC_TDLY = 0x0F;            // 提前采样设置0x0805，滞后采样设置0x0005;待确定
#endif

	/*初始化Driver输出*/
	uint16 halfPWM = (PWM_VALUE_LOAD) >> 1;
	NFOC_DR1	= halfPWM;
	NFOC_DR1N	= halfPWM;
	NFOC_DR2	= halfPWM;
	NFOC_DR2N	= halfPWM;
	NFOC_DR3	= halfPWM;
	NFOC_DR3N	= halfPWM;

	write_csr(DRV1_CMPU1, halfPWM);
	write_csr(DRV1_CMPD1, halfPWM);
	write_csr(DRV1_CMPU2, halfPWM);
	write_csr(DRV1_CMPD2, halfPWM);
	write_csr(DRV1_CMPU3, halfPWM);
	write_csr(DRV1_CMPD3, halfPWM);
	
#if (OVMDL_EN == 1)
	set_csr(DRV1_FCR0, NOVMDL); // 过调制
#endif
	

	/*电流环复矢量解耦*/
	if (usSRegHoldBuf[CVMOD] == 0)
	{
		clr_csr(DRV1_FCR6, CVD_EN);                //复矢量解耦使能位
	}
	else
	{
		// 设置电流环复矢量控制器相关系数
		set_csr(DRV1_FCR6, CVD_EN);                
		// 若NFOC_UDCMAX = 0x7fff, NFOC_UDCMIN = 0x7fff，然后只给KP、KI的话就和原来的电流环效果一样
		
		NFOC_UDCMAX = usSRegHoldBuf[UDCMAX]; // UdcMax; //
		NFOC_UDCMIN = usSRegHoldBuf[UDCMIN]; // UdcMin; //
		NFOC_UDCLIM = usSRegHoldBuf[UDCMIN]; // UdcMin; // UDCLIM需要赋个初值
	}
	
	/*电流环PI使能*/
#if OPENLOOP_ENABLE_TEST
	if (mcState == mcStart)
	{
		set_csr(DRV1_FCR0, NVQDIS); // Q轴PI禁止使能
		set_csr(DRV1_FCR0, NVDDIS); // D轴PI禁止使能
		clr_csr(DRV1_FCR6, CVD_EN); // 复矢量解耦使能位
	}
	else
	{
		clr_csr(DRV1_FCR0, NVQDIS);		// Q轴PI使能
		clr_csr(DRV1_FCR0, NVDDIS);		// D轴PI使能
	}
#else
	clr_csr(DRV1_FCR0, NVQDIS);		// Q轴PI使能
	clr_csr(DRV1_FCR0, NVDDIS);		// D轴PI使能
#endif
	
	// 角度补偿，IPARK提前1.5拍，修复高速相位补偿，0.5载波补偿
	NFOC_PCOM = 0; // RV:0000H PARK角度固定补偿
	NFOC_PTCK = 0; // RV:0000H PARK补偿速度系数 Q13
	NFOC_IPTCK = 0; // RV:0000H IPARK补偿速度系数 Q13
	NFOC_PTHETA = 0; // PARK角度
	NFOC_IPTCPS = 0; // IPTCPS=OMEGA*FBASE*IPTCK

	
	/*电流环PI控制器输出阈值*/
	NFOC_DMAX = usSRegHoldBuf[DOUTMAX];
	NFOC_DMIN = usSRegHoldBuf[DOUTMIN];

	NFOC_QMAX = usSRegHoldBuf[QOUTMAX];
	NFOC_QMIN = usSRegHoldBuf[QOUTMIN];

	/*速度环控制器输出阈值*/
    NFOC_IQMAX = usSRegHoldBuf[VOUTMAX];
    NFOC_IQMIN = usSRegHoldBuf[VOUTMIN];
	
    /*滤波设置*/
	set_csr(DRV1_FCR5, IDQ_LPF_EN); // 使能ID/IQ滤波
	set_csr(DRV1_FCR5, IDQ_FB_SEL); // ID/IQ选择滤波后的值作为反馈值
	NFOC_IDK = usSRegHoldBuf[IDQLPFK]; // 9000Hz 51BE fs:32kHz
	NFOC_IQK = usSRegHoldBuf[IDQLPFK];
	
	set_csr(DRV1_FCR4, UDQ_LPF_EN);     // UDQ低通滤波器使能	0:不使能  1:使能
	NFOC_UDK = usSRegHoldBuf[UDQLPFK];  // 9000Hz 51BE fs:32kHz
	NFOC_UQK = usSRegHoldBuf[UDQLPFK];
	
	NFOC_UDCK = 0x1000; // 母线电压采样滤波系数
	
	/*速度计算MT法*/
	NFOC_MBASE = usSRegHoldBuf[QEPSPEEDCOE];
	NFOC_MSHFT = usSRegHoldBuf[SPEEDCOERANK];
	if (GetReg(usSRegHoldBuf[DRIVESWITCH], SW_MTMODE))
	{
		NFOC_MARR = VELCTRL_TS * 2;
	}
	else
	{
		NFOC_MARR = VELCTRL_TS;
	}
	
	/*设置DAC硬件过流值*/
#if (HardwareCurrent_Protect == Hardware_CMP_Protect || \
	HardwareCurrent_Protect == Hardware_FO_CMP_Protect)	//两者都选择
#if FUNC_EEPROM_ENABLED
		uint16 temp = usSRegHoldBuf[HARDCURRENT] >> 5;
	if (temp > 511)
		temp = 511;
	write_csr(DAC_DR0, temp);
#else
	write_csr(DAC_DR0, 511);// 暂时设为最大值，避免输入电流在电机使能后电压值跳动导致误触发
#endif
#endif
	
	/*电流基准校正*/
#if(HW_ADC_CALIB_MODE == CALIB_START)
	UpdateCurrentOffset();              // 电流偏置的获取
#endif
	
#if MOTOR_VCM_ENABLED
	/*音圈电机控制需设置ME中断*/
	if (usSRegHoldBuf[MOTORCOMMTYPE] == VOICECOIL_MOTOR)
	{
		write_csr(ME_ST0, 8);				// 在Clarke变换这一步产生中断
		set_csr(ME_IER, 0x03);				// 开启ME状态机中断
		write_csr(IP10, MEFSM_INT_PRI);		// ME状态机中断(中断号10) 优先级别为6
	}
#endif

#if FUNC_FIELDWEAKEN_ENABLED
#if MCU_VERSION_7504DA
	set_csr(DRV1_FCR8, FWIQRF_EN); // 开启弱磁时的iqref滤波器1使能
#else
	/*弱磁控制需设置ME中断*/
	write_csr(ME_ST0, 567);				
	set_csr(ME_IER, NPE0 | NIE0);
	write_csr(IP10, MEFSM_INT_PRI);		// ME状态机中断(中断号10) 优先级别为6
#endif
#endif
	
	set_csr(DRV1_FCR0, NCALEN);			// FOC计算使能
}


/*---------------------------------------------------------------------------
 * Name		:	FOC_RunModeUpdate
 * Input	:	No
 * Output	:	No
 * Description:	根据运行模式RunMod来设置相应寄存器
 *---------------------------------------------------------------------------*/
void FOC_RunModeUpdate(void)
{	
#if FUNC_SOFTCTRL_ENABLED
	if ((mcFocCtrl.RunMod == CURMOD) ||
		((mcFocCtrl.RunMod == POS_CURMOD) && (FieldSoftControl_GetSwToPosFlag() != 1)))
#elif FUNC_MIT_ENABLED
	if (mcFocCtrl.RunMod == CURMOD || mcFocCtrl.RunMod == POS_SPD_CURMOD)
#else
	if (mcFocCtrl.RunMod == CURMOD)
#endif
	{
		clr_csr(DRV1_FCR1, WPILDEN);	// 速度环输出NFOC_WUFIN自动装载进转矩滤波器输入NFOC_FIX0
		clr_csr(DRV1_FCR1, WPIAUTO1);	// 00: 外环不使能
		clr_csr(DRV1_FCR1, WPIAUTO0);	
	}
	else 
	{
		set_csr(DRV1_FCR1, WPILDEN);	// 速度环输出NFOC_WUFIN自动装载进转矩滤波器输入NFOC_FIX0
		set_csr(DRV1_FCR1, WPIAUTO1);	// 11: 由WPI_CAL_CYCLE1来决定速度环的运行频率
		set_csr(DRV1_FCR1, WPIAUTO0);	
		
#if FUNC_FIELDWEAKEN_ENABLED
		set_csr(DRV1_FCR2, FWEAK_EN);
		set_csr(DRV1_FCR2, FWEAK_MD); // 弱磁模式1使能位有效
#endif
	}

}


/*---------------------------------------------------------------------------
 * Name		:	MotorControlInit
 * Input	:	No
 * Output	:	No
 * Description:	软件变量上电初始化，包括保护参数的初始化、电机状态初始化
 *---------------------------------------------------------------------------*/
void MotorControlInit(void)
{
	Scope_Init();
	
	ElecAngCal_Init();
	
#if FUNC_GAINSW_ENABLED
	GainSW_Demo_Init();  // 增益切换初始化
#endif //#if FUNC_GAINSW_ENABLED
	
	PI_Init();	
	
#if FUNC_FRICOMP_ENABLED
	FricCmp_Demo_Init();
#endif

#if FUNC_FEEDBACKONLOAD_ENABLED
	FeedBackOnLoad_Demo_Init();
#endif
	
#if FUNC_SPDFF_ENABLED
	SpdFF_Init(); // 速度前馈初始化
#endif //#if FUNC_SPDFF_ENABLED

#if FUNC_TOQFF_ENABLED
	ToqFF_Init(); // 转矩前馈初始化
#endif //#if FUNC_TOQFF_ENABLED

	ModeSW_Init(); // 模式切换初始化

#if FUNC_INERTIA_ENABLED
	InFricId_Demo_Init();
#endif //#if FUNC_INERTIA_ENABLED

	InputFilter_Rst();
	
	HardVelFilter_Clr(&HardInFilt1VelFeb);

	HardVelFilter_Clr(&HardInFilt2VelFeb);
	
#if FUNC_PCOM_ENABLED > 0
	PCOM_Init();
#endif
	
#if FUNC_CURRENTBACK_ENABLED
	CurrBack_Demo_Init();
#endif

#if DEADTIME_COMPENSATE_ENABLED
	DeadTimeCompensationInit(usSRegHoldBuf[DEADCOMPCUR]);
#endif
	
#if COMM_CAN_PDO_ENABLED
	DynTrap_Demo_Init();
#endif

    mcFaultDect.pErrorHist = &usSRegInBuf[ERRORTIME0];

	// 固件版本号
    usSRegInBuf[FIRMWAREVERSION_H] = (uint32)FIRMWAREVERSION_I >> 16;
    usSRegInBuf[FIRMWAREVERSION_L] =(uint32)FIRMWAREVERSION_I;
	// 载波周期
    usSRegInBuf[PWMFREQUENCY] = (uint16)PWM_FREQUENCY;
    usSRegInBuf[CURCTRLFREQUENCY] = (uint16)CURCTRL_FREQUENCY;
    usSRegInBuf[VELCTRLFREQUENCY] = (uint16)VELCTRL_FREQUENCY;
    usSRegInBuf[POSCTRLFREQUENCY] = (uint16)POSCTRL_FREQUENCY;
    usSRegInBuf[SERCURFREQ] = (uint16) SERIAL_CURRENT_FREQUENCY;
    usSRegInBuf[SERVELFREQ] = (uint16) SERIAL_VELOCITY_FREQUENCY;
    usSRegInBuf[ANACURFREQ] = (uint16) ANALOG_CURRENT_FREQUENCY;
    usSRegInBuf[ANAVELFREQ] = (uint16) ANALOG_VELOCITY_FREQUENCY;
    usSRegInBuf[SCOPEFREQ] = (uint16) SCOPE_FREQUENCY;
    usSRegInBuf[OUTFILTFREQ] = (uint16) OUTFILT_FREQUENCY;
    usSRegInBuf[VELFEBFREQ] = (uint16) VELFEB_FREQUENCY;
    usSRegInBuf[POSREFFREQ] = (uint16) POSREF_FREQUENCY;
    // 驱动器类型
    usSRegInBuf[DRIVERTYPE] = HARDWARE_VERSION;
    // 主频时钟
    usSRegInBuf[MCUCLOCK] = (uint16)MCU_CLOCK;
    // 功能开关
    usSRegInBuf[FUNCMASK0] = (uint16)(EXCTRL_ECAT_ENABLED | (EXCTRL_PULSE_ENABLED << 1) | (EXCTRL_ANALOG_ENABLED << 2) |
    	(EXCTRL_ENCOUT_ENABLED << 3) | (EXCTRL_CANOPEN_ENABLED << 4) | (MOTOR_VCM_ENABLED << 5) |
    	(ENCODER_SEL_ABZ_ENABLED << 6) | (ENCODER_SEL_HALL_ENABLED << 7) | (ENCODER_SEL_TMG_ENABLED << 8) |
    	(ENCODER_SEL_BISS_ENABLED << 9) | (ENCODER_SEL_HALLSONLY_ENABLED << 10) | (FUNC_REGENERATION_ENABLED << 11) |
    	(FUNC_HOME_ENABLED << 12) | (FUNC_MOTOREST_ENABLED << 13) | (FUNC_CURTUNE_ENABLED << 14) |
    	(FUNC_GAINSW_ENABLED << 15));

    usSRegInBuf[FUNCMASK1] = (uint16)(DOUBLESAMP_ENABLE_SWITCH | (FUNC_SPDFF_ENABLED << 1) | (FUNC_TOQFF_ENABLED << 2) |
    	(FUNC_FORCECTRL_ENABLED << 3) | (FUNC_FORCECLOSEDLOOP_ENABLED << 4) | (FUNC_FRICOMP_ENABLED << 5) |
    	(FUNC_HIGHMODE_ENABLED << 6) | (FUNC_DUTYUPDATENOW_ENABLED << 7) | (SPECIAL_ELESCREW_ENABLE << 8) |
    	(FUNC_ENCODERCOMP_ENABLED << 9) | (FUNC_FEEDBACKONLOAD_ENABLED << 10) | (FUNC_NTF_ENABLED << 11)) |
    	(FUNC_SWEEP_ENABLED << 12);
}


/*---------------------------------------------------------------------------
 * Name		:	Motor_EnableServo
 * Input	:	No
 * Output	:	No
 * Description:	电机上使能
 *---------------------------------------------------------------------------*/
void Motor_EnableServo(void)
{
	FOC_Init(); 

	NFOC_IDREF = 0;
	NFOC_IQREF = 0;
	
	NFOC_UDFIN = 0;
    NFOC_UQFIN = 0;
	
	usSRegInBuf[CMDCUR] = 0;
	usSRegInBuf[MB_IDREF] = 0;
	
#if FUNC_ERRORCORRECTION_ENABLED
	ErrorCorrection_Demo_Init(); // 误差校准初始化
#endif
	
#if FUNC_GAINSW_ENABLED
	GainSW_Demo_Init();  // 增益切换初始化
#endif //#if FUNC_GAINSW_ENABLED
	
	PI_Init();	// 速度环PI初始化
	
#if FUNC_FEEDBACKONLOAD_ENABLED
	FeedBackOnLoad_Demo_Init();
#endif
	
#if FUNC_MIT_ENABLED           //MIT模式
	MIT_Demo_Init();
#endif
	
#if FUNC_FRICOMP_ENABLED
	FricCmp_Demo_Init();
#endif
	
#if FUNC_INERTIA_ENABLED
	InFricId_Demo_Init();
#endif //#if FUNC_INERTIA_ENABLED
	
#if FUNC_CURRENTBACK_ENABLED
	CurrBack_Demo_Init();
#endif
	
	InputFilter_Rst();

	mcFocCtrl.TargetRefFirstFlag = 0; // 计算位置命令的标志位清零
	
	usSRegInBuf[CMDVEL] = 0;
	mcElecAng.FOCThetaFlag = 0;

	Motor_Profile_Reset();	// 初始化轨迹规划相关变量

#if FUNC_NTF_ENABLED
	NTF_Demo_Init();  // 末端抖动抑制初始化
#endif //#if FUNC_NTF_ENABLED
  
#if FUNC_SPDFF_ENABLED
    SpdFF_Init(); // 速度前馈初始化
#endif //#if FUNC_SPDFF_ENABLED

#if FUNC_TOQFF_ENABLED   
    ToqFF_Init(); // 转矩前馈初始化
#endif //#if FUNC_TOQFF_ENABLED
    
#if EXCTRL_ECAT_ENABLED || EXCTRL_CANOPEN_ENABLED || COMM_CAN_PDO_ENABLED
    ECATCSP_Init();// 线性插补初始化
#endif //#if EXCTRL_ECAT_ENABLED
  
#if COMM_CAN_PDO_ENABLED
    DynTrap_Demo_Init();
#endif
   
#if DEADTIME_COMPENSATE_ENABLED
    DeadTimeCompensationInit(usSRegHoldBuf[DEADCOMPCUR]);
#endif
    
	RunMode_Update();
	
	OutputFilter_Init();  // 转矩滤波器初始化
	
#if (HW_UVW_POLARITY == UVWPOL_NORMAL)
	write_csr(DRV1_CMR, 0x0abF);
#else
	write_csr(DRV1_CMR, 0x057F);
#endif
    set_csr(DRV1_OUT, MOE);
    set_csr(DRV1_CR, DRVOE);			// Driver输出使能	0-->Disable		1-->Enable
		
	SetReg(usSRegInBuf[DRIVESTATUS], STATUS_ENABLE, STATUS_ENABLE);
	
#if FUNC_FIELDWEAKEN_ENABLED
	FieldWeakenControlInit();
#endif
	
#if FUNC_SOFTCTRL_ENABLED
	FieldSoftControl_Demo_Init();
#endif
}


/*---------------------------------------------------------------------------
 * Name		:	Motor_DisableServo
 * Input	:	No
 * Output	:	No
 * Description:	电机去使能
 *---------------------------------------------------------------------------*/
void Motor_DisableServo(void)
{
	clr_csr(DRV1_CR, DRVOE);	// Driver输出使能	0-->Disable		1-->Enable
	clr_csr(DRV1_OUT, MOE);
	clr_csr(DRV1_FCR0, NCALEN);	// 关FOC计算使能，相当于关FOC
	mcFocCtrl.CurLoopEnable = 0;
	mcFocCtrl.VelLoopEnable = 0;
	mcFocCtrl.PosLoopEnable = 0;
#if FUNC_FORCECLOSEDLOOP_ENABLED
	mcFocCtrl.FrcLoopEnable = 0;
#endif


	PI_Clear();
	
#if FUNC_FEEDBACKONLOAD_ENABLED
	FeedBackOnLoad_Clr();
#endif
	
#if FUNC_FRICOMP_ENABLED
	FricCmp_Clr();
#endif
	
	InputFilter_Rst();
	
#if FUNC_CURRENTBACK_ENABLED
	CurrBackProtect_Clr(&MotorCurrBack);
#endif
	
#if FUNC_NTF_ENABLED
	NTF_Rst();
#endif //#if FUNC_NTF_ENABLED
	
#if SPECIAL_ELESCREW_ENABLE
	memset(&ScrewCtrl, 0, sizeof(EleScrewTypedf));
	ScrewCtrl.ScrewStep = SCREW_NEAR_STEP;
	ScrewCtrl.ScrewStatus = READY;
	ScrewCtrl.PauseFlag = 0;
#endif //#if SPECIAL_ELESCREW_ENABLE
	
	mcFocCtrl.ThetaErr = 0;

#if FUNC_SOFTCTRL_ENABLED
	FieldSoftControl_Clr();
#endif
	
#if FUNC_MIT_ENABLED
	MIT_Clr();
#endif

#if FUNC_FORCECTRL_ENABLED
	ForceCtrl_Clr();
#endif
	Motor_Profile_Clr();
	ModeSW.SwitchFlag = 0;
	
	memset(&mcSigGen, 0, sizeof(SigGenTypeDef));
	SetReg(usSRegInBuf[DRIVESTATUS], STATUS_ENABLE, 0);  
	
	usSRegInBuf[PTPVCMD] = 0;
	usSRegInBuf[CMDVEL] = 0;
	usSRegInBuf[CMDCUR] = 0;
	usSRegInBuf[ACTCUR] = 0;
}


/*---------------------------------------------------------------------------
 * Name		:	Motor_Open
 * Input	:	No
 * Output	:	No
 * Description:	电机进入运行状态
 *---------------------------------------------------------------------------*/
void Motor_Open(void)
{  
	McStaSet.SetFlag.AutoEnableSetFlag = 1;
		
	Motor_DisableServo();
	Motor_EnableServo();
}


/*---------------------------------------------------------------------------
 * Name		:	Motor_Run
 * Input	:	No
 * Output	:	No
 * Description:	运行状态下的运行模式及参数设置
 *---------------------------------------------------------------------------*/
void Motor_Run(void)
{ 
	// 串口位置模式
	Motor_Profile_Update();
	
	// 串口速度模式
	Motor_SerialIn_Update();
}


/*---------------------------------------------------------------------------
 * Name		:	UpdateParam
 * Input	:	No
 * Output	:	No
 * Description:	根据参数表的变化更新一些参数变量
 *---------------------------------------------------------------------------*/
void UpdateParam(void)
{	
	Scope_Update();
	
#if FUNC_GAINSW_ENABLED
	GainSW_Demo_Update();
#endif //#if FUNC_GAINSW_ENABLED
	
	PI_Update();
	
	InputFilter_Init();
	
	Encoder_Update();
	
	Motor_PhaseFind_Update();
	
	OutputFilter_CoeUpdate();

#if FUNC_SOFTCTRL_ENABLED
	FieldSoftControl_PIDUpdate();
#endif
	
#if FUNC_MIT_ENABLED
	MIT_CmdParaUpdate();
#endif
	
#if FUNC_PROBE_ENABLED
	Probe_Update();
#endif
	
#if FUNC_PCOM_ENABLED
	PCOM_Update();
#endif

    mcRegParam.DriveCtrl = usSRegHoldBuf[DRIVECTRL];

	mcRegParam.ProfileCtrl = usSRegHoldBuf[PROFILECTRL];

    mcRegParam.FlashCtrl = usSRegHoldBuf[FLASHCTRL];

    mcRegParam.DigInStatus = usSRegInBuf[DIGINSTATUS];


    mcRegParam.PulseMode = GetReg(usSRegHoldBuf[DRIVEMODE],MODE_PULSEMODE1 | MODE_PULSEMODE0);
    if(mcRegParam.PulseMode != mcRegParam.PulseMode_Pre)
    {
        mcRegParam.PulseMode_Pre = mcRegParam.PulseMode;
    }

    // 电流环PI或复矢量控制器
    if (usSRegHoldBuf[CVMOD] == 0)
    {
    	NFOC_DKP = usSRegHoldBuf[DQKP];
    	NFOC_DKI = usSRegHoldBuf[DQKI];
    	NFOC_QKP = usSRegHoldBuf[DQKP];
    	NFOC_QKI = usSRegHoldBuf[DQKI];
    }
    else
    {
    	NFOC_DKP = usSRegHoldBuf[DKP];
    	NFOC_DKI = usSRegHoldBuf[DKI];
    	NFOC_QKP = usSRegHoldBuf[QKP];
    	NFOC_QKI = usSRegHoldBuf[QKI];
    	
    	NFOC_CVK1 = ((int32)usSRegHoldBuf[CVK1] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVK2 = ((int32)usSRegHoldBuf[CVK2] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVK3 = ((int32)usSRegHoldBuf[CVK3] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVDK4 = ((int32)usSRegHoldBuf[CVDK4] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVQK4 = ((int32)usSRegHoldBuf[CVQK4] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVDK5 = ((int32)usSRegHoldBuf[CVDK5] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVQK5 = ((int32)usSRegHoldBuf[CVQK5] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVKI2D = ((int32)usSRegHoldBuf[CVKI2D] * (int32)usSRegHoldBuf[CFR]) >> 14;
    	NFOC_CVKI2Q = ((int32)usSRegHoldBuf[CVKI2Q] * (int32)usSRegHoldBuf[CFR]) >> 14;

    	if (mcRegParam.DQKQSel != usSRegHoldBuf[CVKQSEL])
    	{
    		reset_csr(DRV1_FCR7, CVKP_SEL1 | CVKP_SEL0 | CVKI1_SEL1 | CVKI1_SEL0 | CVKI2_SEL1 | CVKI2_SEL0 |
    			CVKO_SEL1 | CVKO_SEL0, usSRegHoldBuf[CVKQSEL] << 8);

    		mcRegParam.DQKQSel = usSRegHoldBuf[CVKQSEL];
    	}
    }


    SetReg(usSRegInBuf[CONTROLSTATUS], CTSTATUS_MCSTATE0 | CTSTATUS_MCSTATE1 | CTSTATUS_MCSTATE2 
                                     | CTSTATUS_MCSTATE3 | CTSTATUS_MCSTATE4, mcState);
    
    mcElecAng.ElecAngDir = ReadBit(usSRegHoldBuf[DRIVEMODE], MODE_ELECANGDIR);
    mcFocCtrl.AngDir = ReadBit(usSRegHoldBuf[DRIVEMODE], MODE_ANGDIR);
    mcFocCtrl.LoadAngDir = ReadBit(usSRegHoldBuf[DRIVEMODE], MODE_SFBANGDIR);
    mcFocCtrl.EAngDir = mcElecAng.ElecAngDir ^ mcFocCtrl.AngDir;

    // MT法
    mcFocCtrl.QepSpeedCoe = usSRegHoldBuf[QEPSPEEDCOE];
    NFOC_MBASE = usSRegHoldBuf[QEPSPEEDCOE];
    NFOC_MSHFT = usSRegHoldBuf[SPEEDCOERANK];
    if (GetReg(usSRegHoldBuf[DRIVESWITCH], SW_MTMODE))
    {
    	NFOC_MARR = VELCTRL_TS * 2;
    }
    else
    {
    	NFOC_MARR = VELCTRL_TS;
    }
    
    
    mcFocCtrl.EncRes = *((uint32*) &usSRegHoldBuf[ENCRES_L]);
    mcFocCtrl.SfbEncRes = *((uint32*) & usSRegHoldBuf[SFBENCRES_L]);
    
    mcFocCtrl.AngToCnt = mcFocCtrl.EncRes / (usSRegHoldBuf[POLES] >> 1);
    mcFocCtrl.QepLoadThetaOffset = *((int32*) & usSRegHoldBuf[SFBANGOFFSET_L]);

    mcFocCtrl.QepThetaOffset = *((int32*) & usSRegHoldBuf[ANGOFFSET_L]);

}


/*---------------------------------------------------------------------------
 * Name		:	Motor_Pulse_Handler
 * Input	:	No
 * Output	:	No
 * Description:	脉冲方向控制
 *---------------------------------------------------------------------------*/
int32 Motor_Pulse_Handler(void)
{	
	return (mcProfile.StartingAngle + read_csr(TIM6_CNTR) * (int16)usSRegHoldBuf[GEARIN] 
		/(int16)usSRegHoldBuf[GEAROUT]);
}


/*---------------------------------------------------------------------------
 * Name		:	RegenerationHandler
 * Input	:	No
 * Output	:	No
 * Description:	再生制动，Handle regeneration base on direct duty control.
 *---------------------------------------------------------------------------*/
void RegenerationHandler(void)
{
	uint16 busvoltage = usSRegInBuf[BUSVOLTAGE];
	uint16 RegenDuty = 0;
	
	if (usSRegHoldBuf[REGENPOW] == 0 || usSRegHoldBuf[REGENRES] == 0 || usSRegHoldBuf[REGENCOE] == 0 ||
		busvoltage <= usSRegHoldBuf[REGENVMLOW] || usSRegHoldBuf[REGENVMLOW] > usSRegHoldBuf[REGENVMHIGH])
	{
		RegenDuty = 0;
	}
	else
	{
		if (busvoltage > usSRegHoldBuf[REGENVMHIGH])
			busvoltage = usSRegHoldBuf[REGENVMHIGH];
		
		RegenDuty = ((uint32)(busvoltage - usSRegHoldBuf[REGENVMLOW]) * usSRegHoldBuf[REGENCOE]) >> 6;
		if (RegenDuty > MAXREGENDUTY)
			RegenDuty = MAXREGENDUTY;
	}
    write_csr(TIM8_DR, MAXREGENDUTY - RegenDuty);

}

#if EXCTRL_ANALOG_ENABLED
/*---------------------------------------------------------------------------
 * Name		:	Motor_AnalogIn_Handler
 * Input	:	No
 * Output	:	No
 * Description:	模拟量控制，VelAnalog and CurAnalog Control
 *---------------------------------------------------------------------------*/
void Motor_AnalogIn_Handler(void)
{
	set_csr(ADC1_CR, ADCBSY);            // ???ADC
	while (readbit_csr(ADC1_CR, ADCBSY)) {};
	
	usSRegInBuf[ANALOGIN] = (int16)(ADCDR_ANAIN >> 3) - usSRegHoldBuf[ANINOFFSET];
	
	if((int16)usSRegInBuf[ANALOGIN] < (int16)usSRegHoldBuf[ANINDEADBAND] && (int16)usSRegInBuf[ANALOGIN] > -(int16)usSRegHoldBuf[ANINDEADBAND])
		mcAnalogInput.AnInValue = 0;
	else
		mcAnalogInput.AnInValue = usSRegInBuf[ANALOGIN];
	
	mcAnalogInput.AnInValue_k += (int32)usSRegHoldBuf[ANINLPFK] * (mcAnalogInput.AnInValue - mcAnalogInput.AnInValue_s);
	mcAnalogInput.AnInValue_s = mcAnalogInput.AnInValue_k >> 15;
	
	if(mcRegParam.WorkMode == VELANALOG) // Velocity loop
        usSRegInBuf[CMDVEL] = ((int32)mcAnalogInput.AnInValue_s * usSRegHoldBuf[ANINVSCALE]) >> 10;
	else if (mcRegParam.WorkMode == CURANALOG) // Current loop
        usSRegInBuf[CMDCUR] = ((int32)mcAnalogInput.AnInValue_s * usSRegHoldBuf[ANINISCALE]) >> 10;
#if FUNC_FORCECLOSEDLOOP_ENABLED
	else // Force loop
		usSRegInBuf[ACTFRC] = ((int32)mcAnalogInput.AnInValue_s * usSRegHoldBuf[ANINFSCALE]) >> 10; // 压力反馈值
#endif
}
#endif // #if EXCTRL_ANALOG_ENABLED


#ifdef FAN_PIN
/*---------------------------------------------------------------------------
 * Name		:	Fan_Control
 * Input	:	No
 * Output	:	No
 * Description:	风扇控制，Control the fan with different duty cycle.
 *---------------------------------------------------------------------------*/
void Fan_Control(void)
{
    xor_csr(FAN_GPIO, FAN_PIN); // FANPIN = ~FANPIN;
}
#endif

/*---------------------------------------------------------------------------*
 * Name		:	void Check_InPos(void)
 * Input	:	NO
 * Output	:	NO
 * Description:	到位信号，The motor is considered settled when PE (position error)  
 *              has remained below PEINPOS for the time defined by PEINPOSTIME.
 *---------------------------------------------------------------------------*/
void Check_InPos(void)
{
	int32 PeinPosRange = 0;
	int32 ThetaErr;
	static int32 TargetAngleLatch = 0;

	if (mcFocCtrl.PosLoopEnable == 1 && mcState != mcFault)						// check in position loop
	{
		ThetaErr = ABS(mcFocCtrl.ThetaErr);
		PeinPosRange = (int32)usSRegHoldBuf[PEINPOSRANGE];
#if FUNC_FEEDBACKONLOAD_ENABLED
		if (FEEDBACK_LOAD == Feedback_GetOnLoadFlag())  
			PeinPosRange = (int16)(((int64)PeinPosRange * Feedback_GetMotorToLoadCoef()) >> LOADCOEF);
#endif		
		if (ThetaErr <= PeinPosRange && (mcFocCtrl.TargetAngleFilt == TargetAngleLatch))
			mcFocCtrl.InPosCounter++;
		else
			mcFocCtrl.InPosCounter = 0;
		TargetAngleLatch = mcFocCtrl.TargetAngleFilt;

		if (mcFocCtrl.InPosCounter >= usSRegHoldBuf[PEINPOSTIME])
		{
			mcFocCtrl.InPosCounter = usSRegHoldBuf[PEINPOSTIME]; // in case overflow
			SetReg(usSRegInBuf[DRIVESTATUS], STATUS_INPOS, STATUS_INPOS);
		}
		else
		{
			SetReg(usSRegInBuf[DRIVESTATUS], STATUS_INPOS, 0);
		}

        /*-----------------------------------STOPPED--------------------------------------*/
		if ((mcRegParam.WorkMode == POSSERIAL 
#if FUNC_MIT_ENABLED
			|| (mcRegParam.WorkMode == MITSERIAL && MIT_GetDynamicWorkMode() == MIT_MODE_POS)
#endif
#if EXCTRL_ECAT_ENABLED || EXCTRL_CANOPEN_ENABLED
			|| usSRegInBuf[FBOPMODE] == PROFILE_POSITION_MODE
#endif
			) && (Position_Profile_GetMotionStatus() == MS_MOVING ||
			(Position_Profile_GetMotionStatus() == MS_FINISH && 
			GetReg(usSRegHoldBuf[PROFILECTRL], PROF_MOTIONEN) != 0 &&
			GetReg(usSRegHoldBuf[PROFILECTRL], PROF_PROFILEREP) != 0)))
		{
			SetReg(usSRegInBuf[DRIVESTATUS], STATUS_STOPPED, STATUS_STOPPED);
		}
		else
		{
			SetReg(usSRegInBuf[DRIVESTATUS], STATUS_STOPPED, 0);
		}
	}
	else
	{
		mcFocCtrl.InPosCounter = 0;
		SetReg(usSRegInBuf[DRIVESTATUS], STATUS_INPOS, 0);
		SetReg(usSRegInBuf[DRIVESTATUS], STATUS_STOPPED, 0);
	}
}

/*---------------------------------------------------------------------------*
 * Name		:	void Reset_PosCmd(void)
 * Input	:	NO
 * Output	:	NO
 * Description:	清零位置指令.
 *---------------------------------------------------------------------------*/
void Reset_PosCmd(void)
{
	mcFocCtrl.TargetRefFirstFlag = 0; // 计算位置命令的标志位清零
	Motor_Profile_Reset();
}


#if MOTOR_VCM_ENABLED
/*---------------------------------------------------------------------------
 * Name		:	VCM_DriverOut
 * Input	:	No
 * Output	:	No
 * Description:	音圈电机，Update PWM of Voice Coil Motor.
 *---------------------------------------------------------------------------*/
void VCM_DriverOut(void)
{
	int16 U_Duty;
	uint16 DR1;
	uint16 DR2;
	
	U_Duty = NFOC_UQFIN;
	U_Duty = ((int32) U_Duty * PWM_VALUE_LOAD) >> 16;

	DR1 = ((PWM_VALUE_LOAD) >> 1) + U_Duty;
	DR2 = ((PWM_VALUE_LOAD) >> 1) - U_Duty;

	NFOC_DR1 = DR1;
	NFOC_DR1N = DR1;
	NFOC_DR2 = DR2;
	NFOC_DR2N = DR2;
	NFOC_DR3 = 0;	// 设为0的话目前是上管常开，下管常闭，W相对地电压为11.8V；因为CMR寄存器设置的极性为下桥正常输出，上桥反向互补输出。
	NFOC_DR3N = 0;	// 设为PWM_VALUE_LOAD的话为下管常开，上管常闭，W相对地电压为0。
}
#endif // #if MOTOR_VCM_ENABLED

