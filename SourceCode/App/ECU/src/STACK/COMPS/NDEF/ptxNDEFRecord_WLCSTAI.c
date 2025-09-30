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
    File        : ptxNDEFRecord_WLCSTAI.c

    Description : WLC Status and Info Record implementation according to WLC Version 2.0.
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNDEFRecord_WLCSTAI.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxWLCSTAI_Init (ptxWLCSTAI_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        memset(container, 0, sizeof(ptxWLCSTAI_t));
    } else
    {
      status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_Create(ptxNDEFRecord_t *record, ptxWLCSTAI_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        status = ptxNDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) TYPE_RTD_WLCSTAI, TYPE_RTD_WLCSTAI_LEN, NULL, 0u, NULL, 0);

        uint8_t *payload_cpy = record->Payload.Data;
        *payload_cpy = container->CONTROL;
        payload_cpy++;

        uint8_t mask = 0x01;
        uint8_t *container_cpy = &(container->BATTERY_LEVEL);
        for(uint8_t i = 0; i < 8; ++i)
        {
            if((container->CONTROL & mask))
            {
                *payload_cpy = *container_cpy;
                payload_cpy++;
            }
            container_cpy++;
            mask = ((uint8_t) (mask << 1));
        }

        if(0 != container->CONTROL2)
        {
            mask = 0x01;
            container_cpy = &(container->BATTERY_VOLTAGE);
            for(uint8_t i = 0; i < 2; ++i)
            {
                if((container->CONTROL2 & mask))
                {
                    *payload_cpy = *container_cpy;
                    payload_cpy++;
                }
                container_cpy++;
                mask = ((uint8_t) (mask << 1));
            }
        }

        record->Payload.Length = (uint32_t) (payload_cpy - record->Payload.Data);
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_Parse(ptxNDEFRecord_t *record, ptxWLCSTAI_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != record) && (NULL != container))
    {
        if( (record->Payload.Length > 1u) &&
            (record->Payload.Length < sizeof(ptxWLCSTAI_t)) &&
            (0 == strncmp(record->Type.sData, TYPE_RTD_WLCSTAI, TYPE_RTD_WLCSTAI_LEN)))
        {
            memset(container, 0, sizeof(ptxWLCSTAI_t));

            uint8_t *payload_cpy = record->Payload.Data;
            container->CONTROL = *payload_cpy;
            payload_cpy++;

            uint8_t *container_cpy = &(container->BATTERY_LEVEL);

            uint8_t mask = 0x01;
            for(uint8_t i = 0; i < 8; ++i)
            {
                if((container->CONTROL & mask))
                {
                    *container_cpy = *payload_cpy;
                    payload_cpy++;
                }
                container_cpy++;
                mask = ((uint8_t) (mask << 1));
            }

            if(0 != container->CONTROL2)
            {
                mask = 0x01;
                container_cpy = &(container->BATTERY_VOLTAGE);
                for(uint8_t i = 0; i < 2; ++i)
                {
                    if((container->CONTROL2 & mask))
                    {
                        *payload_cpy = *container_cpy;
                        payload_cpy++;
                    }
                    container_cpy++;
                    mask = ((uint8_t) (mask << 1));
                }
            }

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

ptxStatus_t ptxWLCSTAI_SetBatteryLevel(ptxWLCSTAI_t *container, uint8_t percent)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_BATLVL);
        container->BATTERY_LEVEL = percent;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_GetBatteryLevel(ptxWLCSTAI_t *container, uint8_t *percent)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != percent))
    {
        if(container->CONTROL & MASK_WLCSTAI_CTRL1_BATLVL)
        {
            *percent = container->BATTERY_LEVEL;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_SetReceivePower(ptxWLCSTAI_t *container, uint8_t power)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_RECPWR);
        container->RECEIVE_POWER = power;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_GetReceivePower(ptxWLCSTAI_t *container, uint8_t *power)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != power))
    {
        if(container->CONTROL & MASK_WLCSTAI_CTRL1_RECPWR)
        {
            *power = container->RECEIVE_POWER;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_SetReceiveVoltage(ptxWLCSTAI_t *container, uint8_t voltage)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_RECVLT);
        container->RECEIVE_VOLTAGE = voltage;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_GetReceiveVoltage(ptxWLCSTAI_t *container, uint8_t *voltage)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != voltage))
    {
        if(container->CONTROL & MASK_WLCSTAI_CTRL1_RECVLT)
        {
            *voltage = container->RECEIVE_VOLTAGE;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_SetReceiveCurrent(ptxWLCSTAI_t *container, uint8_t current)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_RECCUR);
        container->RECEIVE_CURRENT = current;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_GetReceiveCurrent(ptxWLCSTAI_t *container, uint8_t *current)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != current))
    {
        if(container->CONTROL & MASK_WLCSTAI_CTRL1_RECCUR)
        {
            *current = container->RECEIVE_CURRENT;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_SetTemperatureBat(ptxWLCSTAI_t *container, int8_t celsius)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_TEMPBAT);
        container->TEMPERATURE_BATTERY = celsius;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_GetTemperatureBat(ptxWLCSTAI_t *container, int8_t *celsius)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != celsius))
    {
        if(container->CONTROL & MASK_WLCSTAI_CTRL1_TEMPBAT)
        {
            *celsius = container->TEMPERATURE_BATTERY;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_SetTemperatureWlcl(ptxWLCSTAI_t *container, int8_t celsius)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_TEMPWLCL);
        container->TEMPERATURE_WLCL = celsius;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_GetTemperatureWlcl(ptxWLCSTAI_t *container, int8_t *celsius)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != celsius))
    {
        if(container->CONTROL & MASK_WLCSTAI_CTRL1_TEMPWLCL)
        {
            *celsius = container->TEMPERATURE_WLCL;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCSTAI_SetBatteryVoltage(ptxWLCSTAI_t *container, uint8_t voltage)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_CTRL2);
        container->CONTROL2 = (uint8_t)(container->CONTROL2 | MASK_WLCSTAI_CTRL2_BATVLT);
        container->BATTERY_VOLTAGE = voltage;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }
    return status;
}

ptxStatus_t ptxWLCSTAI_GetBatteryVoltage(ptxWLCSTAI_t *container, uint8_t *voltage)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != voltage))
    {
        if( (container->CONTROL & MASK_WLCSTAI_CTRL1_CTRL2) &&
            (container->CONTROL2 & MASK_WLCSTAI_CTRL2_BATVLT))
        {
            *voltage = container->BATTERY_VOLTAGE;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }
    return status;
}

ptxStatus_t ptxWLCSTAI_SetBatteryCurrent(ptxWLCSTAI_t *container, uint8_t current)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        container->CONTROL = (uint8_t)(container->CONTROL | MASK_WLCSTAI_CTRL1_CTRL2);
        container->CONTROL2 = (uint8_t)(container->CONTROL2 | MASK_WLCSTAI_CTRL2_BATCUR);
        container->BATTERY_CURRENT = current;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }
    return status;
}

ptxStatus_t ptxWLCSTAI_GetBatteryCurrent(ptxWLCSTAI_t *container, uint8_t *current)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != current))
    {
        if( (container->CONTROL & MASK_WLCSTAI_CTRL1_CTRL2) &&
            (container->CONTROL2 & MASK_WLCSTAI_CTRL2_BATCUR))
        {
            *current = container->BATTERY_CURRENT;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_NoDataAvailable);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}
