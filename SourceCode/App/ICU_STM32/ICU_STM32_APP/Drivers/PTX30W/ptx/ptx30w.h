/** \file
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2024, Renesas Electronics Corporation and/or its affiliates


    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of
       conditions and the following disclaimer in the documentation and/or other
       materials provided with the distribution.

    3. Neither the name of Renesas nor the names of its
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission.



    THIS SOFTWARE IS PROVIDED BY Renesas "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL RENESAS OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ---------------------------------------------------------------

    Project     : PTX30W
    Module      : PTX30W API
    File        : ptx30w.h
*/
#ifndef PTX_PTX30W_H_
#define PTX_PTX30W_H_

#include "ptxStatus.h"
#include "ptx30w_Hip.h"
#include "ptx30w_Nsc.h"
#include "ptx30w_ConfigHelper.h"
#include "ptxNDEFMessage.h"
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
#define PTX30W_VMON_UV_PER_LSB      (125U)      /**< Resolution of the VoltageMonitor (12.5mV per LSB). */
#define PTX30W_VMON_OFFSET_MV       (2400U)     /**< Offset of the voltage monitor (not able to measure below 2.4V). */
#define PTX30W_SYS_GET_PARAM_CNT    (8U)        /**< Amount of parameters returned by the NSC_GET_PARAM_CMD. */
#define PTX30W_SYS_SET_PARAM_CNT    (11U)       /**< Max. amount of parameters set by the NSC_SET_PARAM_CMD. */
#define PTX30W_DIEINFO_LEN          (16U)       /**< Length of the die info. */
#define PTX30W_DEVICEINFO_LEN       (21U)       /**< Length of the device information. */
#define PTX30W_HW_VERSION           (0x21)      /**< Hardware version for B21 silicon. */
#define PTX30W_WLCP_STATUS_MASK     (0x03)      /**< Bitmask for ptxWlcpStatus_t. */

/*
 * ####################################################################################################################
 * TYPES
 * ####################################################################################################################
 */
/** Error status of the system. */
typedef enum ptxErrorStatus
{
    ErrorStatus_OK              = 0x00, /** System OK */
    ErrorStatus_IcTemperature   = 0x01, /** IC temperature error */
    ErrorStatus_Protocol        = 0x02, /** Protocol error */
    ErrorStatus_BattConnection  = 0x04, /** Battery connection error */
    ErrorStatus_BattTemperature = 0x08, /** Battery temperature error */
    ErrorStatus_TcmTimeout      = 0x0C, /** Trickle charge timeout */
    ErrorStatus_CcmTimeout      = 0x10, /** Constant current charging timeout */
    ErrorStatus_CvmTimeout      = 0x14, /** Constant voltage charging timeout */
    ErrorStatus_RFU                     /** Reserved for future use. */
} ptxErrorStatus_t;

/** Status of the battery charger. */
typedef enum ptxBcStatus
{
    BcStatus_Disabled       = 0x00, /** Charging disabled */
    BcStatus_Tcm            = 0x01, /** Trickle charging */
    BcStatus_Ccm            = 0x02, /** Constant current charging */
    BcStatus_Cvm            = 0x03, /** Constant voltage charging */
    BcStatus_ChargingDone   = 0x04, /** Charging finished */
    BcStatus_RFU                    /** Reserved for future use. */
} ptxBcStatus_t;

/** Error status of the NTC. */
typedef enum ptxNtcStatus
{
    NtcStatus_Normal    = 0x00, /** Normal temperature */
    NtcStatus_Cold      = 0x02, /** Under temperature: When set, battery is under 10 degree Celsius. */
    NtcStatus_ECold     = 0x03, /** Extended under temperature: When set, battery is under 0 degree Celsius. */
    NtcStatus_Hot       = 0x04, /** Over temperature: When set, battery is above 45 degree Celsius. */
    NtcStatus_EHot      = 0x0C, /** Extended over temperature: When set, battery is above 60 degree Celsius. */
    NtcStatus_RFU
} ptxNtcStatus_t;

/** Status of the Poller. */
typedef enum ptxWlcpStatus
{
    WlcpStatus_NotConnected = 0x00, /** No WLCP device connected */
    WlcpStatus_Connected    = 0x01, /** WLCP device connected (WLCP-INFO record written) */
    WlcpStatus_Charging     = 0x03, /** Charging started (1st WLCL-CTL record read) */
    WlcpStatus_RFU                  /** Reserved for future use. */
} ptxWlcpStatus_t;

/** Enum describing the types of runtime parameters. */
typedef enum ptxRuntimeParamType
{
    ParamType_ChargeCurrent,        /** Parameter type to set the charging current. */
    ParamType_TerminationVoltage,   /** Parameter type to set the termination voltage. */
    ParamType_TrickleVoltage,       /** Parameter type to set the trickle voltage. */
    ParamType_RechargeVoltage,      /** Parameter type to set the recharge voltage. */
    ParamType_ChargerEnable,        /** Parameter type to set enable/disable the internal charger. */
    ParamType_WptDuration,          /** Parameter type to set the WPT duration. */
    ParamType_ShippingMode,         /** Parameter type to enable/disable shipping mode. */
    ParamType_WptRequestSource,     /** Parameter type to set the source for the WPT_Req bit in the WLC record. */
    ParamType_DetuneEnable,         /** Parameter type to enable/disable RF detuning. */
    ParamType_NfcEnable,            /** Parameter type to enable/disable NFC communication. */
    ParamType_LimiterVoltage,       /** Parameter type to set the voltage limiter threshold. */
    ParamType_RFU                   /** Reserved for future use. */
} ptxRuntimeParamType_t;

/** Union encapsulating all the supported runtime parameters. */
typedef union ptxRuntimeParamValue
{
    bool                    ChargerEnable;      /** Enable/disable the charger. */
    uint8_t                 ChargeCurrent;      /** Charge current in milliamperes. */
    ptx30wVTerm_t           TerminationVoltage; /** Termination voltage, please  see \ref ptx30wVTerm_t. */
    ptx30wVTrickle_t        TrickleVoltage;     /** Trickle voltage, please see \ref ptx30wVTrickle_t. */
    ptx30wVRecharge_t       RechargeVoltage;    /** Recharge voltage, please see \ref ptx30wVRecharge_t. */
    ptx30wLimiterVoltage_t  LimiterVoltage;     /** Limiter voltage, please see \ref ptx30wLimiterVoltage_t. */
    ptx30wWptDuration_t     WptDuration;		/** WPT duration, please see \ref ptx30wWptDuration_t. */
    ptx30wWptReqSel_t     	WptRequest;			/** WPT request (source), please see \ref ptx30wWptReqSel_t. */
    bool                    DetuneEnable;       /** Enable/Disable RF-detuning. */
    bool                    ShippingMode;       /** Enable/Disable the shipping mode. */
    bool                    NfcEnable;          /** Enable/Disable the NFC interface. */
    uint8_t                 Raw;                /** For accessing the "raw" value (used internally). */
} ptxRuntimeParamValue_t;

/** Struct representing the runtime parameter type and the corresponding value. */
typedef struct ptxParameter
{
    ptxRuntimeParamType_t   Type;   /** Parameter type. */
    ptxRuntimeParamValue_t  Value;  /** Parameter value. */
} ptxParameter_t;

/** Struct representing all available status parameters. */
typedef struct ptxSystemStatus
{
    bool                ChargerEnabled; /** Indicates if the battery charger is enabled. */
    bool                RfFieldDetected;/** Indicates the status of the RF field. */
    ptxErrorStatus_t    Error;          /** Error status of the system. */
    ptxBcStatus_t       ChargerStatus;  /** Battery charger status. The status is only valid in case an RF-Field is present. */
    uint16_t            VddBat;         /** VddBat Voltage in mV. VddBat is set to 2400 when RF field is present (RFF_STATUS == 1)
                                         *  and charging is NOT in progress.
                                         */
    uint16_t            VddC;           /** Vddc Voltage in mV */
    ptxNtcStatus_t      NtcStatus;      /** NTC temperature status. */
    ptxWlcpStatus_t     WlcpStatus;     /** Is the WLC-Poller connected? Is it charging? */
} ptxSystemStatus_t;

/** Struct representing all available charging parameters. */
typedef struct ptxChargingParams
{
    uint8_t             ChargeCurrent;      /** Desired charge current in milliamperes. (Min = 5mA; Max = 250mA). */
    ptx30wVTerm_t       TerminationVoltage; /** Termination voltage, see \ref ptx30wVTerm_t. */
    ptx30wVTrickle_t    TrickleVoltage;     /** Trickle voltage, see \ref ptx30wVTrickle_t. */
    ptx30wVRecharge_t   RechargeVoltage;    /** Recharge voltage, see \ref ptx30wVRecharge_t. */
    bool                EnableCharging;     /** Control battery charging. This will enable/disable the current flow
                                             *  into the battery, however it will NOT affect WptReqSel!
                                             */
} ptxChargingParams_t;

/** Struct representing all available HW-info parameters. */
typedef struct ptxDeviceInformation
{
    ptxCommandStatus_t  CommandStatus;                  /** Information if the previous HIP command has failed or not. */
    uint8_t             HardwareVersion;                /** Hardware version (A10, B20, etc.). */
    uint16_t            FirmwareVersion;                /** Firmware version number. */
    uint8_t             DieInfo[PTX30W_DIEINFO_LEN];    /** Die information. */
    uint8_t             OemValid;                       /** Oem valid flag. */
} ptxDeviceInformation_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief   Initializes the PTX30W interface and verifies the HW revision of the PTX30W.
 *          Attention: This API can fail with two error codes:
 *          - A "ptxStatus_WrongHardware" error means that the hardware version is too old or the PTX30W does not have
 *            a valid firmware image flashed.
 *          - A "ptxStatus_InterfaceError" means the host MCU could not receive the I2C_ACK. There are three main reasons
 *            why this can happen:
 *              1)  The wrong I2C address was passed to ptx30w_Init().
 *              2)  SCL or SDA is not connected correctly.
 *              3)  PTX30W is still in shipping mode.
 *
 * \param[in] address   I2C slave address of the PTX30W.
 * \param[in] crc       Enable/Disable CRC check for I2C communication
 * \param[in] ack       Enable/Disable acknowledge response for write commands.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_Init(uint8_t address, bool crc, bool ack);

/**
 * \brief Deinitializes the PTX30W interface.
 *
 */
void ptx30w_Deinit();

/**
 * \brief Retrieves the hardware version, firmware version and OEM parameter status from the device.
 *
 * \param[in,out] deviceInfo Pointer to struct for storing the device information.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_GetDeviceInformation(ptxDeviceInformation_t *deviceInfo);

/**
 * \brief Retrieves all available runtime informations at once and maps them to human
 *          readable values (StatusCodes, ErrorCodes, etc.).
 *
 * \param[in,out] systemControl Pointer to struct for storing the state of the system.
 *                  Contains information about charging status, battery voltage, voltage of
 *                  VDDC, presence of RF field.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_GetSystemStatus(ptxSystemStatus_t *systemStatus);

/**
 * \brief Set charging parameters such as the charge current, termination voltage, recharge voltage
 *          during runtime.
 * Attention when setting "RechargeVoltage": It shall be set at least 300mV lower than TerminationVoltage.
 * Attention when setting "ChargeCurrent": It shall be set equal or lower than NFC_ICHG (which is set during "ptx30wOemConfig_SetBcICharge()").
 *
 * \param[in] chargingParams Pointer to struct containing all available charging parameters.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_SetChargingParams(ptxChargingParams_t *chargingParams);

/**
 * \brief Set individual charging parameters such as the charge current, termination voltage, recharge voltage
 *          during runtime.
 * Attention when setting "RechargeVoltage": It shall be set at least 300mV lower than TerminationVoltage.
 * Attention when setting "ChargeCurrent": It shall be set equal or lower than NFC_ICHG (which is set during "ptx30wOemConfig_SetBcICharge()").
 *
 * \param[in] params    Pointer to an array of parameters to be written to the device.
 * \param[in] paramLen  Length of parameters to be written to the device.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_SetRuntimeParams(ptxParameter_t *params, uint8_t paramLen);

/**
* \brief Sets the duration of the charging cycle.
*
* \param[in] wptDuration    Duration of the WPT charging cycle (between 8ms and 70min)
*
* \return Status of the operation see \ref ptxStatus_t
*/
ptxStatus_t ptx30w_SetHostWptDuration(ptx30wWptDuration_t wptDuration);

/**
 * \brief Calling this function will cause the 30W to re-enter shipping mode!
 *       The device will go into an ultra low power state. It won't respond to I2C
 *       commands anymore. Wakeup via SM-Pin or WPT-Cycle only.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_EnterShippingMode();

/**
 * \brief Set the source of the WPT Request Bit
 *
 * \param[in] wptReqSel  Source of the WPT_REQ bit. It is either permanently
 *                           enabled/disabled or controlled by the internal battery charger.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_SetWptReqSel(ptx30wWptReqSel_t wptReqSel);

/**
 * \brief Enables/disables the RF-detuning feature of the 30W.
 *
 * \param[in] enable     Enable/Disable RF-detuning.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_EnableDetuning(bool enable);

/**
 * \brief Enables/disables the NFC communication interface of the 30W.
 *
 * \param[in] enable Enable/Disable NFC communication.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_EnableNfc(bool enable);

/**
 * \brief Writes a custom NDEF message to the 30W's memory, which can be read
 *           with a smartphone or any other mobile device supporting NFC.
 *
 * \param[in] records    Pointer to an array of NDEF records.
 * \param[in] recordsLen Amount of NDEF records to be written.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_SetCustomNdefMessage(ptxNDEFRecord_t *records, uint8_t recordsLen);

/**
 * \brief Writes the OEM parameters into the 30W's NVM. This function can only be called
 *           once after factory reset. If successful, the PTX30W will do a soft reset (reboot).
 *
 * \param[in] oemParameters Pointer the OEM parameter data structure.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_WriteOemParameters(const ptxOemConfigParam_t *oemParameters);

/**
 * \brief Reads data from the 30W's internal message buffer (up to 63 bytes of payload).
 *
 * \param[in, out]   rxData      Valid pointer to a buffer, for storing the received data.
 * \param[in, out]   rxDataLen   Pointer to variable containing the length of the available buffer.
 *                               On success, the variable will contain the actual number of received bytes.
 * \param[in]        rxTimeoutMs Timeout to wait for the message from the Poller.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_TDC_Read(uint8_t *rxData, uint8_t *rxDataLen, uint32_t rxTimeoutMs);

/**
 * \brief Writes data to the 30W's internal message buffer (up to 63 bytes of payload).
 *
 * \param[in] txData             Valid pointer to a buffer to be transmitted.
 * \param[in] txDataLen          Length variable storing the number of bytes to be transmitted.
 * \param[in] ackTimeoutMs       Timeout in ms to wait for an acknowledge from the Poller.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_TDC_Write(uint8_t *txData, uint8_t txDataLen, uint32_t ackTimeoutMs);

/**
 * \brief Checks if the previously sent message was received by the Poller.
 *
 * \param[in] received Pointer to store the received status.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30w_TDC_IsReceived(uint8_t *received);


#ifdef __cplusplus
}
#endif

#endif /* Guard */
