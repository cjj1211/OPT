/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : NSC
    File        : ptxNSC_WLC.c

    Description :
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNSC_WLC.h"
#include "ptxNSC.h"
#include "ptxStatus.h"
#include "ptxNSC_RfConfigVal.h"
#include <string.h>

/*
 * ####################################################################################################################
 * INTERNAL TYPES
 * ####################################################################################################################
 */

/**
 * Processing NSC WLC Notifications
 */
static void ptxNSC_WLC_ProcessingExt ( ptxNSC_t *nscCtx, uint8_t *payload, size_t payloadNtf);

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */


ptxStatus_t ptxNSC_WLC_Init( ptxNSC_t *nscCtx, pptxNSC_ExtCallBack_t callbackExt, void *ctxExt)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(nscCtx, ptxStatus_Comp_NSC) && (NULL != callbackExt))
    {
        /*
         * Assign callback and context for calling up WLC Notifications
         */
        nscCtx->ExtensionCb = callbackExt;
        nscCtx->ExtensionCtx = ctxExt;

        /*
         * Assign processing function for Extension Notifications
         */
        nscCtx->ExtensionNtfProcess = (pptxNSC_Process_Ext_NTF_t) &ptxNSC_WLC_ProcessingExt;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NSC, ptxStatus_InvalidParameter);
    }

    status = ptxNSC_CheckSystemState(nscCtx, status);

    return status;
}

ptxStatus_t ptxNSC_WLC_Deinit( ptxNSC_t *nscCtx)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(nscCtx, ptxStatus_Comp_NSC))
    {
        /*
         * Clear callback and Context
         */
        nscCtx->ExtensionCb = NULL;
        nscCtx->ExtensionCtx = NULL;
        nscCtx->ExtensionNtfProcess = NULL;

    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NSC, ptxStatus_InvalidParameter);
    }

    status = ptxNSC_CheckSystemState(nscCtx, status);

    return status;
}

ptxStatus_t ptxNSC_UpdateWLCConfig( struct ptxNSC *nscCtx, ptxNSC_WLC_Config_Parameters wlcConfigParams)
{
    ptxStatus_t status = ptxStatus_Success;

    if PTX_COMP_CHECK(nscCtx, ptxStatus_Comp_NSC)
    {
        uint8_t misc_cfg_len = 255;
        uint8_t misc_cfg[misc_cfg_len];

        status = ptxNSC_GetMiscRFConfig(nscCtx, &misc_cfg[0], &misc_cfg_len);
        if (ptxStatus_Success == status)
        {
            ptxNSC_RfConfigTlv_t rfConfParams;

            /** Following index is link to the position of WLC inside MISC section in Rf Config sheet.
             *  Please, update this in case position of WLC changes in MISC section. */
            uint8_t index_wlc_section = 30u;

            if (NULL != wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel1)
            {
                misc_cfg[index_wlc_section] = *wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel1;
            }
            index_wlc_section++;

            if (NULL != wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel2)
            {
                misc_cfg[index_wlc_section] = *wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel2;
            }
            index_wlc_section++;

            if (NULL != wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel3)
            {
                misc_cfg[index_wlc_section] = *wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel3;
            }
            index_wlc_section++;

            if (NULL != wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel4)
            {
                misc_cfg[index_wlc_section] = *wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel4;
            }
            index_wlc_section++;

            if (NULL != wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel5)
            {
                misc_cfg[index_wlc_section] = *wlcConfigParams.WlcConfigPowerLevel.WbNum_PowLevel5;
            }
            index_wlc_section++;

            if (NULL != wlcConfigParams.WlcAdcConstThld)
            {
                misc_cfg[index_wlc_section] = *wlcConfigParams.WlcAdcConstThld;
            }
            index_wlc_section++;

            if (NULL != wlcConfigParams.WlcPga2Gain)
            {
                misc_cfg[index_wlc_section] = *wlcConfigParams.WlcPga2Gain;
            }

            rfConfParams.ID = RfCfgParam_RegsMisc;
            rfConfParams.Value = misc_cfg;
            rfConfParams.Len = misc_cfg_len;

            status = ptxNSC_RfConfig(nscCtx, &rfConfParams, 1);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NSC, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNSC_WLC_WPTStartCmd(struct ptxNSC *nscCtx, ptxNSC_WLC_PowerTransferOptions_t *options)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(nscCtx, ptxStatus_Comp_NSC) && (NULL != options) && (NULL != options->WbCharging) && (options->WptDuration <= 20u))
    {
        ptxNscHal_BufferId_t bufferId = NscWriteBuffer_1;
        uint8_t *txBuf[1u];
        size_t txLen[1u];

        uint8_t cmd_index = 0;
        uint8_t wpt_start_cmd[PTX_NSC_WLC_START_CMD_LEN];

        /* NSC_WLC_WPT_START_CMD. */
        wpt_start_cmd[cmd_index] = PTX_NSC_WLC_START_CMD_OPCODE;
        cmd_index++;

        wpt_start_cmd[cmd_index] = options->WptDuration;
        cmd_index++;

        uint8_t wpt_options = 0x00;

        if (1u == options->StopRequest)
        {
            wpt_options = 0x01u;
        }

        if (1u == options->Bfod)
        {
            wpt_options |= 0x02u;
        }

        if (NULL != options->WbPostCharging)
        {
            wpt_options |= 0x10u;
        }

        wpt_start_cmd[cmd_index] = wpt_options;
        cmd_index++;

        wpt_start_cmd[cmd_index] = options->Bfod_Gt;
        cmd_index++;

        wpt_start_cmd[cmd_index] = options->RffSyncDuration;
        cmd_index++;

        for(uint8_t i = 0; i < 32u; ++i)
        {
            wpt_start_cmd[cmd_index] = options->WbCharging[i];
            cmd_index++;
        }

        if (NULL != options->WbPostCharging)
        {
            for(uint8_t i = 0; i < 32u; ++i)
            {
                wpt_start_cmd[cmd_index] = options->WbPostCharging[i];
                cmd_index++;
            }
        }

        txBuf[0] = wpt_start_cmd;
        txLen[0] = cmd_index;

        status = ptxNSC_Send(nscCtx, bufferId, txBuf, txLen, 1u);

        if (ptxStatus_Success == status)
        {
            size_t nsc_wpt_start_resp_len = 0;
            uint8_t *nsc_wpt_start_resp = NULL;

            status = ptxNSC_ReceiveRsp(nscCtx, &nsc_wpt_start_resp, &nsc_wpt_start_resp_len, PTX_NSC_TRANSFER_TO);

            if ((ptxStatus_Success == status) && (NULL != nsc_wpt_start_resp) && (nsc_wpt_start_resp_len > 0))
            {
                if ((PTX_NSC_WLC_START_RSP_LEN == nsc_wpt_start_resp_len) && (PTX_NSC_WLC_START_RSP_OPCODE == nsc_wpt_start_resp[0]))
                {
                    status = ptxNSC_ProcessRspErrorCode (nsc_wpt_start_resp[1u]);
                } else
                {
                    status = PTX_STATUS(ptxStatus_Comp_NSC, ptxStatus_NscProtocolError);
                }
            }
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NSC, ptxStatus_InvalidParameter);
    }

    status = ptxNSC_CheckSystemState(nscCtx, status);

    return status;
}

ptxStatus_t ptxNSC_WLC_WPTStopCmd(ptxNSC_t *nscCtx, uint8_t *postWptWb)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(nscCtx, ptxStatus_Comp_NSC))
    {
        ptxNscHal_BufferId_t bufferId = NscWriteBuffer_1;
        uint8_t *txBuf[1u];
        size_t txLen[1u];

        uint8_t cmd_index = 0;
        uint8_t wpt_stop_cmd[PTX_NSC_WLC_STOP_CMD_LEN];

        /* NSC_WLC_WPT_STOP_CMD. */
        wpt_stop_cmd[cmd_index] = PTX_NSC_WLC_STOP_CMD_OPCODE;
        cmd_index++;

        uint8_t wpt_options = 0x00;

        if (NULL != postWptWb)
        {
            wpt_options |= 0x10u;
        }

        wpt_stop_cmd[cmd_index] = wpt_options;
        cmd_index++;

        if (NULL != postWptWb)
        {
            for(uint8_t i = 0; i < 32u; ++i)
            {
                wpt_stop_cmd[cmd_index] = postWptWb[i];
                cmd_index++;
            }
        }

        txBuf[0] = wpt_stop_cmd;
        txLen[0] = cmd_index;

        status = ptxNSC_Send(nscCtx, bufferId, txBuf, txLen, 1u);

        if (ptxStatus_Success == status)
        {
            size_t nsc_wpt_stop_resp_len = 0;
            uint8_t *nsc_wpt_stop_resp = NULL;

            status = ptxNSC_ReceiveRsp(nscCtx, &nsc_wpt_stop_resp, &nsc_wpt_stop_resp_len, PTX_NSC_TRANSFER_TO);

            if ((ptxStatus_Success == status) && (NULL != nsc_wpt_stop_resp) && (nsc_wpt_stop_resp_len > 0))
            {
                if ((PTX_NSC_WLC_STOP_RSP_LEN == nsc_wpt_stop_resp_len) && (PTX_NSC_WLC_STOP_RSP_OPCODE == nsc_wpt_stop_resp[0]))
                {
                    status = ptxNSC_ProcessRspErrorCode (nsc_wpt_stop_resp[1u]);
                } else
                {
                    status = PTX_STATUS(ptxStatus_Comp_NSC, ptxStatus_NscProtocolError);
                }
            }
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NSC, ptxStatus_InvalidParameter);
    }

    status = ptxNSC_CheckSystemState(nscCtx, status);

    return status;
}


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

static void ptxNSC_WLC_ProcessingExt ( ptxNSC_t *nscCtx, uint8_t *payload, size_t payloadNtf)
{
    if (PTX_COMP_CHECK(nscCtx, ptxStatus_Comp_NSC) && (payload != NULL) && (payloadNtf > 0))
    {
        ptxNSC_WLC_Events_t event;

        switch(payload[0])
        {
            case PTX_NSC_WLC_WPT_FINISHED_NTF_OPCODE:

                switch (payload[1u])
                {
                    case PTX_NSC_WLC_WPT_NTF_OP_WPT_FINISHED:
                        event = PowerTransfer_OK;
                        (void)nscCtx->ExtensionCb(nscCtx->ExtensionCtx, &event, NULL, 0);
                        break;

                    case PTX_NSC_WLC_WPT_NTF_OP_WPT_FOD:
                        event = PowerTransfer_FoD;
                        (void)nscCtx->ExtensionCb(nscCtx->ExtensionCtx, &event, NULL, 0);
                        break;

                    case PTX_NSC_WLC_WPT_NTF_OP_WPT_STOPREQ:
                        event = PowerTransfer_WPTStop;
                        (void)nscCtx->ExtensionCb(nscCtx->ExtensionCtx, &event, NULL, 0);
                        break;

                    case PTX_NSC_WLC_WPT_NTF_OP_WPT_POWUP:
                    case PTX_NSC_WLC_WPT_NTF_OP_WPT_POWDOWN:
                        /* unsupported event - ignore */
                        break;

                    default:
                        /* unsupported event - ignore */
                        break;
                }
                break;

            default:
                /* unsupported opcode (not related to WLC) - ignore */
                break;
        }
    }
}


