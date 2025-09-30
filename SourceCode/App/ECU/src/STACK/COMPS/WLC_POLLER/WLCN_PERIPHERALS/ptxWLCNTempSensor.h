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
    File        : ptxWLCNTempSensor.h

    Description : Definitions and API of temperature sensor driver.
*/
/**
 * \addtogroup grp_ptx_api_wlcn_peripherals_tsense PTX WLCN Temperature Sensor API
 *
 * @{
 */
#ifndef STACK_COMPS_WLC_POLLER_WLCN_PERIPHERALS_PTXWLCNTEMPSENSOR_H_
#define STACK_COMPS_WLC_POLLER_WLCN_PERIPHERALS_PTXWLCNTEMPSENSOR_H_


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
    #define PTX_WLCN_TEMP_SENSOR_ADDRESS        (0x4B)
#else
    #define PTX_WLCN_TEMP_SENSOR_ADDRESS        (0x4C)
#endif

#define PTX_WLCN_TEMP_SENSOR_TEMP_REGISTER      (0x00)
#define PTX_WLCN_TEMP_SENSOR_CONF_REGISTER      (0x01)

/**
 * WLCN Temperature Sensor. Main Context.
 */
typedef struct ptxWLCN_TempSensor
{
    ptxStatus_Comps_t   CompId;     /**< Component Id. */
    uint16_t            BusAddress; /**< Slave address of temperature sensor. */
} ptxWLCN_TempSensor_t;

/*
 * ####################################################################################################################
 * FUNCTIONS FOR WLCN PERIPHERALS
 * ####################################################################################################################
 */

/**
 * \brief Initialize HW and SW needed for temperature sensor on poller board.
 *
 * \param[in]       tempSensor           Pointer to not initialized instance of Temperature Sensor.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TempSensor_Init(ptxWLCN_TempSensor_t *sensor, uint16_t address);

/**
 * \brief De-initialize HW and SW needed for temperature sensor on poller board.
 *
 * \param[in]       tempSensor           Pointer to initialized instance of Temperature Sensor.
 * \param[out]      tempValue            Pointer where
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TempSensor_ReadTemp(ptxWLCN_TempSensor_t *sensor, int16_t *tempValue);

/**
 * \brief De-initialize HW and SW needed for temperature sensor on poller board.
 *
 * \param[in]       tempSensor           Pointer to initialized instance of Temperature Sensor.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TempSensor_Deinit(ptxWLCN_TempSensor_t *sensor);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */
