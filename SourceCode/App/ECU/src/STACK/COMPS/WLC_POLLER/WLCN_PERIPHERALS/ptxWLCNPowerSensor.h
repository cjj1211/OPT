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
    File        : ptxWLCNPowerSensor.h

    Description : Definitions and API of power sensor driver.
*/
/**
 * \addtogroup grp_ptx_api_wlcn_peripherals_psense PTX WLCN Power Sensor API
 *
 * @{
 */
#ifndef STACK_COMPS_WLC_POLLER_WLCN_PERIPHERALS_PTXWLCNPOWERSENSOR_H_
#define STACK_COMPS_WLC_POLLER_WLCN_PERIPHERALS_PTXWLCNPOWERSENSOR_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxStatus.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#ifdef POLLER_WLC_PCB_VERSION_V03
    #define INA231_PS
#else
    #define INA219_PS
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES FOR INA231 SENSOR
 * ####################################################################################################################
 */
#ifdef INA231_PS
    #define INA231_REG_CONFIG                   (0x00)   // Config register address
    #define INA231_CONFIG_RESET                 (0x8000) // Reset Bit
    #define INA231_CONFIG_BCT_MASK              (0x0038) // Bus voltage conversion time mask
    #define INA231_CONFIG_SCT_MASK              (0x01C0) // Shunt voltage conversion time mask
    #define INA231_CONFIG_AVG_MASK              (0x0E00) // Nr of Averaging samples mask
    #define INA231_CONFIG_MODE_MASK             (0x0007) // Operating Mode Mask
    #define INA231_REG_SHUNTVOLTAGE             (0x01)   // Shunt voltage register
    #define INA231_REG_BUSVOLTAGE               (0x02)   // Bus voltage register
    #define INA231_REG_POWER                    (0x03)   // Power register
    #define INA231_REG_CURRENT                  (0x04)   // Current register
    #define INA231_REG_CALIBRATION              (0x05)   // Calibration register
    #define INA231_REG_MASKENABLE               (0x06)   // Status and enable  Register (CNVR Bit)
    #define INA231_CNVR_BIT                     (0x0008) // Conversion ready bit

    /*      Reset values                                */
    enum {
       INA231_CONFIG_RESET_BIT         =       (0x8000),
       INA231_CONFIG_RESET_VALUE       =       (0x4127),
    };

    /*      Values for operating mode                   */
    enum {
       INA231_CONFIG_MODE_POWERDOWN            = (0x0000),
       INA231_CONFIG_MODE_SVOLT_TRIGGERED      = (0x0001),
       INA231_CONFIG_MODE_BVOLT_TRIGGERED      = (0x0002),
       INA231_CONFIG_MODE_SANDBVOLT_TRIGGERED  = (0x0003),
       INA231_CONFIG_MODE_POWERDOWN_2          = (0x0004),
       INA231_CONFIG_MODE_SVOLT_CONTINUOUS     = (0x0005),
       INA231_CONFIG_MODE_BVOLT_CONTINUOUS     = (0x0006),
       INA231_CONFIG_MODE_SANDBVOLT_CONTINUOUS = (0x0007), // - Device default
    };

    /*      Values for shunt voltage conversion time    */
    enum {
       INA231_CONFIG_VSH_CT_140US  = (0x0000),  // Shunt voltage conversion time set to 140us
       INA231_CONFIG_VSH_CT_204US  = (0x0008),  // Shunt voltage conversion time set to 204us
       INA231_CONFIG_VSH_CT_332US  = (0x0010),  // Shunt voltage conversion time set to 332us
       INA231_CONFIG_VSH_CT_588US  = (0x0018),  // Shunt voltage conversion time set to 588us
       INA231_CONFIG_VSH_CT_1MS1   = (0x0020),  // Shunt voltage conversion time set to 1.1ms - Device default
       INA231_CONFIG_VSH_CT_2MS116 = (0x0028),  // Shunt voltage conversion time set to 2.116ms
       INA231_CONFIG_VSH_CT_4MS156 = (0x0030),  // Shunt voltage conversion time set to 4.156ms
       INA231_CONFIG_VSH_CT_8MS244 = (0x0038),  // Shunt voltage conversion time set to 8.224ms
    };

    /*      Values for bus voltage conversion time      */
    enum {
       INA231_CONFIG_VBUS_CT_140US = (0x0000),  // Bus voltage conversion time set to 140us
       INA231_CONFIG_VBUS_CT_204US = (0x0040),  // Bus voltage conversion time set to 204us
       INA231_CONFIG_VBUS_CT_332US = (0x0080),  // Bus voltage conversion time set to 332us
       INA231_CONFIG_VBUS_CT_588US = (0x00C0),  // Bus voltage conversion time set to 588us
       INA231_CONFIG_VBUS_CT_1MS1  = (0x0100),  // Bus voltage conversion time set to 1.1ms - Device default
       INA231_CONFIG_VBUS_CT_2MS116= (0x0140),  // Bus voltage conversion time set to 2.116ms
       INA231_CONFIG_VBUS_CT_4MS156= (0x0180),  // Bus voltage conversion time set to 4.156ms
       INA231_CONFIG_VBUS_CT_8MS244= (0x01C0),  // Bus voltage conversion time set to 8.224ms
    };

    /*      Values for averaging factor                 */
    enum {
       INA231_CONFIG_AVG_SAMPLES_1     = (0x0000),  // Averaged 1 conversion sample - Device default
       INA231_CONFIG_AVG_SAMPLES_4     = (0x0200),  // Averaged 4 conversion samples
       INA231_CONFIG_AVG_SAMPLES_16    = (0x0400),  // Averaged 16 conversion samples
       INA231_CONFIG_AVG_SAMPLES_64    = (0x0600),  // Averaged 64 conversion samples
       INA231_CONFIG_AVG_SAMPLES_128   = (0x0800),  // Averaged 128 conversion samples
       INA231_CONFIG_AVG_SAMPLES_256   = (0x0A00),  // Averaged 256 conversion samples
       INA231_CONFIG_AVG_SAMPLES_512   = (0x0C00),  // Averaged 512 conversion samples
       INA231_CONFIG_AVG_SAMPLES_1024  = (0x0E00),  // Averaged 1024 conversion samples
    };

    #define INA2XX_REG_CONFIG                       INA231_REG_CONFIG
    #define INA2XX_REG_CALIBRATION                  INA231_REG_CALIBRATION
    #define INA2XX_CONFIG_MODE_MASK                 INA231_CONFIG_MODE_MASK
    #define INA2XX_CONFIG_MODE_SANDBVOLT_TRIGGERED  INA231_CONFIG_MODE_BVOLT_TRIGGERED
    #define INA2XX_REG_POWER                        INA231_REG_POWER
    #define INA2XX_REG_CURRENT                      INA231_REG_CURRENT
    #define INA2XX_CNVR_BIT                         INA231_CNVR_BIT
    #define INA2XX_CONFIG_RESET_BIT                 INA231_CONFIG_RESET_BIT
    #define INA2XX_CONFIG_RESET_VALUE               INA231_CONFIG_RESET_VALUE
    #define INA2XX_CONFIG_MODE_BVOLT_TRIGGERED      INA231_CONFIG_MODE_BVOLT_TRIGGERED
    #define INA2XX_CONFIG_MODE_SVOLT_TRIGGERED      INA231_CONFIG_MODE_SVOLT_TRIGGERED
    /*
     * POLLER Sensor Default Settings
     * Max. VBus        5400mV
     * Max. current:    800mA
     * Rshunt:          40mOhm
     * Vshuntmax:       32mV
     *
     * VBUSLSB:         1.25mV/bit
     * CurrentLSB:      100uA
     * PowerLSB:        2,5mW/bit = 2500uW/bit
     *
     *
     * min Current_LSB = Max Current/ 2^15 = 0,8/2^15 = 24,41uA/LSB -> select 100uA/LSB for easier calculation
     * Calibration register = 0.00512 / [CurrentLSB * Rshunt]
     * Calibration register = 0.00512 / [(100*10^(-6)) * 0.04] = 1280 = 0x0500
     */
    #define     POLLER_SENSOR_CURRENT_LSB               (100U)
    #define     POLLER_SENSOR_POWER_LSB                 (2500U)
    #define     POLLER_POWER_SENSOR_I2C_ADDRESS         (0x4A)
    #define     POLLER_SENSOR_SHUNT_RESISTOR            (40u)
    #define     POLLER_SENSOR_CONFIGURATION_REGISTER    (INA231_CONFIG_MODE_POWERDOWN + INA231_CONFIG_VSH_CT_140US + \
                                                         INA231_CONFIG_VBUS_CT_140US + INA231_CONFIG_AVG_SAMPLES_1)
    #define     POLLER_SENSOR_CALIBRATION_REGISTER      (0x0500) // 0,04Ohm 100uA
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES FOR INA219 SENSOR
 * ####################################################################################################################
 */
#ifdef INA219_PS
    #define INA219_REG_CONFIG                   (0x00)      // config register address
    #define INA219_CONFIG_RESET                 (0x8000)    // Reset Bit
    #define INA219_CONFIG_BVOLTAGERANGE_MASK    (0x2000)    // Bus Voltage Range Mask
    #define INA219_CONFIG_GAIN_MASK             (0x1800)    // Gain Mask
    #define INA219_CONFIG_BADCRES_MASK          (0x0780)    // Bus ADC Resolution Mask
    #define INA219_CONFIG_SADCRES_MASK          (0x0078)    // Shunt ADC Resolution and Averaging Mask
    #define INA219_CONFIG_MODE_MASK             (0x0007)    // Operating Mode Mask
    #define INA219_REG_SHUNTVOLTAGE             (0x01)      // shunt voltage register
    #define INA219_REG_BUSVOLTAGE               (0x02)      // bus voltage register
    #define INA219_REG_POWER                    (0x03)      // power register
    #define INA219_REG_CURRENT                  (0x04)      // current register
    #define INA219_REG_CALIBRATION              (0x05)      // calibration register
    #define INA219_CNVR_BIT                     (0x0002)    // conversion ready bit

    /*      reset values        */
    enum {
        INA219_CONFIG_RESET_BIT         = (0x8000),
        INA219_CONFIG_RESET_VALUE       = (0x399F),
    };

    /*      bus voltage range values        */
    enum {
        INA219_CONFIG_BVOLTAGERANGE_16V = (0x0000),  // 0-16V Range
        INA219_CONFIG_BVOLTAGERANGE_32V = (0x2000),  // 0-32V Range
    };

    /*      values for gain bits        */
    enum {
        INA219_CONFIG_GAIN_1_40MV   = (0x0000),  // Gain 1, 40mV Range
        INA219_CONFIG_GAIN_2_80MV   = (0x0800),  // Gain 2, 80mV Range
        INA219_CONFIG_GAIN_4_160MV  = (0x1000),  // Gain 4, 160mV Range
        INA219_CONFIG_GAIN_8_320MV  = (0x1800),  // Gain 8, 320mV Range
    };

    /*      values for bus ADC resolution       */
    enum {
        INA219_CONFIG_BADCRES_9BIT  = (0x0000),  // 09-bit bus res = 0..511
        INA219_CONFIG_BADCRES_10BIT = (0x0080),  // 10-bit bus res = 0..1023
        INA219_CONFIG_BADCRES_11BIT = (0x0100),  // 11-bit bus res = 0..2047
        INA219_CONFIG_BADCRES_12BIT = (0x0180),  // 12-bit bus res = 0..4097
    };

    /*      values for shunt ADC resolution     */
    enum {
        INA219_CONFIG_SADCRES_9BIT_1S_84US      = (0x0000),  // 1 x 9-bit shunt sample
        INA219_CONFIG_SADCRES_10BIT_1S_148US    = (0x0008),  // 1 x 10-bit shunt sample
        INA219_CONFIG_SADCRES_11BIT_1S_276US    = (0x0010),  // 1 x 11-bit shunt sample
        INA219_CONFIG_SADCRES_12BIT_1S_532US    = (0x0018),  // 1 x 12-bit shunt sample
        INA219_CONFIG_SADCRES_12BIT_2S_1060US   = (0x0048),  // 2 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_4S_2130US   = (0x0050),  // 4 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_8S_4260US   = (0x0058),  // 8 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_16S_8510US  = (0x0060),  // 16 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_32S_17MS    = (0x0068),  // 32 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_64S_34MS    = (0x0070),  // 64 x 12-bit shunt samples averaged together
        INA219_CONFIG_SADCRES_12BIT_128S_69MS   = (0x0078),  // 128 x 12-bit shunt samples averaged together
    };

    /*      values for operating mode       */
    enum {
        INA219_CONFIG_MODE_POWERDOWN            = (0x0000),
        INA219_CONFIG_MODE_SVOLT_TRIGGERED      = (0x0001),
        INA219_CONFIG_MODE_BVOLT_TRIGGERED      = (0x0002),
        INA219_CONFIG_MODE_SANDBVOLT_TRIGGERED  = (0x0003),
        INA219_CONFIG_MODE_ADCOFF               = (0x0004),
        INA219_CONFIG_MODE_SVOLT_CONTINUOUS     = (0x0005),
        INA219_CONFIG_MODE_BVOLT_CONTINUOUS     = (0x0006),
        INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS = (0x0007),
    };

    #define INA2XX_REG_CONFIG                       INA219_REG_CONFIG
    #define INA2XX_REG_CALIBRATION                  INA219_REG_CALIBRATION
    #define INA2XX_CONFIG_MODE_MASK                 INA219_CONFIG_MODE_MASK
    #define INA2XX_CONFIG_MODE_SANDBVOLT_TRIGGERED  INA219_CONFIG_MODE_BVOLT_TRIGGERED
    #define INA2XX_REG_POWER                        INA219_REG_POWER
    #define INA2XX_REG_CURRENT                      INA219_REG_CURRENT
    #define INA2XX_CNVR_BIT                         INA219_CNVR_BIT
    #define INA2XX_CONFIG_RESET_BIT                 INA219_CONFIG_RESET_BIT
    #define INA2XX_CONFIG_RESET_VALUE               INA219_CONFIG_RESET_VALUE
    #define INA2XX_CONFIG_MODE_BVOLT_TRIGGERED      INA219_CONFIG_MODE_BVOLT_TRIGGERED
    #define INA2XX_CONFIG_MODE_SVOLT_TRIGGERED      INA219_CONFIG_MODE_SVOLT_TRIGGERED
    /*
     * POLLER Sensor Default Settings
     * Max. VBus        5400mV
     * Max. current:    800mA
     * Rshunt:          40mOhm
     * Vshuntmax:       32mV
     * CurrentLSB:      100uA
     * PowerLSB:        2000uW
     * Calibration register = 0.04096 / [CurrentLSB * Rshunt]
     * Calibration register = 0.04096 / [(100*10^(-6)) * 0.04] = 10240 = 0x2800
     */
    #define     POLLER_SENSOR_MAXIMUM_CURRENT           (100U)
    #define     POLLER_SENSOR_CURRENT_LSB               (100U)
    #define     POLLER_SENSOR_POWER_LSB                 (2000U)
    #define     POLLER_POWER_SENSOR_I2C_ADDRESS         (0x40)
    #define     POLLER_SENSOR_SHUNT_RESISTOR            (40u)
    #define     POLLER_SENSOR_CONFIGURATION_REGISTER    (INA219_CONFIG_MODE_POWERDOWN + INA219_CONFIG_SADCRES_9BIT_1S_84US + \
                                                         INA219_CONFIG_BADCRES_9BIT + INA219_CONFIG_GAIN_1_40MV + \
                                                         INA219_CONFIG_BVOLTAGERANGE_16V)
    #define     POLLER_SENSOR_CALIBRATION_REGISTER      (0x2800) //0,04Ohm 100uA
#endif

/**
 * Power Sensor configuration parameters. These parameters are loaded during sensor initialization.
 */
typedef struct ptxWLCN_PowerSensor_Cfg
{
    uint8_t             ShuntResistor;          /**< Shunt resistor value, in mOhms. */
    uint8_t             CurrentLSB;             /**< Current LSB value, in uA. */
    uint16_t            PowerLSB;               /**< Power LSB value, in uW. */
    uint16_t            ConfigurationRegister;  /**< Configuration register content. */
    uint16_t            CalibrationRegister;    /**< Calibration register content. */
} ptxWLCN_PowerSensor_Cfg_t;


/**
 * Power sensor wrapper structure.
 */
typedef struct ptxWLCN_PowerSensor
{
    ptxStatus_Comps_t           CompId;         /**< Component Id. */
    uint16_t                    BusAddress;     /**< Bus address of the sensor. */
    ptxWLCN_PowerSensor_Cfg_t   *ConfigParam;   /**< Pointer to configuration parameters structure. */
    uint16_t                    BusVoltage;     /**< Bus voltage acquired from the power sensor, in mV. */
    uint32_t                    ShuntVoltage;   /**< Shunt voltage acquired from the power sensor, in uV. */
    uint32_t                    Current;        /**< Current acquired from the power sensor, in uA. */
    uint32_t                    Power;          /**< Power acquired from the power sensor, in mW. */
} ptxWLCN_PowerSensor_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialize power sensor with default configuration settings and provided communication interface instance.
 *
 * \note The caller has to provide allocated and initialized I2C interface instance. I2C interface initialization is not done here.
 *       Also, the caller provides the sensor type for which configuration settings will be loaded.
 *
 * \param[in]       sensorInst          Sensor instance, allocated by the caller.
 * \param[in]       cfgPars             Defines for which sensor default configuration parameters have to be loaded.
 *                                      Allocated communication interface (already opened and operable) instance needs to be provided here.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_PowerSensor_Init(ptxWLCN_PowerSensor_t *sensor, uint16_t i2cAddress);

/**
 * \brief De-Initialize power sensor.
 *
 * \param[in]       sensorInst          Sensor instance, allocated by the caller.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_PowerSensor_DeInit(ptxWLCN_PowerSensor_t *sensor);

/**
 * \brief Read all sensor data values (bus and shunt voltage, current, power) and update sensor instance data.
 *
 * \param[in]       sensorInst          Already allocated and initialized sensor instance.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_PowerSensor_GetData(ptxWLCN_PowerSensor_t *sensor);

/**
 * \brief Read bus voltage and update sensor instance data member.
 *
 * \param[in]       sensorInst          Already allocated and initialized sensor instance.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_PowerSensor_GetVoltage(ptxWLCN_PowerSensor_t *sensor);

/**
 * \brief Read current value and update sensor instance data member.
 *
 * \param[in]       sensorInst          Already allocated and initialized sensor instance.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_PowerSensor_GetCurrent(ptxWLCN_PowerSensor_t *sensor);

/**
 * \brief Read power value and update sensor instance data member.
 *
 * \param[in]       sensorInst          Already allocated and initialized sensor instance.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_PowerSensor_GetPower(ptxWLCN_PowerSensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif /* STACK_COMPS_WLC_POLLER_WLCN_PERIPHERALS_PTXWLCNPOWERSENSOR_H_ */
/** @} */
