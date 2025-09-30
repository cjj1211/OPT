/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130
    Module      : PERIPHERALS
    File        : ptxPERIPH_DCDC.c

    Description :

 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxPERIPH_DCDC.h"
#include <string.h>

#define PTX_DCDC_ENABLE_PIN             BSP_IO_PORT_00_PIN_13;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxPERIPH_DCDC_Init(ptxPERIPH_DCDC_t *dcdc)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != dcdc)
    {
        /** Let's clear DCDC context. */
        (void)memset(dcdc, 0, sizeof(ptxPERIPH_DCDC_t));

        /** Let's assign driver instances. */
        dcdc->ioport_instance = &g_ioport;
        dcdc->dac_instance = &g_dac8_1;

        /** Let's assign input parameters. */
        dcdc->DCDC_pin = PTX_DCDC_ENABLE_PIN;

        /** Let's initialialize DCCD pin. */
        status = ptxPERIPH_DCDC_EnableDCDC(dcdc);

        if (ptxStatus_Success == status)
        {
            /** Let's initialialize DAC pin. */
            status = ptxPERIPH_DCDC_InitDAC(dcdc);

            if (ptxStatus_Success == status)
            {
                /** Let's set DAC to 0 to have default output voltage of DCDC. */
                status = ptxPERIPH_DCDC_SetDAC(dcdc, 0);

                /** 1ms delay for system boot up. */
                uint32_t delay = 1;
                R_BSP_SoftwareDelay(delay, BSP_DELAY_UNITS_MILLISECONDS);
            }
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxPERIPH_DCDC_Deinit(ptxPERIPH_DCDC_t *dcdc)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != dcdc)
    {
        if (dcdc->DCDC_pin != 0)
        {
            /** Let's disable DCCD pin. */
            status = ptxPERIPH_DCDC_DisableDCDC(dcdc);

            /** Let's deinit DCCD pin. */
            status = ptxPERIPH_DCDC_DeinitDAC(dcdc);

            /** Let's clear poller board context. */
            (void)memset(dcdc, 0, sizeof(ptxPERIPH_DCDC_t));
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxPERIPH_DCDC_EnableDCDC(ptxPERIPH_DCDC_t *dcdc)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != dcdc)
    {
        R_IOPORT_PinWrite(dcdc->ioport_instance->p_ctrl, dcdc->DCDC_pin, BSP_IO_LEVEL_HIGH);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxPERIPH_DCDC_DisableDCDC(ptxPERIPH_DCDC_t *dcdc)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != dcdc)
    {
        R_IOPORT_PinWrite(dcdc->ioport_instance->p_ctrl, dcdc->DCDC_pin, BSP_IO_LEVEL_LOW);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxPERIPH_DCDC_InitDAC(ptxPERIPH_DCDC_t *dcdc)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != dcdc)
    {
        dcdc->dac_instance->p_api->open(dcdc->dac_instance->p_ctrl, dcdc->dac_instance->p_cfg );
        dcdc->dac_instance->p_api->start(dcdc->dac_instance->p_ctrl);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxPERIPH_DCDC_DeinitDAC(ptxPERIPH_DCDC_t *dcdc)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != dcdc)
    {
        dcdc->dac_instance->p_api->stop(dcdc->dac_instance->p_ctrl);
        dcdc->dac_instance->p_api->close(dcdc->dac_instance->p_ctrl);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxPERIPH_DCDC_SetDAC(ptxPERIPH_DCDC_t *dcdc, uint16_t value)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != dcdc)
    {
#ifdef _RENESAS_RA_
        R_DAC_Write(g_dac8_1.p_ctrl, value);
#else
        dcdc->dac_instance->p_api->write(g_dac8_1.p_ctrl, value);
#endif
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


