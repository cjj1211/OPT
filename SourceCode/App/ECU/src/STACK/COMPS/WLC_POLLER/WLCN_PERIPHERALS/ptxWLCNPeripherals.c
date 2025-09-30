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
    File        : ptxWLCNPeripherals.c

    Description :
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxStatus.h"
#include "ptxPLAT.h"
#include "ptxPERIPH_I2C.h"
#include "ptxWLCNPeripherals.h"
#include <string.h>

/*
 * ####################################################################################################################
 * STATIC FUNCTIONS
 * ####################################################################################################################
 */

/*
 * ####################################################################################################################
 * API FUNCTIONS WLCN PERIPHERALS
 * ####################################################################################################################
 */

ptxStatus_t ptxWLCN_Peripherals_Init(ptxWLCN_Peripherals_t *WLCNPeriph, ptxWLCN_Peripherals_InitParams_t initPars)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != WLCNPeriph)
    {
        /** Let's clear WLCNPeripherals. */
        (void)memset(WLCNPeriph, 0, sizeof(ptxWLCN_Peripherals_t));

        status = ptxPERIPH_I2C_Init();

        if (ptxStatus_Success == status)
        {
            /** Let's initialize power sensor. */
            status = ptxWLCN_PowerSensor_Init(&WLCNPeriph->PwrSensor, POLLER_POWER_SENSOR_I2C_ADDRESS);

            if (ptxStatus_Success == status)
            {
                /** Let's initialize DCDC set up. */
                status = ptxPERIPH_DCDC_Init(&WLCNPeriph->Dcdc);

                if (ptxStatus_Success == status)
                {
                    /** Assign Component-ID. */
                    WLCNPeriph->CompId = ptxStatus_Comp_WLCN_PERIPH;

                    /** Let's calibrate the DCDC output voltage. */
                    status = ptxWLCN_Peripherals_Calibration(WLCNPeriph, initPars.DcdcVoltage);

                    if (ptxStatus_Success != status)
                    {
                        /** Clear WLCNPeripherals in case of error. */
                        (void)memset(WLCNPeriph, 0, sizeof(ptxWLCN_Peripherals_t));
                    }
                }
            }
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_Peripherals_Deinit(ptxWLCN_Peripherals_t *WLCNPeriph)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(WLCNPeriph, ptxStatus_Comp_WLCN_PERIPH))
    {
        (void) ptxWLCN_PowerSensor_DeInit(&WLCNPeriph->PwrSensor);
        (void) ptxPERIPH_DCDC_Deinit(&WLCNPeriph->Dcdc);
        (void) ptxPERIPH_I2C_Deinit();
        (void) memset(WLCNPeriph, 0, sizeof(ptxWLCN_Peripherals_t));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_Peripherals_Calibration(ptxWLCN_Peripherals_t *WLCNPeriph, uint16_t dcdcVoltageTarget)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((PTX_COMP_CHECK(WLCNPeriph, ptxStatus_Comp_WLCN_PERIPH)) &&
            ( (dcdcVoltageTarget >= PTX_PERIPH_DCDC_VOLTAGE_MIN_VAL) && (dcdcVoltageTarget <= PTX_PERIPH_DCDC_VOLTAGE_MAX_VAL)))
    {
        uint16_t dac_value;

        /** First step to set linear value on DAC. */
        dac_value = (uint16_t)(PTX_PERIPH_DCDC_VOLTAGE_DAC(dcdcVoltageTarget) / PTX_PERIPH_DCDC_VOLTAGE_UV_PER_STEP);

        status = ptxPERIPH_DCDC_SetDAC(&WLCNPeriph->Dcdc, dac_value);

        /** Wait time given to DCDC to set value. */
        uint32_t sleep_ms = 10;
        R_BSP_SoftwareDelay(sleep_ms, BSP_DELAY_UNITS_MILLISECONDS);

        if (ptxStatus_Success == status)
        {
            /** Let's get the Voltage out of Power Sensor. */
            uint16_t dcdc_voltage_measure = 0;
            uint8_t dcdc_voltage_valid = 0;
            uint8_t dcdc_regaulate_attempts = 0;
            const uint8_t max_dcdc_regaulate_attempts = 5;

            do
            {
                status = ptxWLCN_Peripherals_PowerSensor_GetVoltage(WLCNPeriph , &dcdc_voltage_measure);

                if (ptxStatus_Success == status)
                {
                    /** Let's check if voltage measured is in a valid range. */
                    if ((dcdc_voltage_measure >= (dcdcVoltageTarget - PTX_PERIPH_DCDC_VOLTAGE_THRESHOLD)) &&
                            (dcdc_voltage_measure <= (dcdcVoltageTarget + PTX_PERIPH_DCDC_VOLTAGE_THRESHOLD)))
                    {
                        dcdc_voltage_valid = 1;
                    }

                    /** If DCDC voltage is not in the expected range, Let's try again to get more accurate value. */
                    if (0 == dcdc_voltage_valid)
                    {
                        int16_t diff_value = (int16_t)(dcdcVoltageTarget - dcdc_voltage_measure);

                        if (diff_value > 0)
                        {
                            uint16_t dac_decrease;
                            dac_decrease = (uint16_t) ((1000u * ((uint32_t) diff_value)) / PTX_PERIPH_DCDC_VOLTAGE_UV_PER_STEP);

                            if ((dac_value - dac_decrease) <= PTX_PERIPH_DCDC_VOLTAGE_DAC_MIN_VAL)
                            {
                                /** Minimum dac value reached. */
                                dac_value = PTX_PERIPH_DCDC_VOLTAGE_DAC_MIN_VAL;
                                dcdc_voltage_valid = 1;
                            } else
                            {
                                /** As targeted DCDC voltage is higher than the measured one, DAC value shall be decreased. */
                                dac_value = (uint16_t) (dac_value - dac_decrease);
                            }
                        } else
                        {
                            /** Dif value is negative, let's change sign to get absolute value. */
                            diff_value = (int16_t)(-diff_value);

                            uint16_t dac_increase;

                            dac_increase = (uint16_t)((1000 * ((uint32_t)diff_value)) / PTX_PERIPH_DCDC_VOLTAGE_UV_PER_STEP);

                            if (dac_value + dac_increase >= PTX_PERIPH_DCDC_VOLTAGE_DAC_MAX_VAL)
                            {
                                /** Maximum dac value reached. */
                                dac_value = PTX_PERIPH_DCDC_VOLTAGE_DAC_MAX_VAL;
                                dcdc_voltage_valid = 1;
                            } else
                            {
                                /** As targeted DCDC voltage is lower than the measured one, DAC value shall be increased. */
                                dac_value = (uint16_t)(dac_value + dac_increase);
                            }
                        }

                        /** Set DAC and wait for the value to be applied. */
                        (void) ptxPERIPH_DCDC_SetDAC(&WLCNPeriph->Dcdc, dac_value);
                        R_BSP_SoftwareDelay(sleep_ms, BSP_DELAY_UNITS_MILLISECONDS);

                        dcdc_regaulate_attempts++;
                    }
                }
            } while ((0 == dcdc_voltage_valid) && (ptxStatus_Success == status) && (dcdc_regaulate_attempts < max_dcdc_regaulate_attempts));

            if (dcdc_regaulate_attempts == max_dcdc_regaulate_attempts)
            {
                status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InternalError);
            }

            /** Keep DCDC output voltage in any case. */
            WLCNPeriph->DcdcCurrentVoltage = dcdc_voltage_measure;
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_Peripherals_SetDCDC_Voltage(ptxWLCN_Peripherals_t *WLCNPeriph, uint16_t dcdcVoltageTarget)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((PTX_COMP_CHECK(WLCNPeriph, ptxStatus_Comp_WLCE_PERIPH)) && (dcdcVoltageTarget >= PTX_PERIPH_DCDC_VOLTAGE_MIN_VAL) && (dcdcVoltageTarget <= PTX_PERIPH_DCDC_VOLTAGE_MAX_VAL))
    {
        uint16_t dac_value;

        dac_value = (uint16_t) (PTX_PERIPH_DCDC_VOLTAGE_DAC(dcdcVoltageTarget) / PTX_PERIPH_DCDC_VOLTAGE_UV_PER_STEP);

        status = ptxPERIPH_DCDC_SetDAC(&WLCNPeriph->Dcdc, dac_value);

    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

/*
 * ####################################################################################################################
 * FUNCTIONS FOR POWER SENSOR
 * ####################################################################################################################
 */

ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetData(ptxWLCN_Peripherals_t *WLCNPeriph, ptxWLCN_Peripherals_PowerSensorData_t *powerSensorData)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((PTX_COMP_CHECK(WLCNPeriph, ptxStatus_Comp_WLCN_PERIPH)) && (NULL != powerSensorData))
    {
        /** Set all measurement values to 0. */
        memset(powerSensorData, 0, sizeof(ptxWLCN_Peripherals_PowerSensorData_t));

        status = ptxWLCN_PowerSensor_GetVoltage(&WLCNPeriph->PwrSensor);
        if (ptxStatus_Success == status)
        {
            powerSensorData->BusVoltage = WLCNPeriph->PwrSensor.BusVoltage;
        }
        status = ptxWLCN_PowerSensor_GetCurrent(&WLCNPeriph->PwrSensor);
        if (ptxStatus_Success == status)
        {
            powerSensorData->Current = WLCNPeriph->PwrSensor.Current;
        }

        powerSensorData->Power = (powerSensorData->BusVoltage * powerSensorData->Current) / 1000000u;

    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetPower(ptxWLCN_Peripherals_t *WLCNPeriph, uint32_t *powerMw)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((PTX_COMP_CHECK(WLCNPeriph, ptxStatus_Comp_WLCN_PERIPH)) && (NULL != powerMw))
    {
        status = ptxWLCN_PowerSensor_GetPower(&WLCNPeriph->PwrSensor);

        if (ptxStatus_Success == status)
        {
            *powerMw = (uint16_t)(WLCNPeriph->PwrSensor.Power / 1000u);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetVoltage(ptxWLCN_Peripherals_t *WLCNPeriph, uint16_t *voltage)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((NULL != WLCNPeriph) && (NULL != voltage))
    {
        status = ptxWLCN_PowerSensor_GetVoltage(&WLCNPeriph->PwrSensor);

        if (ptxStatus_Success == status)
        {
            *voltage = WLCNPeriph->PwrSensor.BusVoltage;
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetCurrent(ptxWLCN_Peripherals_t *WLCNPeriph, uint32_t *current)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((NULL != WLCNPeriph) && (NULL != current))
    {
        status = ptxWLCN_PowerSensor_GetCurrent(&WLCNPeriph->PwrSensor);

        if (ptxStatus_Success == status)
        {
            *current = WLCNPeriph->PwrSensor.Current;
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}
