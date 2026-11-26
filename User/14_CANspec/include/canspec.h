/*
 * Copyright (c) 2025-2026 Fortior Technology Co., Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * File Name     : CANspec.h
 * Author        : Summer
 * Date          : 2025-8-30
 * Description   : CANspec main function.
 *
 * Record        :
 * V1.0, 2025-8-30, Summer: Created file
 */

#ifndef _CANSPEC_H
#define _CANSPEC_H

#include "canport.h"
#include "Parameter.h"

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif


	/* ----------------------- Defines ------------------------------------------*/

/*! \brief If Modbus Slave SDO support is enabled. */
#define CS_SLAVE_SDO_ENABLED                    (COMM_CAN_ENABLED)

/*! \brief If Modbus Slave SDO support is enabled. */
#define CS_SLAVE_PDO_ENABLED                    ( COMM_CAN_PDO_ENABLED )


// 数据域最大长度byte
#define CANFD_MAXLEN							( 64 )
#define CAN_MAXLEN								( 8 )

/*! \brief Maximum number of Modbus functions codes the protocol stack
 *    should support.
 *
 * The maximum number of supported Modbus functions must be greater than
 * the sum of all enabled functions in this file and custom function
 * handlers. If set to small adding more functions will fail.
 */
#define CS_FUNC_HANDLERS_MAX                    ( 3 )

/*! \brief If the <em>Read Input Registers</em> function should be enabled. */
#define CS_FUNC_READ_INPUT_ENABLED              ( 0 )
/*! \brief If the <em>Read Holding Registers</em> function should be enabled. */
#define CS_FUNC_READ_HOLDING_ENABLED            ( CS_SLAVE_SDO_ENABLED )
/*! \brief If the <em>Write Single Register</em> function should be enabled. */
#define CS_FUNC_WRITE_HOLDING_ENABLED           ( CS_SLAVE_SDO_ENABLED )
/*! \brief If the <em>Write Multiple registers</em> function should be enabled. */
#define CS_FUNC_WRITE_MULTIPLE_HOLDING_ENABLED  ( 0 )
/*! \brief If the <em>Read Scope Register</em> function should be enabled. */
#define CS_FUNC_READ_SCOPE_ENABLED              ( CS_SLAVE_SDO_ENABLED )
/*! \brief If the <em>Control Frame</em> function should be enabled. */
#define CS_FUNC_CONTROL_FRAME_ENABLED           ( CS_SLAVE_PDO_ENABLED )
/*! @} */


#define CS_ADDRESS_BROADCAST				  ( 0 )   /*! Modbus broadcast address. */
#define CS_ADDRESS_MIN						  ( 1 )   /*! Smallest possible slave address. */
#define CS_ADDRESS_MAX						  ( 7 )   /*! Biggest possible slave address. */
#define CS_FUNC_NONE                          ( 0 )
#define CS_FUNC_READ_HOLDING_REGISTER         ( 3 )
#define CS_FUNC_READ_INPUT_REGISTER           ( 4 )
#define CS_FUNC_WRITE_REGISTER                ( 6 )
#define CS_FUNC_WRITE_MULTIPLE_REGISTERS      ( 16 )
#define CS_FUNC_ERROR                         ( 128 )
#define CS_FUNC_READ_SCOPE_REGISTER           ( 30 )  // self-defined


#define CS_CODEID_SDO						  ( 0xC0 )
#define CS_CODEID_PDO						  ( 0xA0 )


#define CS_PDU_SIZE_MAX     253 /*!< Maximum size of a PDU. */
#define CS_PDU_SIZE_MIN     1   /*!< Function Code */
#define CS_PDU_FUNC_OFF     0   /*!< Offset of function code in PDU. */
#define CS_PDU_DATA_OFF     1   /*!< Offset for response data in PDU. */

	/* ----------------------- Type definitions ---------------------------------*/

	/*! \ingroup modbus
	 * \brief If register should be written or read.
	 *
	 * This value is passed to the callback functions which support either
	 * reading or writing register values. Writing means that the application
	 * registers should be updated and reading means that the modbus protocol
	 * stack needs to know the current register values.
	 *
	 * \see eCSRegHoldingCB( ), eCSRegCoilsCB( ), eCSRegDiscreteCB( ) and
	 *   eCSRegInputCB( ).
	 */
	typedef enum
{
	CS_REG_READ,                /*!< Read register values and pass to protocol stack. */
	CS_REG_WRITE                /*!< Update register values. */
} eCSRegisterMode;

/*! \ingroup modbus
 * \brief Errorcodes used by all function in the protocol stack.
 */
typedef enum
{
	CS_ENOERR,                  /*!< no error. CS_EX_NONE */
	CS_EINVAL,                  /*!< illegal argument. */
	CS_ENOREG,                  /*!< illegal register address. CS_EX_ILLEGAL_DATA_ADDRESS */
	CS_EPORTERR,                /*!< porting layer error. */
	CS_EIO,                     /*!< I/O error. CS_EX_SLAVE_DEVICE_FAILURE */
	CS_ENORES,                  /*!< insufficient resources. */
	CS_ETIMEDOUT,               /*!< timeout error occurred. CS_EX_SLAVE_BUSY */
	CS_EILLSTATE,               /*!< protocol stack in illegal state. */
} eCSErrorCode;


typedef enum
{
	CS_EX_NONE = 0x00,
	CS_EX_ILLEGAL_FUNCTION = 0x01,
	CS_EX_ILLEGAL_DATA_ADDRESS = 0x02,
	CS_EX_ILLEGAL_DATA_VALUE = 0x03,
	CS_EX_ILLEGAL_DATA_LENGTH = 0x04,
	CS_EX_ERROR_NODEID = 0x05,
	CS_EX_ERROR_CODID = 0x06,
	CS_EX_SLAVE_BUSY = 0x07,
} eCSException;


typedef eCSException(*pxCSFunctionHandler)(UCHAR * pucFrame, USHORT * pusLength);

typedef struct
{
	UCHAR ucFunctionCode;
	pxCSFunctionHandler pxHandler;
} xCSFunctionHandler;

/* ----------------------- Function prototypes ------------------------------*/
/*! \ingroup modbus
 * \brief Initialize the Modbus protocol stack.
 *
 * This functions initializes the ASCII or RTU module and calls the
 * init functions of the porting layer to prepare the hardware. Please
 * note that the receiver is still disabled and no Modbus frames are
 * processed until eCSEnable( ) has been called.
 *
 * \param eMode If ASCII or RTU mode should be used.
 * \param ucSlaveAddress The slave address. Only frames sent to this
 *   address or to the broadcast address are processed.
 * \param ucPort The port to use. E.g. 1 for COM1 on windows. This value
 *   is platform dependent and some ports simply choose to ignore it.
 * \param ulBaudRate The baudrate. E.g. 19200. Supported baudrates depend
 *   on the porting layer.
 * \param eParity Parity used for serial transmission.
 *
 * \return If no error occurs the function returns eCSErrorCode::CS_ENOERR.
 *   The protocol is then in the disabled state and ready for activation
 *   by calling eCSEnable( ). Otherwise one of the following error codes
 *   is returned:
 *    - eCSErrorCode::CS_EINVAL If the slave address was not valid. Valid
 *        slave addresses are in the range 1 - 247.
 *    - eCSErrorCode::CS_EPORTERR IF the porting layer returned an error.
 */
eCSErrorCode eCSInit(USHORT baudrate);


/*! \ingroup modbus
 * \brief Release resources used by the protocol stack.
 *
 * This function disables the Modbus protocol stack and release all
 * hardware resources. It must only be called when the protocol stack
 * is disabled.
 *
 * \note Note all ports implement this function. A port which wants to
 *   get an callback must define the macro CS_PORT_HAS_CLOSE to 1.
 *
 * \return If the resources where released it return eCSErrorCode::CS_ENOERR.
 *   If the protocol stack is not in the disabled state it returns
 *   eCSErrorCode::CS_EILLSTATE.
 */
//eCSErrorCode eCSClose(void);

/*! \ingroup modbus
 * \brief Enable the Modbus protocol stack.
 *
 * This function enables processing of Modbus frames. Enabling the protocol
 * stack is only possible if it is in the disabled state.
 *
 * \return If the protocol stack is now in the state enabled it returns
 *   eCSErrorCode::CS_ENOERR. If it was not in the disabled state it
 *   return eCSErrorCode::CS_EILLSTATE.
 */
eCSErrorCode eCSEnable(void);

/*! \ingroup modbus
 * \brief Disable the Modbus protocol stack.
 *
 * This function disables processing of Modbus frames.
 *
 * \return If the protocol stack has been disabled it returns
 *  eCSErrorCode::CS_ENOERR. If it was not in the enabled state it returns
 *  eCSErrorCode::CS_EILLSTATE.
 */
eCSErrorCode eCSDisable(void);

/*! \ingroup modbus
 * \brief The main pooling loop of the Modbus protocol stack.
 *
 * This function must be called periodically. The timer interval required
 * is given by the application dependent Modbus slave timeout. Internally the
 * function calls xCSPortEventGet() and waits for an event from the receiver or
 * transmitter state machines.
 *
 * \return If the protocol stack is not in the enabled state the function
 *   returns eCSErrorCode::CS_EILLSTATE. Otherwise it returns
 *   eCSErrorCode::CS_ENOERR.
 */
eCSErrorCode eCSPoll(void);




#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
