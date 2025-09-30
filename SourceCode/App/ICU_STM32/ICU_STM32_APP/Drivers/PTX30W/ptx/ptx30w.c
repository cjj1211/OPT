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
    File        : ptx30w.c

    Description : Top level API
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxPlat.h"
#include "ptx30w.h"
#include "ptx30w_Hip.h"
#include "ptx30w_Nsc.h"
#include <string.h>

/**
 * \brief Converts the digital value from the voltage monitor into millivolts.
 *
 * \param adcVal Digital value measured by the VoltageMonitor.
 * \return Voltage in millivolts.
 */
static uint16_t adcToMv(uint8_t adcVal);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptxStatus_t ptx30w_Init(uint8_t address, bool crc, bool ack)
{
    ptxStatus_t status;

    status = ptxPlat_I2C_Init();

    if (ptxStatus_Success == status)
    {
        status = ptxPlat_IRQ_Init();
    }

    if (ptxStatus_Success == status)
    {
        /** Configure the PTX30W Command interface. */
        const ptx30wHif_t ptx30wHifCfg =
        {
            .ChainingBit = false,
            .AkPresent = ack,
            .CrcPresent = crc,
            .I2cAddr = address
        };

        status = ptx30wHip_InitHifParameters(&ptx30wHifCfg);
    }

    ptxDeviceInformation_t device_information;
    memset(&device_information, 0, sizeof(ptxDeviceInformation_t));

    if (ptxStatus_Success == status)
    {
        (void) ptx30wNsc_Init();

        /** Dummy communication to test if communication via I2C bus. */
        status = ptx30w_GetDeviceInformation(&device_information);
    }

    if (ptxStatus_Success == status)
    {
        if(PTX30W_HW_VERSION != device_information.HardwareVersion)
        {
            status = ptxStatus_WrongHardware;
        }
    }

    return status;
}

void ptx30w_Deinit()
{
    (void) ptxPlat_IRQ_Deinit();
    (void) ptxPlat_I2C_Deinit();
}

ptxStatus_t ptx30w_GetDeviceInformation(ptxDeviceInformation_t *deviceInfo)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != deviceInfo)
    {
        uint8_t req_len = PTX30W_DEVICEINFO_LEN;
        uint8_t buffer[req_len];

        memset(buffer, 0, req_len);

        status = ptx30wHip_ReadSystemStatus(buffer, req_len);

        if(ptxStatus_Success == status)
        {
            /** Map the parameters. */
            deviceInfo->CommandStatus = buffer[0];
            deviceInfo->HardwareVersion = buffer[1];
            deviceInfo->FirmwareVersion = (uint16_t) ((((uint16_t)buffer[2]) << 8U) | ((uint16_t)buffer[3]));
            memcpy(deviceInfo->DieInfo, &buffer[4], PTX30W_DIEINFO_LEN);
            deviceInfo->OemValid = buffer[20];
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}

ptxStatus_t ptx30w_GetSystemStatus(ptxSystemStatus_t *systemStatus)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != systemStatus)
    {
        memset(systemStatus, 0, sizeof(ptxSystemStatus_t));

        ptxRuntimeParam_t parameters[PTX30W_SYS_GET_PARAM_CNT];
        memset(parameters, 0, sizeof(ptxRuntimeParam_t) * PTX30W_SYS_GET_PARAM_CNT);

        ptxRuntimeParameters_t system_params;

        status = ptx30wNsc_ReadRuntimeParameters(&system_params);

        if(ptxStatus_Success == status)
        {
            /** Remap the parameters. */
            systemStatus->ChargerEnabled    = (0U != system_params.BcEnable ? true : false);
            systemStatus->RfFieldDetected   = (0U != system_params.RffStatus ? true : false);
            systemStatus->Error             = (ptxErrorStatus_t) system_params.ErrorStatus;
            systemStatus->ChargerStatus     = (ptxBcStatus_t) system_params.BcStatus;
            systemStatus->VddBat            = adcToMv(system_params.VdbatAdcVal);
            systemStatus->VddC              = adcToMv(system_params.VddcAdcVal);
            systemStatus->NtcStatus         = (ptxNtcStatus_t) system_params.NtcStatus;
            systemStatus->WlcpStatus        = (system_params.WlcpConnected & PTX30W_WLCP_STATUS_MASK);
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}

ptxStatus_t ptx30w_SetChargingParams(ptxChargingParams_t *chargingParams)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != chargingParams)
    {
        ptxParameter_t params[5];
        memset(params, 0, sizeof(ptxParameter_t) * 5);

        params[0].Type = ParamType_ChargerEnable;
        params[0].Value.ChargerEnable = chargingParams->EnableCharging;

        params[1].Type = ParamType_ChargeCurrent;
        params[1].Value.ChargeCurrent = chargingParams->ChargeCurrent;

        params[2].Type = ParamType_TerminationVoltage;
        params[2].Value.TerminationVoltage = chargingParams->TerminationVoltage;

        params[3].Type = ParamType_RechargeVoltage;
        params[3].Value.RechargeVoltage = chargingParams->RechargeVoltage;

        params[4].Type = ParamType_TrickleVoltage;
        params[4].Value.TrickleVoltage = chargingParams->TrickleVoltage;

        status = ptx30w_SetRuntimeParams(params, 5);
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}

ptxStatus_t ptx30w_SetRuntimeParams(ptxParameter_t *params, uint8_t paramLen)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != params) && (paramLen > 0) && (paramLen <=  PTX30W_SYS_SET_PARAM_CNT))
    {
        ptxRuntimeParam_t parameters[PTX30W_SYS_SET_PARAM_CNT];
        memset(parameters, 0, sizeof(ptxRuntimeParam_t) * PTX30W_SYS_SET_PARAM_CNT);

        for(uint8_t i = 0; (i < paramLen) && (ptxStatus_Success == status); ++i)
        {
            switch(params[i].Type)
            {
                case ParamType_ChargeCurrent:
                    parameters[i].Type = NscParamType_BcIchgCtrl;
                    parameters[i].Value = (uint8_t) (MASK_BC_ICHG_CTRL & (((params[i].Value.ChargeCurrent - 1U) >> 1U) << POS_BC_ICHG_CTRL));
                    break;
                case ParamType_TerminationVoltage:
                    parameters[i].Type = NscParamType_BcVtermCtrl;
                    parameters[i].Value = (uint8_t) (MASK_BC_VTERM_CTRL & (params[i].Value.TerminationVoltage << POS_BC_VTERM_CTRL));
                    break;
                case ParamType_TrickleVoltage:
                    parameters[i].Type = NscParamType_BcVtrckCtrl;
                    parameters[i].Value = (uint8_t) (MASK_BC_VTRK_CTRL & (params[i].Value.TrickleVoltage << POS_BC_VTRK_CTRL));
                    break;
                case ParamType_RechargeVoltage:
                    parameters[i].Type = NscParamType_BcVrchgCtrl;
                    parameters[i].Value = (uint8_t) (MASK_BC_VRCHG_CTRL & (params[i].Value.RechargeVoltage << POS_BC_VRCHG_CTRL));
                    break;
                case ParamType_ChargerEnable:
                    parameters[i].Type = NscParamType_BcEnable;
                    parameters[i].Value = params[i].Value.ChargerEnable;
                    break;
                case ParamType_WptDuration:
                    parameters[i].Type = NscParamType_HostWptDurationInt;
                    parameters[i].Value = params[i].Value.WptDuration;
                    break;
                case ParamType_ShippingMode:
                    parameters[i].Type = NscParamType_ShippingModeEnable;
                    parameters[i].Value = params[i].Value.ShippingMode;
                    break;
                case ParamType_WptRequestSource:
                    parameters[i].Type = NscParamType_WptReqSel;
                    parameters[i].Value = params[i].Value.WptRequest;
                    break;
                case ParamType_DetuneEnable:
                    parameters[i].Type = NscParamType_DetuneEnable;
                    parameters[i].Value = params[i].Value.DetuneEnable;
                    break;
                case ParamType_NfcEnable:
                    parameters[i].Type = NscParamType_NfcEnable;
                    parameters[i].Value = params[i].Value.NfcEnable;
                    break;
                case ParamType_LimiterVoltage:
                    parameters[i].Type = NscParamType_LimThSel;
                    parameters[i].Value = params[i].Value.LimiterVoltage;
                    break;
                default:
                    status = ptxStatus_InvalidParameter;
                    break;
            }
        }

        if(ptxStatus_Success == status)
        {
            status = ptx30wNsc_WriteRuntimeParameters(parameters, paramLen);
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}

ptxStatus_t ptx30w_SetHostWptDuration(ptx30wWptDuration_t wptDuration)
{
    ptxParameter_t param;

    param.Type = ParamType_WptDuration;
    param.Value.WptDuration = wptDuration;

    return ptx30w_SetRuntimeParams(&param, 1U);
}

ptxStatus_t ptx30w_EnterShippingMode()
{
    ptxParameter_t param;

    param.Type = ParamType_ShippingMode;
    param.Value.ShippingMode = true;

    return ptx30w_SetRuntimeParams(&param, 1U);
}

ptxStatus_t ptx30w_SetWptReqSel(ptx30wWptReqSel_t wptReqSel)
{
    ptxParameter_t param;

    param.Type = ParamType_DetuneEnable;
    param.Value.WptRequest = wptReqSel;

    return ptx30w_SetRuntimeParams(&param, 1U);
}

ptxStatus_t ptx30w_EnableDetuning(bool enable)
{
    ptxParameter_t param;

    param.Type = ParamType_DetuneEnable;
    param.Value.DetuneEnable = enable;

    return ptx30w_SetRuntimeParams(&param, 1U);
}

ptxStatus_t ptx30w_EnableNfc(bool enable)
{
    ptxParameter_t param;

    param.Type = ParamType_NfcEnable;
    param.Value.NfcEnable = enable;

    return ptx30w_SetRuntimeParams(&param, 1U);
}

// ptxStatus_t ptx30w_SetCustomNdefMessage(ptxNDEFRecord_t *records, uint8_t recordsLen)
// {
//     ptxStatus_t status = ptxStatus_Success;

//     if( (NULL != records) && (0 != recordsLen) )
//     {
//         uint8_t rec_data[144];
//         size_t rec_data_len = sizeof(rec_data);

//         status = ptxNDEFMessage_Create(records, recordsLen, rec_data,  &rec_data_len);

//         if (ptxStatus_Success == status)
//         {
//             status = ptx30wNsc_SetCustomNdefMessage(rec_data, (uint8_t) rec_data_len);
//         }
//     }
//     else
//     {
//         status = ptxStatus_InvalidParameter;
//     }

//     return status;
// }

ptxStatus_t ptx30w_WriteOemParameters(const ptxOemConfigParam_t *oemParameters)
{
    return ptx30wNsc_WriteOemParameters(oemParameters);
}

ptxStatus_t ptx30w_TDC_Read(uint8_t *rxData, uint8_t *rxDataLen, uint32_t rxTimeoutMs)
{
    ptxStatus_t status = ptxStatus_Success;

    ptxStatus_t temp_status = ptxPlat_IRQ_WaitForIrq(rxTimeoutMs);

    if(ptxStatus_Success == temp_status)
    {
        (void) ptx30wNsc_Tdc_Handle();
    }

    // delay_ms(5);
    status = ptx30wNsc_Tdc_RxMessage(rxData, rxDataLen);

    return status;
}

ptxStatus_t ptx30w_TDC_Write(uint8_t *txData, uint8_t txDataLen, uint32_t ackTimeoutMs)
{
    ptxStatus_t status = ptxStatus_Success;

    status = ptx30wNsc_Tdc_TxMessage(txData, txDataLen);

    if( (0 != ackTimeoutMs) && (ptxStatus_Success == status) )
    {
        uint8_t is_received = 0;

        status = ptxPlat_IRQ_WaitForIrq(ackTimeoutMs);

        if(ptxStatus_Success == status)
        {
            status = ptx30wNsc_Tdc_TxMessageReceived(&is_received);
            // printf("ptx30wNsc_Tdc_TxMessageReceived=[%d] is_received=[%d]\n", status, is_received);
            // delay_ms(5);
        }

        if( (ptxStatus_Success == status) && (0 == is_received))
        {
            status = ptxStatus_TimeOut;
        }
    }

    return status;
}

ptxStatus_t ptx30w_TDC_IsReceived(uint8_t *received)
{
    return ptx30wNsc_Tdc_TxMessageReceived(received);
}

static uint16_t adcToMv(uint8_t adcVal)
{
    return (uint16_t) ( PTX30W_VMON_OFFSET_MV + ( (((uint16_t) adcVal) * PTX30W_VMON_UV_PER_LSB) / 10U) );
}
