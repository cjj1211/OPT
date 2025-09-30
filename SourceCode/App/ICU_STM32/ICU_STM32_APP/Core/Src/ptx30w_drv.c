/**
  ******************************************************************************
  * @file    ptx30w_drv.c
  * @brief   This file provides code for the drivers of the PTX30W.
  ******************************************************************************
  *  作者 : 张巍 wei.zhang@neuroxess.com
  *  描述 : PTX30W驱动
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ptx30w_drv.h"
#include "ptx30w.h"
#include "ptxStatus.h"
#include "ptxPlat.h"
#include "ptx30w_ConfigHelper.h"
#include "ptx30w_Hip_Int.h"
#include "i2c.h"
#include "protocol.h"

#define PTX30W_RT_PARAM_CNT    (1)

#define PTX_PAYLOAD_LEN         (62u)
#define PTX_REQ_FRAME_LEN       (0x05)
#define PTX_POLL_WRITE_REQ_OPC  (0xA5)
#define PTX_POLL_READ_REQ_OPC   (0xA6)
#define PTX_HLP_TIMEOUT         (35u)

ptxSystemStatus_t system_status;

void ptx30wMemoryDump(void)
{
    ptxStatus_t status = ptxStatus_Success;
    uint16_t data[8] = {0};

    for (uint16_t i = 0x800; (i < 0xfff) && (ptxStatus_Success == status); i += 8) {
        status = ptx30wHip_ReadCodeMemory(i, data, 8);
        printf("/* 0x%04X */ 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X status:%d\r\n", i, \
            data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], status);
    }
}

/*  SDK V2.3.0  */
int ptx30w_driver_init(void)
{
    ptxStatus_t status;

    status = ptx30w_Init(0x4B, false, false);

    ptxDeviceInformation_t device_information;
    memset(&device_information, 0, sizeof(ptxDeviceInformation_t));

    if (ptxStatus_Success == status) {
        /* Retrieve information about the hardware version and firmware version. */
        status = ptx30w_GetDeviceInformation(&device_information);
    }
#if defined(LOG_DEBUG)
    printf("hw version=0x%x fw version=0x%x OEM Valid:%d sta=%d\r\n", \
        device_information.HardwareVersion, \
        device_information.FirmwareVersion, \
        device_information.OemValid, \
        status);
#endif

    // ptx30wMemoryDump();

    /**
     * A firmware version of zero, indicates that the device is not
     * completely setup --> OEM parameters need to be set.
     */
#if 0
    if ((ptxStatus_Success == status) && (0 == device_information.OemValid)) {
        /**
         * Configure the OEM parameters (this can only be done once).
         */
        ptxOemConfigParam_t ptx30Config;
        status |= ptx30wOemConfig_Init(&ptx30Config);                                           /** Initialize the struct. */
        status |= ptx30wOemConfig_SetBcICharge(&ptx30Config, 200u, 30u);                        /** Set charge current to ~200mA in WPT phase, ~30mA for NFC phase. */
        status |= ptx30wOemConfig_SetBcVTerm(&ptx30Config, ptx30wVTerm_4V18);                   /** Set the termination voltage to 4.18V. */
        status |= ptx30wOemConfig_SetBcVRecharge(&ptx30Config, ptx30wVRecharge_3V87);           /** Set recharge voltage to 3.87V. */
        status |= ptx30wOemConfig_SetBcVTrickle(&ptx30Config, ptx30wVTrickle_3V0);              /** Set trickle charge voltage to 3.0V. */
        status |= ptx30wOemConfig_SetBcITerm(&ptx30Config, 20u);                                /** Set termination current to 20mA. */
        status |= ptx30wOemConfig_SetBcIChargeCold(&ptx30Config, ptx30wIChargePercent_100);     /** Keep 100% charging current for temp. range of 0C < T < +10C. */
        status |= ptx30wOemConfig_SetBcIChargeHot(&ptx30Config, ptx30wIChargePercent_100);      /** Keep 100% charging current for temp. range of +45C < T < +60C. */
        status |= ptx30wOemConfig_SetBcVTermCold(&ptx30Config, ptx30wVTermReduction_None);      /** Do not reduce termination voltage for temp. range of 0C < T < +10C. */
        status |= ptx30wOemConfig_SetBcVTermHot(&ptx30Config, ptx30wVTermReduction_None);       /** Do not reduce termination voltage for temp. range of +45C < T < +60C. */
        status |= ptx30wOemConfig_SetVdMcuMode(&ptx30Config, ptx30wVdMcuMode_Output_3V3);       /** Set VdMcu to 3.3V. */
        status |= ptx30wOemConfig_SetBcLoBatOffEn(&ptx30Config, 1u);                            /** Enable Low Battoff. */
        status |= ptx30wOemConfig_SetBcEnable(&ptx30Config, 1u);                                /** Enable battery charging. */
        status |= ptx30wOemConfig_SetISensorThresh(&ptx30Config, ptx30wIThreshSel_2mA);         /** Set the current threshold to 2mA. */
        status |= ptx30wOemConfig_SetVddCThLow(&ptx30Config, 3600u);							/** Sets the minimum regulation voltage to 3.6V. */
        status |= ptx30wOemConfig_SetVdBatOffsetHigh(&ptx30Config, 600u);                       /** Power regulation VDBAT offset, high (600mV). */
        status |= ptx30wOemConfig_SetVdBatOffsetLow(&ptx30Config, 125u);                        /** Power regulation VDBAT offset, low. (125mV). */
        status |= ptx30wOemConfig_SetI2cAddress(&ptx30Config, 0x4B);                            /** Set I2C Address to 0x4B. */
        status |= ptx30wOemConfig_SetIrqPolarity(&ptx30Config, ptx30wIrqPolarity_ActiveHigh);   /** Configure IRQ line to be active high. */
        status |= ptx30wOemConfig_SetAdjWptDurationInt(&ptx30Config, ptx30wWptDuration_128ms);  /** Set the adjust charging cycle duration to 128ms. */
        status |= ptx30wOemConfig_SetTcmWptDurationInt(&ptx30Config, ptx30wWptDuration_512ms);  /** Set the trickle charge cycle duration to 512ms. */
        status |= ptx30wOemConfig_SetCcmWptDurationInt(&ptx30Config, ptx30wWptDuration_4096ms); /** Set the constant current charge cycle duration to 4s. */
        status |= ptx30wOemConfig_SetCvmWptDurationInt(&ptx30Config, ptx30wWptDuration_2048ms); /** Set the constant voltage charge cycle duration to 2s. */
        status |= ptx30wOemConfig_SetTcmTimeout(&ptx30Config, 0u);                              /** Disable timeout. */
        status |= ptx30wOemConfig_SetCcmTimeout(&ptx30Config, 0u);                              /** Disable timeout. */
        status |= ptx30wOemConfig_SetCvmTimeout(&ptx30Config, 0u);                              /** Disable timeout. */
        status |= ptx30wOemConfig_SetCapWtInt(&ptx30Config, ptx30wWptDuration_16384ms);         /** The Poller will disable its RF-field for this specified duration
                                                                                                        when the listener's battery is fully, before polling again. */
        status |= ptx30wOemConfig_SetGpio0Config(&ptx30Config, ptx30wGpio0Config_StartUpCircuit);/** Configure GPIO0 to control the start-up circuit. */
        status |= ptx30wOemConfig_SetGpio1Config(&ptx30Config, ptx30wGpio1Config_WlcpCtrl);      /** Set GPIO1 to be controlled by the WLC- Poller. */
        status |= ptx30wOemConfig_SetBcUvlo(&ptx30Config, ptx30wBcUvlo_3V0);                     /** Set UVLO to 3.0V. */
        status |= ptx30wOemConfig_SetWptReqSel(&ptx30Config, ptx30wWptReqSel_BcControlled);      /** WPT Request controlled by battery charger. */
        status |= ptx30wOemConfig_SetNfcResistiveMod(&ptx30Config, ptx30wResistorSetting_17Ohm); /** Resistor value of modulator set to 17 Ohm in communication phase during modulation. */
        status |= ptx30wOemConfig_SetNfcResistiveSet(&ptx30Config, ptx30wResistorSetting_27Ohm); /** Resistor value of modulator set to 27 Ohm in communication phase when idle. */
        status |= ptx30wOemConfig_SetWptResistiveMod(&ptx30Config, ptx30wResistorSetting_Disabled);/** Resistor value of modulator set to 'open' in WPT phase during modulation. */
        status |= ptx30wOemConfig_SetWptResistiveSet(&ptx30Config, ptx30wResistorSetting_Disabled);/** Resistor value of modulator set to 'open' in WPT phase when idle. */
        status |= ptx30wOemConfig_SetNtcMode(&ptx30Config, ptx30wNtcMode_Period1ms);              /** Sets the read out interval of the NTC to 1ms. */
        status |= ptx30wOemConfig_SetLimiterVoltage(&ptx30Config, ptx30wLimiterVoltage_4v8);	  /** Set the limiter voltage to 4.8 Volts. This setting defines maximum voltage on VDBUFC.
                                                                                                       Set this value approx. 500mV higher than the termination voltage. */
        /**
         *  An "invalid command" error, indicates that the OEM parameters were already
         *  written to the device.
         */
        if (ptxStatus_Success == status) {
            status = ptx30w_WriteOemParameters(&ptx30Config);
        }
    }
#endif
    ptxParameter_t params[PTX30W_SYS_SET_PARAM_CNT];
    memset(params, 0, sizeof(ptxParameter_t) * PTX30W_SYS_SET_PARAM_CNT);

    params[0].Type = ParamType_ChargerEnable;
    params[0].Value.ChargerEnable = true;

    /* Please pay attention when setting the ChargeCurrent: It must be set equal or lower than the NFC
    charging current from the OEM parameters. */
    params[1].Type = ParamType_ChargeCurrent;
    params[1].Value.ChargeCurrent = 200;

    params[2].Type = ParamType_TerminationVoltage;
    params[2].Value.TerminationVoltage = ptx30wVTerm_4V24;

    /* Please pay attention when setting the RechargeVoltage: It must be at least 300mV lower than the TerminationVoltage. */
    params[3].Type = ParamType_RechargeVoltage;
    params[3].Value.RechargeVoltage = ptx30wVRecharge_3V73;

    params[4].Type = ParamType_TrickleVoltage;
    params[4].Value.TrickleVoltage = ptx30wVTrickle_3V2;

    params[5].Type = ParamType_LimiterVoltage;
    params[5].Value.LimiterVoltage = ptx30wLimiterVoltage_5v0;

    params[6].Type = ParamType_WptDuration;
    params[6].Value.WptDuration = ptx30wWptDuration_1024ms;

    params[7].Type = ParamType_WptRequestSource;
    params[7].Value.WptRequest = ptx30wWptReqSel_BcControlled;

    params[8].Type = ParamType_DetuneEnable;
    params[8].Value.DetuneEnable = false;

    params[9].Type = ParamType_NfcEnable;
    params[9].Value.NfcEnable = true;

    params[10].Type = ParamType_ShippingMode;
    params[10].Value.ShippingMode = false;

    status = ptx30w_SetRuntimeParams(params, PTX30W_SYS_SET_PARAM_CNT);
#if defined(LOG_DEBUG)
    if (status) {
        printf("set RUNTIME para error=%d\n", status);
    } else {
        printf("set RUNTIME para successful\n");
    }
#endif
    // status = ptx30w_set_rt(ParamType_NfcEnable, 1);
    // status = ptx30w_set_charge();

    return status;
}

int ptx30w_set_rt(ptxRuntimeParamType_t type, uint8_t value)
{
    ptxStatus_t status = ptxStatus_Success;
    ptxParameter_t params[PTX30W_RT_PARAM_CNT];
    memset(params, 0, sizeof(ptxParameter_t) * PTX30W_RT_PARAM_CNT);

    switch (type) {
        case ParamType_ChargeCurrent:
            params[0].Type = ParamType_ChargeCurrent;
            params[0].Value.ChargeCurrent = value;
            break;

        case ParamType_TerminationVoltage:
            params[0].Type = ParamType_TerminationVoltage;
            params[0].Value.TerminationVoltage = value;
            break;

        case ParamType_RechargeVoltage:
            params[0].Type = ParamType_RechargeVoltage;
            params[0].Value.RechargeVoltage = value;
            break;

        case ParamType_NfcEnable:
            params[0].Type = ParamType_NfcEnable;
            params[0].Value.NfcEnable = value;
            break;

        case ParamType_ChargerEnable:
            params[0].Type = ParamType_ChargerEnable;
            params[0].Value.ChargerEnable = value;
            break;

        case ParamType_ShippingMode:
            params[0].Type = ParamType_ShippingMode;
            params[0].Value.ShippingMode = value;
            break;

        case ParamType_DetuneEnable:
            params[0].Type = ParamType_DetuneEnable;
            params[0].Value.DetuneEnable = value;
            break;

        default:
            break;
    }

    status = ptx30w_SetRuntimeParams(params, PTX30W_RT_PARAM_CNT);
#if defined(LOG_DEBUG)
    if (!status) {
        printf("ptx30 set RUNTIME para SUCCESS\r\n");
    } else {
        printf("ptx30 set RUNTIME para FAILED, status[%d]\r\n", status);
    }
#endif
    return status;
}

int32_t ptx30w_update_status(void)
{
    ptxStatus_t status = ptxStatus_Success;

    status = ptx30w_GetSystemStatus(&system_status);
    if (ptxStatus_Success != status) {
        printf("ptx30_get_status ERROR[%d]\n", status);
        return -1;
    }

    printf("err[%d] cen[%d] cs[%d] rfd[%d] tmp[%d] vb[%d]mV vd[%d]mV wlcp[0x%02X]\n",   \
            system_status.Error,            \
            system_status.ChargerEnabled,   \
            system_status.ChargerStatus,    \
            system_status.RfFieldDetected,  \
            system_status.NtcStatus,        \
            system_status.VddBat,           \
            system_status.VddC,             \
            system_status.WlcpStatus);  /*  0x00 No WLCP device connected\
                                            0x01 WLCP device connected (WLCP-INFO record written)
                                            0x03 Charging started (1st WLCL-CTL record read) */
    return 0;
}

static ptxStatus_t ptx30w_TDC_HlpRx(uint8_t *rxData, uint32_t *rxDataLen)
{
    ptxStatus_t status = ptxStatus_Success;
    uint8_t frame_buffer[63];
    uint8_t frame_buffer_len = sizeof(frame_buffer);

    status = ptx30w_TDC_Read(frame_buffer, &frame_buffer_len, PTX_HLP_TIMEOUT);
    if (ptxStatus_Success != status) {
        printf("1 ptx30w_TDC_Read: status=%d\n", status);
    }

    // printf("frame_buffer_len[%d]\r\n", frame_buffer_len);
    // printf("frame_buffer[0] == [0x%02X]\r\n", frame_buffer[0]);
    // printf("frame_buffer[4] == [0x%02X]\r\n", frame_buffer[4]);

    if (    (ptxStatus_Success == status)
        &&  (PTX_REQ_FRAME_LEN == frame_buffer_len)
        &&  (PTX_POLL_WRITE_REQ_OPC == frame_buffer[0])
        &&  (PTX_POLL_WRITE_REQ_OPC == frame_buffer[4]))
    {
        uint32_t bytes_to_read = 0;
        /** Assemble the 24-bit length information. */
        bytes_to_read |= ((frame_buffer[1] << 16) & 0xFF0000);
        bytes_to_read |= ((frame_buffer[2] << 8)  & 0x00FF00);
        bytes_to_read |= ((frame_buffer[3] << 0)  & 0x0000FF);

        /**
         * Send our buffer size back to the poller.
         * Not evaluated by the poller YET!
         */
        frame_buffer[1] = (uint8_t) ((*rxDataLen >> 16) & 0xFF);
        frame_buffer[2] = (uint8_t) ((*rxDataLen >> 8)  & 0xFF);
        frame_buffer[3] = (uint8_t) ((*rxDataLen >> 0)  & 0xFF);
        status = ptx30w_TDC_Write(frame_buffer, PTX_REQ_FRAME_LEN, PTX_HLP_TIMEOUT);
        if (ptxStatus_Success != status) {
            printf("HlpRx ptx30w_TDC_Write status=%d\n", status);
        }

        uint32_t rx_data_offset = 0;
        uint32_t pending_len = bytes_to_read;
        /** Calculate the amount of frames to receive. */
        uint32_t frames_to_read = (pending_len / PTX_PAYLOAD_LEN) + 1u;
        /** Read frame by frame in a loop. */
        while( (ptxStatus_Success == status) && (0 != frames_to_read) )
        {
            frame_buffer_len = sizeof(frame_buffer);
            status = ptx30w_TDC_Read(frame_buffer, &frame_buffer_len, PTX_HLP_TIMEOUT);
            if (ptxStatus_Success != status) {
                printf("2 ptx30w_TDC_Read status=%d\n", status);
            }

            /**
             * TDC frames have a maximum length of 63 bytes of payload. In this simple protocol the very first byte
             * of each frame contains the actual frame number (wraps around every 256 frames). So, 62 bytes of 'real'
             * payload are left for usage.
             */
            if(        (ptxStatus_Success == status)
                    && (0 != frame_buffer_len)                                  /**< Check if we received something. */
                    && (frame_buffer[0] == (rx_data_offset/PTX_PAYLOAD_LEN)) )  /**< Validate frame number. */
            {
                /** Minus 1 due to frame number byte. */
                uint8_t actual_payload = (uint8_t) (frame_buffer_len - 1u);
                /** Copy the frame into the correct destination buffer location. */
                memcpy(&rxData[rx_data_offset], &frame_buffer[1], actual_payload);
                /** Update all the helper variables. */
                pending_len -= actual_payload;
                rx_data_offset += actual_payload;
                --frames_to_read;
            }
            else
            {
                status = ptxStatus_ProtocolError;
            }
        }

        if(ptxStatus_Success == status)
        {
            /** Set the number of received bytes. */
            *rxDataLen = bytes_to_read;
        }
    }
    else
    {
        status = ptxStatus_TimeOut;
    }

    if(ptxStatus_Success != status)
    {
        *rxDataLen = 0;
    }

    return status;
}

static ptxStatus_t ptx30w_TDC_HlpTx(uint8_t *txData, uint32_t txDataLen)
{
    ptxStatus_t status = ptxStatus_Success;

    /**
     * Reserve enough memory to store the memory of one TDC frame.
     */
    uint8_t frame_buffer[63];
    uint8_t frame_buffer_len = sizeof(frame_buffer);

    /**
     * Even if we want to write data to the poller we need to wait for the poller
     * to request the data (only applies to *this* implementation approach!)
     */
    status = ptx30w_TDC_Read(frame_buffer, &frame_buffer_len, PTX_HLP_TIMEOUT);
    if (ptxStatus_Success != status) {
        printf("HlpTx ptx30w_TDC_Read=%d\n", status);
    }

    // printf("frame_buffer_len[%d]\r\n", frame_buffer_len);
    // printf("frame_buffer[0] == [0x%02X]\r\n", frame_buffer[0]);
    // printf("frame_buffer[4] == [0x%02X]\r\n", frame_buffer[4]);

    /**
     * Validate the received frame.
     */
    if(    (ptxStatus_Success == status)
        && (PTX_REQ_FRAME_LEN == frame_buffer_len)
        && (PTX_POLL_READ_REQ_OPC  == frame_buffer[0])
        && (PTX_POLL_READ_REQ_OPC  == frame_buffer[4]) )
    {
        /** Check the HLP buffer size of the poller. */
        uint32_t poller_buffer_size = 0;
        /** Retrieve the 24-bit length information. */
        poller_buffer_size |= ((frame_buffer[1] << 16) & 0xFF0000);
        poller_buffer_size |= ((frame_buffer[2] << 8)  & 0x00FF00);
        poller_buffer_size |= ((frame_buffer[3] << 0)  & 0x0000FF);

        if(poller_buffer_size >= txDataLen)
        {
            /**
             * Assemble buffer for the handshake response.
             * Transmit the information of how much data we want to send.
             */
            frame_buffer[0] = PTX_POLL_READ_REQ_OPC;
            /** The listener sends the size information of the amount of data to the poller. */
            frame_buffer[1] = (uint8_t) ((txDataLen >> 16) & 0xFF);
            frame_buffer[2] = (uint8_t) ((txDataLen >> 8)  & 0xFF);
            frame_buffer[3] = (uint8_t) ((txDataLen >> 0)  & 0xFF);
            frame_buffer[4] = PTX_POLL_READ_REQ_OPC;

            /** Transmit the initial frame. Wait a maximum of 35ms for the poller to read the data. */
            status = ptx30w_TDC_Write(frame_buffer, PTX_REQ_FRAME_LEN, PTX_HLP_TIMEOUT);
            if (ptxStatus_Success != status) {
                printf("1 HlpTx ptx30w_TDC_Write=%d\n", status);
            }

            if(ptxStatus_Success == status)
            {
                uint32_t tx_data_offset = 0;
                uint32_t pending_len = txDataLen;
                /** Calculate the amount of frames to receive. */
                uint32_t frames_to_write = (pending_len / PTX_PAYLOAD_LEN) + 1u;
                while ( (ptxStatus_Success == status) && (0 != frames_to_write) )
                {
                    uint8_t payload_len = (uint8_t)(pending_len > PTX_PAYLOAD_LEN ? PTX_PAYLOAD_LEN : pending_len);
                    /** Frame number. */
                    frame_buffer[0] = (uint8_t) (tx_data_offset / PTX_PAYLOAD_LEN);
                    /** Copy payload. */
                    memcpy(&frame_buffer[1], &txData[tx_data_offset], payload_len);
                    /** Write to listener. */
                    status = ptx30w_TDC_Write(frame_buffer, payload_len + 1, PTX_HLP_TIMEOUT);
                    if (ptxStatus_Success != status) {
                        printf("2 HlpTx ptx30w_TDC_Write=%d\n", status);
                    }
                    /** Update all the helper variables. */
                    pending_len -= payload_len;
                    tx_data_offset += payload_len;
                    --frames_to_write;
                }
            }
        }
        else
        {
            status = ptxStatus_InsufficientResources; /**< Buffer size on poller side too small. */
        }
    }

    return status;
}

// static int ptx30w_TDC_parse(uint8_t *buf, uint32_t len, sdu_ctx_t *p_info)
// {
//     if (buf[0] == 0x55 && buf[1] == 0xaa) {
//         p_info->sdu.seq = (buf[3] << 8) | buf[2];
//         p_info->sdu.pdu.type = buf[4];
//         p_info->sdu.pdu.len = (buf[6] << 8) | buf[5];
//         printf("seq[%d] type[0x%x] len[%d]\n", p_info->sdu.seq, p_info->sdu.pdu.type, p_info->sdu.pdu.len);

//         if (len == (p_info->sdu.pdu.len + 7)) {
//             memcpy(p_info->sdu.pdu.data, &buf[7], p_info->sdu.pdu.len);
//         }
//     }

//     return 0;
// }

static int ptx30w_TDC_build(uint8_t *buf, sdu_ctx_t *p_info)
{
    uint32_t len = 0;

    switch (p_info->sdu.pdu.type) {
        case 0x1e:
            /* code */
            break;

        default:
            break;
    }

    return len;
}

void ptx30w_TDC_transfer(void)
{
    uint8_t statu = ptxStatus_Success;
    uint8_t tdc[20] = {0};
    sdu_ctx_t sdu;
    uint32_t len = sizeof(tdc);

    uint8_t build[14] = {0};
    uint32_t build_len = 0;
    int ret = 0;

    statu = ptx30w_TDC_HlpRx(tdc, &len);
    if (ptxStatus_Success != statu) {
        printf("ptx30w_TDC_HlpRx=[%d]\n\n", statu);
    } else {
        printf("tdc read len[%d]: ", len);
//        printf_hex(tdc, len);
#if 0
        ptx30w_TDC_parse(tdc, len, &sdu);
        // build_len = ptx30w_TDC_build(build, sizeof(build), &sdu);
#endif
        statu = ptx30w_TDC_HlpTx(tdc, len);
        if (statu) {
            printf("ptx30w_TDC_HlpTx=[%d]\n", statu);
        } else {
            printf("once loop over\n");
        }
        printf("\n\n");
    }
}
