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
    File        : ptxWLCNPowerSensor.c

    Description : Driver implementation for generic power sensor (in this case TI INA219/INA231)
    providing voltage and current digital data over I2C.

 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxWLCNPowerSensor.h"
#include "ptxPERIPH_I2C.h"
#include <string.h>

const ptxWLCN_PowerSensor_Cfg_t PollerSensConfiguration =
{
    /** POLLER SENSOR */
    .ShuntResistor = POLLER_SENSOR_SHUNT_RESISTOR,
    .CurrentLSB = POLLER_SENSOR_CURRENT_LSB,
    .PowerLSB = POLLER_SENSOR_POWER_LSB,
    .ConfigurationRegister = POLLER_SENSOR_CONFIGURATION_REGISTER,
    .CalibrationRegister = POLLER_SENSOR_CALIBRATION_REGISTER
};

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */
static ptxStatus_t ptxWLCN_PowerSensor_WriteData(uint8_t regAddress, uint16_t regValue);
static ptxStatus_t ptxWLCN_PowerSensor_ReadData(uint8_t regAddress, uint16_t *regValue);
static ptxStatus_t ptxWLCN_PowerSensor_SetRegPtr(uint8_t regAddress);
static ptxStatus_t ptxWLCN_PowerSensor_ReadRaw(uint16_t *regValue);
static ptxStatus_t ptxWLCN_PowerSensor_Reset(ptxWLCN_PowerSensor_t *sensor);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptxStatus_t ptxWLCN_PowerSensor_Init(ptxWLCN_PowerSensor_t *sensor, uint16_t i2cAddress)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != sensor)
    {
        sensor->ConfigParam = (ptxWLCN_PowerSensor_Cfg_t*) &PollerSensConfiguration;

        status = ptxPERIPH_I2C_SetSlaveAddress(i2cAddress);

        if (ptxStatus_Success == status)
        {
            /** Configure the sensor. */
            uint16_t config_reg = sensor->ConfigParam->ConfigurationRegister;
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, config_reg);

            if (ptxStatus_Success == status)
            {
                uint16_t cal_reg = sensor->ConfigParam->CalibrationRegister;
                status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CALIBRATION, cal_reg);

                if (ptxStatus_Success == status)
                {
                    sensor->BusAddress = i2cAddress;
                    /** Init and configuration went well. Do final set-up. */
                    sensor->CompId = ptxStatus_Comp_WLCN_PERIPH;
                }
            }
        }
        if (ptxStatus_Success != status)
        {
            memset(sensor, 0, sizeof(ptxWLCN_PowerSensor_t));
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxWLCN_PowerSensor_DeInit(ptxWLCN_PowerSensor_t *sensor)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(sensor, ptxStatus_Comp_WLCN_PERIPH))
    {
        (void)memset(sensor, 0, sizeof(ptxWLCN_PowerSensor_t));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_PowerSensor_GetData(ptxWLCN_PowerSensor_t *sensor)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(sensor, ptxStatus_Comp_WLCN_PERIPH))
    {
        /**
         * Sensor is in power-down mode.
         * 1) Set shunt and bus voltage triggered mode. Typ. 40us needed to wake up from power down.
         * 2) Read bus voltage register.
         * 3) Check CNV_RD bit. If not set, repeat 2 and 3. Max 3 retries.
         * 5) Read current and power registers.
         * 4) Go back to power down mode?
         */
        uint16_t reg_val = 0;

        /**
         * Set slave address at one place in a function body. No need to repeat the call in local function calls
         * because they all refer to the same device.
         * Avoid communication in ISRs which could change address.
         */
        status = ptxPERIPH_I2C_SetSlaveAddress(sensor->BusAddress);

        if (ptxStatus_Success == status)
        {
            uint16_t config_reg = sensor->ConfigParam->ConfigurationRegister & INA2XX_CONFIG_MODE_MASK;
            config_reg = (uint16_t)(config_reg | INA2XX_CONFIG_MODE_SANDBVOLT_TRIGGERED);
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, config_reg);
        }

        if (ptxStatus_Success == status)
        {
            uint8_t num_retries = 0;
            uint8_t conv_ok = 0;
#ifdef INA231_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA231_REG_MASKENABLE);
#endif
#ifdef INA219_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA219_REG_BUSVOLTAGE);
#endif
            if (ptxStatus_Success == status)
            {
                do
                {
                    /**
                     * Get bus voltage register. Check if conversion is done.
                     * Typically, conversion (shunt and bus voltage) is done for the second read attempt.
                     * For a single conversion type - shunt or bus voltage, conversion data is ready
                     * already for the first read attempt.
                     */
                    reg_val = 0;
                    status = ptxWLCN_PowerSensor_ReadRaw(&reg_val);
                    if (ptxStatus_Success == status)
                    {
                        if ((reg_val & INA2XX_CNVR_BIT) != 0)
                        {
                            /** Conversion is done. */
                            conv_ok = 1;
#ifdef INA231_PS
                            status = ptxWLCN_PowerSensor_ReadData(INA231_REG_BUSVOLTAGE, &reg_val);
                            if (ptxStatus_Success == status)
                            {
                                /**

                                 * Resolution is 1.25mV.
                                 */
                                reg_val = (uint16_t)((reg_val & 0x7FFF) * 1.25);
                                sensor->BusVoltage = reg_val;
                            }
#endif
#ifdef INA219_PS
                            /**
                             * The result is first right-shifted by 3 bits and then multiplied by 4 (i.e. shifted left by 2 bits).
                             * Resolution is 4mV. This gives 1 right shift in total.
                             */
                            reg_val = (reg_val & 0xFFF8) >> 1;
                            sensor->BusVoltage = reg_val;
#endif
                            break;
                        }
                    }
                    num_retries++;
                } while(num_retries < 3);

                if((ptxStatus_Success == status) && (0 == conv_ok))
                {
                    /**
                     * Communication was successful, but conversion not done in expected time.
                     * Mark this as internal error.
                     */
                    status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InternalError);
                }
            }
        }

        if (ptxStatus_Success == status)
        {
            /** Get current register. */
            reg_val = 0;
            status = ptxWLCN_PowerSensor_ReadData(INA2XX_REG_CURRENT, &reg_val);

            if (ptxStatus_Success == status)
            {
                sensor->Current = (uint32_t)(reg_val * sensor->ConfigParam->CurrentLSB);
            }
        }

        if (ptxStatus_Success == status)
        {
            /** Get power register. */
            reg_val = 0;
            status = ptxWLCN_PowerSensor_ReadData(INA2XX_REG_POWER, &reg_val);

            if (ptxStatus_Success == status)
            {
                sensor->Power = (uint32_t)(reg_val * sensor->ConfigParam->PowerLSB);
            }
        }

        if (ptxStatus_Success == status)
        {
            /** Set default mode. */
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, sensor->ConfigParam->ConfigurationRegister);
        } else
        {
            /** Reset and re-initialize sensor. Keep current error status and forward it. */
            (void) ptxWLCN_PowerSensor_Reset(sensor);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }
    return status;
}

ptxStatus_t ptxWLCN_PowerSensor_GetVoltage(ptxWLCN_PowerSensor_t *sensor)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(sensor, ptxStatus_Comp_WLCN_PERIPH))
    {
        /**
         * Sensor is in power-down mode.
         * 1) Set bus voltage triggered mode. Typ. 40us needed to wake up from power down.
         * 2) Read bus voltage register.
         * 3) Check CNV_RD bit. If not set, repeat 2 and 3. Max 3 retries.
         * 4) Go back to power down mode?
         */
        uint16_t reg_val = 0;

        /**
         * Set slave address at one place in a function body. No need to repeat the call in local function calls
         * because they all refer to the same device.
         * Avoid communication in ISRs which could change address.
         */

        status = ptxPERIPH_I2C_SetSlaveAddress(sensor->BusAddress);

        if (ptxStatus_Success == status)
        {
            uint16_t config_reg = sensor->ConfigParam->ConfigurationRegister & INA2XX_CONFIG_MODE_MASK;
            config_reg = (uint16_t)(config_reg | INA2XX_CONFIG_MODE_BVOLT_TRIGGERED);
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, config_reg);
        }

        if (ptxStatus_Success == status)
        {
            uint8_t num_retries = 0;
            uint8_t conv_ok = 0;
#ifdef INA231_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA231_REG_MASKENABLE);
#endif
#ifdef INA219_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA219_REG_BUSVOLTAGE);
#endif

            if (ptxStatus_Success == status)
            {
                do
                {
                    /**
                     * Get bus voltage register. Check if conversion is done.
                     * Typically, conversion (shunt and bus voltage) is done for the second read attempt.
                     * For a single conversion type - shunt or bus voltage, conversion data is ready
                     * already for the first read attempt.
                     */
                    reg_val = 0;
                    status = ptxWLCN_PowerSensor_ReadRaw(&reg_val);
                    if (ptxStatus_Success == status)
                    {
                        if ((reg_val & INA2XX_CNVR_BIT) != 0)
                        {
                            /** Conversion is done. */
                            conv_ok = 1;
#ifdef INA231_PS
                            status = ptxWLCN_PowerSensor_ReadData(INA231_REG_BUSVOLTAGE, &reg_val);
                            if (ptxStatus_Success == status)
                            {
                                /**
                                 * The result is first right-shifted by 3 bits and then multiplied by 4 (i.e. shifted left by 2 bits).
                                 * Resolution is 4mV. This gives 1 right shift in total.
                                 */
                                reg_val = (uint16_t)((reg_val & 0x7FFF) * 1.25);
                                sensor->BusVoltage = reg_val;
                            }
#endif
#ifdef INA219_PS
                            /**
                             * The result is first right-shifted by 3 bits and then multiplied by 4 (i.e. shifted left by 2 bits).
                             * Resolution is 4mV. This gives 1 right shift in total.
                             */
                            reg_val = (reg_val & 0xFFF8) >> 1;
                            sensor->BusVoltage = reg_val;
#endif
                            break;
                        }
                    }
                    num_retries++;
                } while(num_retries < 3);

                if((ptxStatus_Success == status) && (0 == conv_ok))
                {
                    /**
                     * Communication was successful, but conversion not done in expected time.
                     * Mark this as internal error.
                     */
                    status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InternalError);
                }
            }
        }

        if (ptxStatus_Success == status)
        {
            /** Set default mode. */
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, sensor->ConfigParam->ConfigurationRegister);
           } else
        {
            /** Reset and re-initialize sensor. Keep current error status and forward it. */
            (void)ptxWLCN_PowerSensor_Reset(sensor);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_PowerSensor_GetCurrent(ptxWLCN_PowerSensor_t *sensor)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(sensor, ptxStatus_Comp_WLCN_PERIPH))
    {
        /**
         * Sensor is in power-down mode.
         * 1) Set shunt voltage triggered mode. Typ. 40us needed to wake up from power down.
         * 2) Read bus voltage register.
         * 3) Check CNV_RD bit. If not set, repeat 2 and 3. Max 3 retries.
         * 4) Read current register.
         * 5) Go back to power down mode?
         */
        uint16_t reg_val = 0;

        /**
         * Set slave address at one place in a function body. No need to repeat the call in local function calls
         * because they all refer to the same device.
         * Avoid communication in ISRs which could change address.
         */

        status = ptxPERIPH_I2C_SetSlaveAddress(sensor->BusAddress);

        if (ptxStatus_Success == status)
        {
            uint16_t config_reg = sensor->ConfigParam->ConfigurationRegister & INA2XX_CONFIG_MODE_MASK;
            config_reg = (uint16_t)(config_reg | INA2XX_CONFIG_MODE_SVOLT_TRIGGERED);
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, config_reg);

        }

        if (ptxStatus_Success == status)
        {
            uint8_t num_retries = 0;
            uint8_t conv_ok = 0;
#ifdef INA231_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA231_REG_MASKENABLE);
#endif
#ifdef INA219_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA219_REG_BUSVOLTAGE);
#endif

            if (ptxStatus_Success == status)
            {
                do
                {
                    /** Get bus voltage register just to check CNVR bit. */
                    reg_val = 0;
                    status = ptxWLCN_PowerSensor_ReadRaw(&reg_val);
                    if (ptxStatus_Success == status)
                    {
                        if ((reg_val & INA2XX_CNVR_BIT) != 0)
                        {
                            /** Conversion is done. Read current register and exit loop. */
                            conv_ok = 1;

                            reg_val = 0;
                            status = ptxWLCN_PowerSensor_ReadData(INA2XX_REG_CURRENT, &reg_val);
                            if (ptxStatus_Success == status)
                            {
                                sensor->Current = (uint32_t)(reg_val * sensor->ConfigParam->CurrentLSB);
                            }

                            break;
                        }
                    }

                    num_retries++;
                } while(num_retries < 3);

                if((ptxStatus_Success == status) && (0 == conv_ok))
                {
                    /**
                     * Communication was successful, but conversion not done in expected time.
                     * Mark this as internal error.
                     */
                    status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InternalError);
                }
            }
        }

        if (ptxStatus_Success == status)
        {
            /** Set default mode. */
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, sensor->ConfigParam->ConfigurationRegister);
        } else
        {
            (void)ptxWLCN_PowerSensor_Reset(sensor);
        }
    } else
    {
        /** Reset and re-initialize sensor. Keep current error status and forward it. */
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_PowerSensor_GetPower(ptxWLCN_PowerSensor_t *sensor)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(sensor, ptxStatus_Comp_WLCN_PERIPH) && (NULL != sensor))
    {
        /**
         * Sensor is in power-down mode.
         * 1) Set shunt and bus voltage triggered mode. Typ. 40us needed to wake up from power down.
         * 2) Read bus voltage register.
         * 3) Check CNV_RD bit. If not set, repeat 2 and 3. Max 3 retries.
         * 4) Read power register.
         * 5) Go back to power down mode?
         */
        uint16_t reg_val = 0;

        /**
         * Set slave address at one place in a function body. No need to repeat the call in local function calls
         * because they all refer to the same device.
         * Avoid communication in ISRs which could change address.
         */

        status = ptxPERIPH_I2C_SetSlaveAddress(sensor->BusAddress);

        if (ptxStatus_Success == status)
        {
            uint16_t config_reg = sensor->ConfigParam->ConfigurationRegister & INA2XX_CONFIG_MODE_MASK;
            config_reg = (uint16_t)(config_reg | INA2XX_CONFIG_MODE_SANDBVOLT_TRIGGERED);
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, config_reg);
        }

        if (ptxStatus_Success == status)
        {
            uint8_t num_retries = 0;
            uint8_t conv_ok = 0;
#ifdef INA231_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA231_REG_MASKENABLE);
#endif
#ifdef INA219_PS
            status = ptxWLCN_PowerSensor_SetRegPtr(INA219_REG_BUSVOLTAGE);
#endif

            if (ptxStatus_Success == status)
            {
                do
                {
                    /** Get bus voltage register just to check CNVR bit. */
                    reg_val = 0;
                    status = ptxWLCN_PowerSensor_ReadRaw(&reg_val);
                    if (ptxStatus_Success == status)
                    {
                       if ((reg_val & INA2XX_CNVR_BIT) != 0)
                       {
                           /** Conversion is done. Read power register and exit loop. */
                           conv_ok = 1;

                           /** Read power register. */
                           reg_val = 0;
                           status = ptxWLCN_PowerSensor_ReadData(INA2XX_REG_POWER, &reg_val);
                           if (ptxStatus_Success == status)
                           {
                               sensor->Power = ((uint32_t)reg_val * sensor->ConfigParam->PowerLSB);
                           }

                           break;
                       }
                    }
                    num_retries++;
                } while(num_retries < 3);

                if((ptxStatus_Success == status) && (0 == conv_ok))
                {
                    /**
                     * Communication was successful, but conversion not done in expected time.
                     * Mark this as internal error.
                     */
                    status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InternalError);
                }
            }
        }

        if (ptxStatus_Success == status)
        {
            /** Set default mode. */
            status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, sensor->ConfigParam->ConfigurationRegister);
        } else
        {
            /** Reset and re-initialize sensor. Keep current error status and forward it. */
            (void)ptxWLCN_PowerSensor_Reset(sensor);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


/*
 * ####################################################################################################################
 * LOCAL FUNCTIONS
 * ####################################################################################################################
 */

static ptxStatus_t ptxWLCN_PowerSensor_WriteData(uint8_t regAddress, uint16_t regValue)
{
    ptxStatus_t status = ptxStatus_Success;

    /**
     * Everything went well so far. Let´s write to the register.
     */
    const uint8_t tx_len = 3u;
    uint8_t tx_buf[tx_len];
    tx_buf[0] = regAddress;
    tx_buf[1] = (uint8_t)((regValue >> 8) & 0x00FF);
    tx_buf[2] = (uint8_t)(regValue & 0x00FF);

    status = ptxPERIPH_I2C_TRx(tx_buf, tx_len, 0, NULL, 0);

    return status;
}

static ptxStatus_t ptxWLCN_PowerSensor_ReadData(uint8_t regAddress, uint16_t *regValue)
{
    ptxStatus_t status = ptxStatus_Success;

    /** Set register pointer. */
    status = ptxPERIPH_I2C_TRx(&regAddress, 1u, 0, NULL, 0);

    if (ptxStatus_Success == status)
    {
        /** Read register value. */
        const uint8_t rx_len = 2u;
        uint8_t rx_buf[rx_len];
        uint16_t reg_val = 0;

        status = ptxPERIPH_I2C_TRx(NULL, 0, 0, rx_buf, rx_len);

        if (ptxStatus_Success == status)
        {
            reg_val = (uint16_t)(rx_buf[0] << 8) & 0xFF00;
            reg_val = (uint16_t)(reg_val | (uint16_t)(rx_buf[1] & 0x00FF));
            *regValue = reg_val;
        }
    }

    return status;
}

static ptxStatus_t ptxWLCN_PowerSensor_SetRegPtr(uint8_t regAddress)
{
    ptxStatus_t status = ptxStatus_Success;

    const uint8_t tx_len = 1u;
    uint8_t tx_data = regAddress;

    /**
     * Set register pointer. This function is useful if there is multiple read from the same register afterwards.
     * Use ptxWLCN_PowerSensor_ReadRaw for subsequent read-outs.
     */
    status = ptxPERIPH_I2C_TRx(&tx_data, tx_len, 0, NULL, 0);

    return status;
}

static ptxStatus_t ptxWLCN_PowerSensor_ReadRaw(uint16_t *regValue)
{
    ptxStatus_t status = ptxStatus_Success;

    /**
     *  Read register value. Do not set register pointer here, just read from the current location.
     *  It is expected that set_reg is done by a dedicated call to ptxWLCN_PowerSensor_SetRegPtr before.
     */
    const uint8_t rx_len = 2u;
    uint8_t rx_buf[rx_len];
    uint16_t reg_val = 0;

    status = ptxPERIPH_I2C_TRx(NULL, 0, 0, rx_buf, rx_len);

    if (ptxStatus_Success == status)
    {
        reg_val = (uint16_t)(rx_buf[0] << 8) & 0xFF00;
        reg_val = (uint16_t)(reg_val | (uint16_t)(rx_buf[1] & 0x00FF));
        *regValue = reg_val;
    }

    return status;
}

static ptxStatus_t ptxWLCN_PowerSensor_Reset(ptxWLCN_PowerSensor_t *sensor)
{
    ptxStatus_t status = ptxStatus_Success;

    /**
     * Set Reset bit in Configuration register. Wait a while.
     * Initialize sensor again to its expected values: configuration power mode, gain, resolution and calibration register.
     */

    status = ptxPERIPH_I2C_SetSlaveAddress(sensor->BusAddress);

    if (ptxStatus_Success == status)
    {
        uint16_t config_reg = sensor->ConfigParam->ConfigurationRegister;
        status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, (config_reg | INA2XX_CONFIG_RESET_BIT));
    }

    if (ptxStatus_Success == status)
    {
        /**
         * Check that reset is done and the time to wait elapsed:
         * Check config register value which has a defined value after reset.
         */
        uint16_t config_reg = 0;
        uint8_t num_retries = 0;

        do
        {
            status = ptxWLCN_PowerSensor_ReadData(INA2XX_REG_CONFIG, &config_reg);

            if (ptxStatus_Success == status)
            {
               if(INA2XX_CONFIG_RESET_VALUE == config_reg)
               {
                   break;
               }
            }
            num_retries++;
        } while(num_retries < 3);
    }

    if (ptxStatus_Success == status)
    {
        uint16_t config_reg = sensor->ConfigParam->ConfigurationRegister;
        status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CONFIG, config_reg);
    }

    if (ptxStatus_Success == status)
    {
        uint16_t cal_reg = sensor->ConfigParam->CalibrationRegister;
        status = ptxWLCN_PowerSensor_WriteData(INA2XX_REG_CALIBRATION, cal_reg);
    }

    return status;
}


