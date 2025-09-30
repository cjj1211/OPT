/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : WLCN_PERIPHERALS
    File        : ptxWLCNTempSensor.c

    Description :
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptxPERIPH_I2C.h"
#include "ptxWLCNTempSensor.h"
#include <string.h>

/*
 * ####################################################################################################################
 * API FUNCTIONS WLCN PERIPHERALS
 * ####################################################################################################################
 */

ptxStatus_t ptxWLCN_TempSensor_Init(ptxWLCN_TempSensor_t *sensor, uint16_t i2cAddress)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != sensor)
    {
        /** Default configuration for temperature register.  */
        /** Set pointer register to temperature register.  */
        status = ptxPERIPH_I2C_SetSlaveAddress(i2cAddress);
        if (ptxStatus_Success == status)
        {
            uint8_t temp_register = (uint8_t)PTX_WLCN_TEMP_SENSOR_TEMP_REGISTER;
            status = ptxPERIPH_I2C_TRx(&temp_register, 1u, 0, NULL, 0);

            if (ptxStatus_Success == status)
            {
                sensor->CompId = ptxStatus_Comp_WLCN_PERIPH;
                sensor->BusAddress = i2cAddress;
            }
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxWLCN_TempSensor_ReadTemp(ptxWLCN_TempSensor_t *sensor, int16_t *tempValue)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(sensor, ptxStatus_Comp_WLCN_PERIPH) && (NULL != tempValue))
    {
        /** Let's read the temperature register.  */
        uint8_t temp_value[2u];

        status = ptxPERIPH_I2C_SetSlaveAddress(sensor->BusAddress);
        if (ptxStatus_Success == status)
        {
            status = ptxPERIPH_I2C_TRx(NULL, 0, 0, &temp_value[0], 2u);

            if (ptxStatus_Success == status)
            {
                volatile uint16_t temp_value_long;
                const uint16_t sign_mask = 0x100;
                const uint16_t step_degree = 2u;

                temp_value_long = (uint16_t)(((uint16_t)(temp_value[0] << 8u)) + (uint16_t)temp_value[1]);
                temp_value_long = temp_value_long >> 7u;

                if (temp_value_long >= sign_mask)
                {
                    /** Negative value */

                    /** Let's remove the sign bit, and get complement two value */
                    temp_value_long = temp_value_long | sign_mask;
                    temp_value_long = (uint16_t)(sign_mask - temp_value_long);
                    *tempValue = (int16_t) ((-1) * (temp_value_long / step_degree));

                } else
                {
                    /** Positive value */
                    *tempValue = (int16_t)(temp_value_long / step_degree);
                }

            }
        }

    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxWLCN_TempSensor_Deinit(ptxWLCN_TempSensor_t *sensor)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != sensor)
    {
        (void) memset(sensor, 0, sizeof(ptxWLCN_TempSensor_t));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}
