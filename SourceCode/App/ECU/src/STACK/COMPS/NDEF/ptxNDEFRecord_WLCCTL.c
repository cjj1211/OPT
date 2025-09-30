/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : NDEF WLC Record
    File        : ptxNDEFRecord_WLCCTL.c

    Description : WLC Listen Control Record according to WLC Version 2.0.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNDEFRecord_WLCCTL.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxWLCCTL_Init (ptxWLCCTL_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        memset(container, 0, sizeof(ptxWLCCTL_t));
    } else
    {
      status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_Create(ptxNDEFRecord_t *record, ptxWLCCTL_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        uint8_t len;
        (void) ptxWLCCTL_GetLength(container, &len);
        status = ptxNDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) TYPE_RTD_WLCCTL, TYPE_RTD_WLCCTL_LEN, NULL, 0, (uint8_t*) container, len);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxWLCCTL_Parse(ptxNDEFRecord_t *record, ptxWLCCTL_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        /* The last byte [ERROR_INFO] of the WLCCTL record is optional */
        if( ((record->Payload.Length == sizeof(ptxWLCCTL_t)) ||
            (record->Payload.Length == sizeof(ptxWLCCTL_t) - MASK_WLCCTL_ERROR_INFO_LEN)) &&
            (0 == strncmp(record->Type.sData, TYPE_RTD_WLCCTL, TYPE_RTD_WLCCTL_LEN)) )
        {
            memset(container, 0, sizeof(ptxWLCCTL_t));
            memcpy(container, record->Payload.Data, record->Payload.Length);
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetLength(ptxWLCCTL_t *container, uint8_t *len)
{
    ptxStatus_t status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);

    if(NULL != len)
    {
        uint8_t hasErrorField;
        uint8_t containerLen = sizeof(ptxWLCCTL_t);

        status = ptxWLCCTL_GetError(container, &hasErrorField);
        if(ptxStatus_Success == status)
        {
            if(!hasErrorField)
            {
                containerLen = (uint8_t)(containerLen - MASK_WLCCTL_ERROR_INFO_LEN);
            }
            *len = containerLen;
        }
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetError(ptxWLCCTL_t *container, uint8_t error)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(error)
        {
            container->STATUS_INFO = (uint8_t)(container->STATUS_INFO | MASK_WLCCTL_STATINF_ERROR);
        } else
        {
            container->STATUS_INFO = (uint8_t)(container->STATUS_INFO & ~MASK_WLCCTL_STATINF_ERROR);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetError(ptxWLCCTL_t *container, uint8_t *error)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != error))
    {
        *error = (container->STATUS_INFO & MASK_WLCCTL_STATINF_ERROR);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetBatteryStatus(ptxWLCCTL_t *container, ptxWLCCTL_BattStat_t battStatus)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO & ~MASK_WLCCTL_STATINF_BATSTAT);
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO | (battStatus << MASK_WLCCTL_STATINF_BATSTAT_POS));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetBatteryStatus(ptxWLCCTL_t *container, ptxWLCCTL_BattStat_t *battStatus)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != battStatus))
    {
        *battStatus = ((container->STATUS_INFO & MASK_WLCCTL_STATINF_BATSTAT) >> MASK_WLCCTL_STATINF_BATSTAT_POS);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetCounter(ptxWLCCTL_t *container, uint8_t counter)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO & ~MASK_WLCCTL_STATINF_CNT);
        container->STATUS_INFO = (uint8_t)(container->STATUS_INFO | (counter & MASK_WLCCTL_STATINF_CNT));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetCounter(ptxWLCCTL_t *container, uint8_t *counter)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != counter))
    {
        *counter = (container->STATUS_INFO & MASK_WLCCTL_STATINF_CNT);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetWptReq(ptxWLCCTL_t *container, uint8_t wptRequest)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(wptRequest)
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG | (0x01 << MASK_WLCCTL_WPTCONF_REQ_POS));
        } else
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG & ~MASK_WLCCTL_WPTCONF_REQ);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetWptReq(ptxWLCCTL_t *container, uint8_t *wptRequest)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != wptRequest))
    {
        *wptRequest = container->WPT_CONFIG & MASK_WLCCTL_WPTCONF_REQ;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetWptDuration(ptxWLCCTL_t *container, uint8_t duration)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG & ~MASK_WLCCTL_WPTCONF_DUR);
        container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG | ((duration << MASK_WLCCTL_WPTCONF_DUR_POS) & MASK_WLCCTL_WPTCONF_DUR));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetWptDuration(ptxWLCCTL_t *container, uint8_t *duration)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != duration))
    {
        *duration = ((container->WPT_CONFIG & MASK_WLCCTL_WPTCONF_DUR) >> MASK_WLCCTL_WPTCONF_DUR_POS);

        if(*duration > MASK_WLCCTL_WPTCONF_DUR_MAX)
        {
            *duration = MASK_WLCCTL_WPTCONF_DUR_MAX;
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetWptDurationMillis(ptxWLCCTL_t *container, uint32_t *wptDurationMs)
{
    ptxStatus_t status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);

    if(NULL != wptDurationMs)
    {
        uint8_t wpt_duration;
        status = ptxWLCCTL_GetWptDuration(container, &wpt_duration);

        if(ptxStatus_Success == status)
        {
            *wptDurationMs = 1u << (wpt_duration + 3u);
        }
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetWptInfoReq(ptxWLCCTL_t *container, uint8_t request)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(request)
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG | MASK_WLCCTL_WPTCONF_INFOREQ);
        } else
        {
            container->WPT_CONFIG = (uint8_t)(container->WPT_CONFIG & ~MASK_WLCCTL_WPTCONF_INFOREQ);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetWptInfoReq(ptxWLCCTL_t *container, uint8_t *request)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != request))
    {
        *request = (container->WPT_CONFIG & MASK_WLCCTL_WPTCONF_INFOREQ);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetPowerAdjust(ptxWLCCTL_t *container, int8_t adjustLevel)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->POWER_ADJ_REQ = adjustLevel;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetPowerAdjustReq(ptxWLCCTL_t *container, int8_t *adjustLevel)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != adjustLevel))
    {
        *adjustLevel = container->POWER_ADJ_REQ;

        if((*adjustLevel > MASK_WLCCTL_PWRADJ_MAX) || (*adjustLevel < MASK_WLCCTL_PWRADJ_MIN))
        {
            *adjustLevel = MASK_WLCCTL_PWRADJ_DEFAULT;
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetBatteryLevel(ptxWLCCTL_t *container, uint8_t battLevel)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->BATTERY_LEVEL = battLevel;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetBatteryLevel(ptxWLCCTL_t *container, uint8_t *battLevel)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != battLevel))
    {
        *battLevel = container->BATTERY_LEVEL;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetDvrSupport(ptxWLCCTL_t *container, ptxWLCCTL_DvrSupport_t support)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->DVR_INFO = (uint8_t)(container->DVR_INFO & ~MASK_WLCCTL_DVRINFO_SUP);
        container->DVR_INFO = (uint8_t)(container->DVR_INFO | (support << MASK_WLCCTL_DVRINFO_SUP_POS));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetDvrSupport(ptxWLCCTL_t *container, ptxWLCCTL_DvrSupport_t *support)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != support))
    {
        *support = ((container->DVR_INFO & MASK_WLCCTL_DVRINFO_SUP) >> MASK_WLCCTL_DVRINFO_SUP_POS);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetDvrInt(ptxWLCCTL_t *container, uint8_t currDvr)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->DVR_INFO = (uint8_t)(container->DVR_INFO & ~MASK_WLCCTL_DVRINFO_INT);
        container->DVR_INFO = (uint8_t)(container->DVR_INFO | (currDvr & MASK_WLCCTL_DVRINFO_INT));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetDvrInt(ptxWLCCTL_t *container, uint8_t *currDvr)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != currDvr))
    {
        *currDvr = (container->DVR_INFO & MASK_WLCCTL_DVRINFO_INT);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetHoldOffWt(ptxWLCCTL_t *container, uint8_t *holdOffWt)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != holdOffWt))
    {
        *holdOffWt = container->HOLD_OFF_WT_INT;

        if(*holdOffWt > MASK_WLCCTL_HOLDOFF_WT_INT_MAX)
        {
            *holdOffWt = MASK_WLCCTL_HOLDOFF_WT_INT_MAX;
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetHoldOffWtMillis(ptxWLCCTL_t *container, uint32_t *holdOffWtMillis)
{
    ptxStatus_t status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);;

    if(NULL != holdOffWtMillis)
    {
        uint8_t hold_off_wt;
        status = ptxWLCCTL_GetHoldOffWt(container, &hold_off_wt);

        if(ptxStatus_Success == status)
        {
            *holdOffWtMillis = (uint32_t) (hold_off_wt << 1u);
        }
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetHoldOffWt(ptxWLCCTL_t *container, uint8_t holdOffWt)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->HOLD_OFF_WT_INT = holdOffWt;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetErrorOvTemp(ptxWLCCTL_t *container, uint8_t ovTemp)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(ovTemp)
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO | MASK_WLCCTL_ERRINFO_TEMP);
        } else
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO & ~MASK_WLCCTL_ERRINFO_TEMP);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetErrorOvTemp(ptxWLCCTL_t *container, uint8_t *ovTemp)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != ovTemp))
    {
        *ovTemp = (container->ERROR_INFO & MASK_WLCCTL_ERRINFO_TEMP);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_SetErrorProt(ptxWLCCTL_t *container, uint8_t protError)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(protError)
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO | MASK_WLCCTL_ERRINFO_PROT_ERR);
        } else
        {
            container->ERROR_INFO = (uint8_t)(container->ERROR_INFO & ~MASK_WLCCTL_ERRINFO_PROT_ERR);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetErrorProt(ptxWLCCTL_t *container, uint8_t *protError)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != protError))
    {
        *protError = (container->ERROR_INFO & MASK_WLCCTL_ERRINFO_PROT_ERR);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCTL_GetErrorPtx(ptxWLCCTL_t *container, ptxWLCCTL_Error_t *err)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != err))
    {
        *err = (container->ERROR_INFO & MASK_WLCCTL_PTX_ERROR_INF_MASK);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}
