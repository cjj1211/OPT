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
    File        : ptxWLCNPeripherals.h

    Description: This file implements several (optional) APIs to
                 control the peripherals on the WLC Poller Eval PCB.
                 These peripherals include a DC/DC converter, a
                 temperature sensor and a power sensor.
*/
/**
 * \addtogroup grp_ptx_api_wlcn_peripherals PTX WLCN PERIPHERALS API
 *
 * @{
 */
#ifndef STACK_COMPS_WLC_POLLER_WLCN_PERIPHERALS_PTXWLCNPERIPHERALS_H_
#define STACK_COMPS_WLC_POLLER_WLCN_PERIPHERALS_PTXWLCNPERIPHERALS_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxWLCNPowerSensor.h"
#include "ptxWLCNTempSensor.h"
#include "ptxPERIPH_DCDC.h"
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

/**
 * WLCN Power sensor data structure.
 */
typedef struct ptxWLCN_Peripherals_PowerSensorData
{
    uint16_t BusVoltage;    /**< Bus voltage acquired from the power sensor, in mV. */
    uint32_t Current;       /**< Current acquired from the power sensor, in uA. */
    uint32_t Power;         /**< Power acquired from the power sensor, in mW. */
} ptxWLCN_Peripherals_PowerSensorData_t;


/**
 * WLCN peripherals init parameters.
 */
typedef struct ptxWLCN_Peripherals_InitParams
{
    uint16_t DcdcVoltage;   /**< Desired DCDC voltage output. */
} ptxWLCN_Peripherals_InitParams_t;


/**
 * WLCN peripherals. Main Context.
 */
typedef struct ptxWLCN_Peripherals
{
    ptxStatus_Comps_t       CompId;             /**< Component Id. */
    ptxWLCN_PowerSensor_t   PwrSensor;          /**< Power Sensor. */
    ptxPERIPH_DCDC_t        Dcdc;               /**< DCDC Context. */
    uint16_t                DcdcCurrentVoltage; /**< Actual voltage at DCDC output. */
} ptxWLCN_Peripherals_t;


/*
 * ####################################################################################################################
 * FUNCTIONS FOR WLCN PERIPHERALS
 * ####################################################################################################################
 */


/**
 * \brief Initialize HW and SW needed for peripherals on poller board.
 *
 * \param[in]       WLCNPeriph           Pointer to not initialized instance of WLCN peripherals.
 * \param[in]       initPars             Initialization parameters of WLCN peripherals.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_Peripherals_Init(ptxWLCN_Peripherals_t *WLCNPeriph, ptxWLCN_Peripherals_InitParams_t initPars);

/**
 * \brief De-initialize HW and SW needed.
 *
 * \param[in]       WLCNPeriph           Pointer to initialized instance of WLCN peripherals.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_Peripherals_Deinit(ptxWLCN_Peripherals_t *WLCNPeriph);

/**
 * \brief Set DCDC output voltage to a targeted value.
 *
 * \param[in]       WLCNPeriph           Pointer to initialized instance of WLCN peripherals.
 * \param[in]       dcdcVoltageTarget    Target voltage (in mV) to which the DCDC converter is going to be calibrated.
 *                                       The set voltage is read back via the power sensor and verified.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_Peripherals_Calibration(ptxWLCN_Peripherals_t *WLCNPeriph, uint16_t dcdcVoltageTarget);

/**
* \brief Set DCDC output voltage to a targeted value without calibration loop
*
* \param[in]       WLCNPeriph           Pointer to initialized instance of WLCN peripherals.
* \param[in]       dcdcVoltageTarget    Target voltage (in mV) to which the DCDC converter is going to be set.
*
* \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
*/
ptxStatus_t ptxWLCN_Peripherals_SetDCDC_Voltage(ptxWLCN_Peripherals_t *WLCNPeriph, uint16_t dcdcVoltageTarget);

/*
 * ####################################################################################################################
 * API FUNCTION FOR WLCN PERIPHERALS
 * ####################################################################################################################
 */

/**
* \brief Get current measurement values from power sensor (voltage, current and power).
 *
 * \param[in]       WLCNPeriph           Pointer to initialized instance of WLCN peripherals.
 * \param[in,out]      powerSensorData   Pointer to data structure where the measurement data of the power sensor is going to be written.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetData(ptxWLCN_Peripherals_t *WLCNPeriph, ptxWLCN_Peripherals_PowerSensorData_t *powerSensorData);

/**
 * \brief Get voltage level from power sensor.
 *
 * \param[in]       WLCNPeriph           Pointer to initialized instance of WLCN peripherals.
 * \param[in,out]   voltage              Pointer where voltage value is going to be written.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetVoltage(ptxWLCN_Peripherals_t *WLCNPeriph, uint16_t *voltage);

/**
* \brief Get current power from power sensor.
*
* \param[in]       WLCNPeriph           Pointer to initialized instance of WLCN peripherals.
* \param[in,out]   powerMw              Pointer where power consumption in mW value is going to be written.
*
* \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
*/
ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetPower(ptxWLCN_Peripherals_t *WLCNPeriph, uint32_t *powerMw);

/**
* \brief Get measured current from power sensor.
*
* \param[in]       WLCNPeriph           Pointer to initialized instance of WLCN peripherals.
* \param[in,out]      current           Pointer where current consumption is going to be written.
*
* \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
*/
ptxStatus_t ptxWLCN_Peripherals_PowerSensor_GetCurrent(ptxWLCN_Peripherals_t *WLCNPeriph, uint32_t *current);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */
