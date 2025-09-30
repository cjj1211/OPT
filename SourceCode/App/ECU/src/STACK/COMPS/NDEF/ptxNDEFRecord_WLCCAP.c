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
    File        : ptxNDEFRecord_WLCCAP.c

    Description : WLC Capability Record implementation according to WLC Version 2.0.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNDEFRecord_WLCCAP.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxWLCCAP_Init (ptxWLCCAP_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        memset(container, 0, sizeof(ptxWLCCAP_t));
    } else
    {
      status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_Create (ptxNDEFRecord_t *record, ptxWLCCAP_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        status = ptxNDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) TYPE_RTD_WLCCAP, TYPE_RTD_WLCCAP_LEN, NULL, 0u, (uint8_t*) container, sizeof(ptxWLCCAP_t));
    } else
    {
      status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_Parse (ptxNDEFRecord_t *record, ptxWLCCAP_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        if( (record->Payload.Length >= sizeof(ptxWLCCAP_t)) &&
            (0 == strncmp(record->Type.sData, TYPE_RTD_WLCCAP, TYPE_RTD_WLCCAP_LEN)) )
        {
            memcpy(container, record->Payload.Data, sizeof(ptxWLCCAP_t));
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

ptxStatus_t ptxWLCCAP_SetProtVersion (ptxWLCCAP_t *container, uint8_t majorVersion, uint8_t minorVersion)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->WLC_PROTOCOL_VER  = (uint8_t) (((majorVersion << MASK_WLCCAP_PROTVERSION_MAJOR_POS) & MASK_WLCCAP_PROTVERSION_MAJOR)
                                               |  ((minorVersion << MASK_WLCCAP_PROTVERSION_MINOR_POS) & MASK_WLCCAP_PROTVERSION_MINOR));
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetProtVersion (ptxWLCCAP_t *container, uint8_t *majorVersion, uint8_t *minorVersion)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != majorVersion) && (NULL != minorVersion))
    {
        *majorVersion = (uint8_t) ((container->WLC_PROTOCOL_VER >> MASK_WLCCAP_PROTVERSION_MAJOR_POS) & 0x0F);
        *minorVersion = (uint8_t) ((container->WLC_PROTOCOL_VER >> MASK_WLCCAP_PROTVERSION_MINOR_POS) & 0x0F);
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;

}

ptxStatus_t ptxWLCCAP_SetMode (ptxWLCCAP_t *container, ptxWLCMode_t mode)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        /** Clear current setting */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~MASK_WLCCAP_CONF_MODEREQ);
        /** Set new value */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | ((mode << MASK_WLCCAP_CONF_MODEREQ_POS) & MASK_WLCCAP_CONF_MODEREQ));
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetMode (ptxWLCCAP_t *container, ptxWLCMode_t *mode)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != mode))
    {
        *mode = ((container->WLC_CONFIG & MASK_WLCCAP_CONF_MODEREQ) >> MASK_WLCCAP_CONF_MODEREQ_POS);
        if(ptxWLCMode_RFU == *mode)
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_ProtocolError);
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_SetNWtRetries (ptxWLCCAP_t *container, uint8_t retries)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        /** Clear current setting */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~MASK_WLCCAP_CONF_NWTRET);
        /** Set new value */
        container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | ((retries << MASK_WLCCAP_CONF_NWTRET_POS) & MASK_WLCCAP_CONF_NWTRET));
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetNWtRetries (ptxWLCCAP_t *container, uint8_t *retries)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != retries))
    {
        *retries = ((container->WLC_CONFIG & MASK_WLCCAP_CONF_NWTRET) >> MASK_WLCCAP_CONF_NWTRET_POS);
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_SetNegWait (ptxWLCCAP_t *container, uint8_t negoWait)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(negoWait)
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | MASK_WLCCAP_CONF_NEGWAIT);
        } else
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~MASK_WLCCAP_CONF_NEGWAIT);
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetNegWait (ptxWLCCAP_t *container, uint8_t *negoWait)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != negoWait))
    {
        *negoWait = (0 != (container->WLC_CONFIG & MASK_WLCCAP_CONF_NEGWAIT) ? 1 : 0);
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_SetRdConf (ptxWLCCAP_t *container, uint8_t rdConf)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        if(rdConf)
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG | MASK_WLCCAP_CONF_RDCONF);
        } else
        {
            container->WLC_CONFIG = (uint8_t)(container->WLC_CONFIG & ~MASK_WLCCAP_CONF_RDCONF);
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetRdConf (ptxWLCCAP_t *container, uint8_t *rdConf)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != rdConf))
    {
        *rdConf = container->WLC_CONFIG & MASK_WLCCAP_CONF_RDCONF;
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_SetCapWt (ptxWLCCAP_t *container, uint8_t capWt)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        /** Clear current setting */
        container->CAP_WT_INT = (uint8_t)(container->CAP_WT_INT & ~MASK_WLCCAP_CAPWT_INT);
        /** Set new value */
        container->CAP_WT_INT = (uint8_t)(container->CAP_WT_INT | ((capWt << MASK_WLCCAP_CAPWT_INT_POS) & MASK_WLCCAP_CAPWT_INT));
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetCapWt (ptxWLCCAP_t *container, uint8_t *capWt)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != capWt))
    {
        *capWt = ((container->CAP_WT_INT & MASK_WLCCAP_CAPWT_INT) >> MASK_WLCCAP_CAPWT_INT_POS);

        if(*capWt > MASK_WLCCAP_CAPWT_INT_MAX)
        {
            *capWt = MASK_WLCCAP_CAPWT_INT_MAX;
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetCapWtMillis (ptxWLCCAP_t *container, uint32_t *capWtMillis)
{
    ptxStatus_t status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);

    if(NULL != capWtMillis)
    {
        uint8_t cap_wt;
        status = ptxWLCCAP_GetCapWt(container, &cap_wt);

        if(ptxStatus_Success == status)
        {
            *capWtMillis = (1u << (cap_wt + 3u));
        }
    }

    return status;
}

ptxStatus_t ptxWLCCAP_SetRdWt (ptxWLCCAP_t *container, uint8_t rdWT)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        /** Clear current setting */
        container->NDEF_RD_WT = (uint8_t)(container->NDEF_RD_WT & ~MASK_WLCCAP_RDWT_INT);
        /** Set new value */
        container->NDEF_RD_WT = (uint8_t)(container->NDEF_RD_WT | ((rdWT << MASK_WLCCAP_RDWT_INT_POS) & MASK_WLCCAP_RDWT_INT));
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetRdWt (ptxWLCCAP_t *container, uint8_t *rdWT)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != rdWT))
    {
        *rdWT = ((container->NDEF_RD_WT & MASK_WLCCAP_RDWT_INT) >> MASK_WLCCAP_RDWT_INT_POS);
        if(*rdWT > MASK_WLCCAP_RDWT_INT_MAX)
        {
            *rdWT = MASK_WLCCAP_RDWT_INT_MAX;
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_SetWrToInt (ptxWLCCAP_t *container, uint8_t timeout)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        /** Clear current setting */
        container->NDEF_WR_TO_INT = (uint8_t)(container->NDEF_WR_TO_INT & ~MASK_WLCCAP_WRTOINT_INT);
        /** Set new value */
        container->NDEF_WR_TO_INT = (uint8_t)(container->NDEF_WR_TO_INT | ((timeout << MASK_WLCCAP_WRTOINT_INT_POS) & MASK_WLCCAP_WRTOINT_INT));
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetWrToInt (ptxWLCCAP_t *container, uint8_t *timeout)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != timeout))
    {
        *timeout = ((container->NDEF_WR_TO_INT & MASK_WLCCAP_WRTOINT_INT) >> MASK_WLCCAP_WRTOINT_INT_POS);
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_SetWrWtInt (ptxWLCCAP_t *container, uint8_t waitingTime)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        /** Clear current setting */
        container->NDEF_WR_WT_INT = (uint8_t)(container->NDEF_WR_WT_INT & ~MASK_WLCCAP_WRWTINT_INT);
        /** Set new value */
        container->NDEF_WR_WT_INT = (uint8_t)(container->NDEF_WR_WT_INT | ((waitingTime << MASK_WLCCAP_WRWTINT_INT_POS) & MASK_WLCCAP_WRWTINT_INT));
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCCAP_GetWrWtInt (ptxWLCCAP_t *container, uint8_t *waitingTime)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != waitingTime))
    {
        *waitingTime = ((container->NDEF_WR_WT_INT & MASK_WLCCAP_WRWTINT_INT) >> MASK_WLCCAP_WRWTINT_INT_POS);
        if(*waitingTime > MASK_WLCCAP_WRWTINT_INT_MAX)
        {
            *waitingTime = MASK_WLCCAP_WRWTINT_INT_MAX;
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}
