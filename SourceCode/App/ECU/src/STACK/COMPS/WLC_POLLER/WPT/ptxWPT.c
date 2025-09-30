/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : WPT
    File        : ptxWPT.c

    Description :
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptxStatus.h"
#include "ptxWPT.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptxStatus_t ptxWPT_Init( ptxWPT_t *wpt, struct ptxNSC *nscCtx, pptxNSC_ExtCallBack_t callbackExt, void *ctxExt)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((NULL != wpt) && (NULL != nscCtx) && (NULL != callbackExt) && (NULL != ctxExt))
    {
        wpt->CompId = ptxStatus_Comp_WPT;
        wpt->nscCtx = nscCtx;

        status = ptxNSC_WLC_Init(nscCtx, callbackExt, ctxExt);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WPT, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWPT_UpdateChipConfig( ptxWPT_t *wpt, ptxNSC_WLC_Config_Parameters wlcConfigParams)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != wpt)
    {
        status = ptxNSC_UpdateWLCConfig(wpt->nscCtx, wlcConfigParams);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WPT, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWPT_Start( ptxWPT_t *wpt, ptxNSC_WLC_PowerTransferOptions_t *options)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != wpt)
    {
        status = ptxNSC_WLC_WPTStartCmd(wpt->nscCtx, options);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WPT, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWPT_Stop( ptxWPT_t *wpt, uint8_t *postWptWb)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != wpt)
    {
        /** Let's call the WPT Stop Command. */
        status = ptxNSC_WLC_WPTStopCmd(wpt->nscCtx, postWptWb);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WPT, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWPT_Deinit(ptxWPT_t *wpt)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != wpt)
    {
        status = ptxNSC_WLC_Deinit(wpt->nscCtx);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WPT, ptxStatus_InvalidParameter);
    }

    return status;
}
