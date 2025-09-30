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
    File        : ptxNDEFRecord_WLCINF.c

    Description : WLC Poll Information Record according to WLC Version 2.0
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNDEFRecord_WLCINF.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxWLCINF_Init (ptxWLCINF_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        memset(container, 0, sizeof(ptxWLCINF_t));
    } else
    {
      status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_Create(ptxNDEFRecord_t *record, ptxWLCINF_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        status = ptxNDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) TYPE_RTD_WLCINF, TYPE_RTD_WLCINF_LEN, NULL, 0u, (uint8_t*) container, sizeof(ptxWLCINF_t));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_Parse(ptxNDEFRecord_t *record, ptxWLCINF_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        if( (record->Payload.Length == sizeof(ptxWLCINF_t)) &&
            (0 == strncmp(record->Type.sData, TYPE_RTD_WLCINF, TYPE_RTD_WLCINF_LEN)) )
        {
            memcpy(container, record->Payload.Data, sizeof(ptxWLCINF_t));
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

ptxStatus_t ptxWLCINF_SetPTX(ptxWLCINF_t *container, uint8_t percent)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->P_TX = percent;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_GetPTX(ptxWLCINF_t *container, uint8_t *percent)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != percent))
    {
        *percent = container->P_TX;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_SetWptStopSupport(ptxWLCINF_t *container, uint8_t wptStopSupport)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(wptStopSupport)
        {
            container->WLC_PCAP_POWER = (uint8_t)(container->WLC_PCAP_POWER | MASK_WLCINF_WLCPCAP_STOPSUP);
        } else
        {
            container->WLC_PCAP_POWER = (uint8_t)(container->WLC_PCAP_POWER & ~MASK_WLCINF_WLCPCAP_STOPSUP);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_GetWptStopSupport(ptxWLCINF_t *container, uint8_t *wptStopSupport)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != wptStopSupport))
    {
        *wptStopSupport = container->WLC_PCAP_POWER & MASK_WLCINF_WLCPCAP_STOPSUP;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_SetPowerClass(ptxWLCINF_t *container, ptxWLCINF_PowerClass_t powerClass)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->WLC_PCAP_POWER = (uint8_t)(container->WLC_PCAP_POWER & ~MASK_WLCINF_PCLASS);
        container->WLC_PCAP_POWER = powerClass;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_GetPowerClass(ptxWLCINF_t *container, ptxWLCINF_PowerClass_t *powerClass)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != powerClass))
    {
        *powerClass = (container->WLC_PCAP_POWER & MASK_WLCINF_PCLASS);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_SetTotPowerSteps(ptxWLCINF_t *container, uint8_t steps)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->TOT_POWER_STEPS = steps;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_GetTotPowerSteps(ptxWLCINF_t *container, uint8_t *steps)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != steps))
    {
        *steps = container->TOT_POWER_STEPS;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_SetCurPowerStep(ptxWLCINF_t *container, uint8_t steps)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CUR_POWER_STEP = steps;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_GetCurPowerStep(ptxWLCINF_t *container, uint8_t *steps)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != steps))
    {
        *steps = container->CUR_POWER_STEP;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_SetNextMinStepInc(ptxWLCINF_t *container, int8_t stepSize)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->NEXT_MIN_STEP_DEC = stepSize;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_GetNextMinStepInc(ptxWLCINF_t *container, int8_t *stepSize)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != stepSize))
    {
        *stepSize = container->NEXT_MIN_STEP_DEC;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_SetNextMinStepDec(ptxWLCINF_t *container, int8_t stepSize)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->NEXT_MIN_STEP_DEC = stepSize;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCINF_GetNextMinStepDec(ptxWLCINF_t *container, int8_t *stepSize)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != stepSize))
    {
        *stepSize = container->NEXT_MIN_STEP_DEC;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}
