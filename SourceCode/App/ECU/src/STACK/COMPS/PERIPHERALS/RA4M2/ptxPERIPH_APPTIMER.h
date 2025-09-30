/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : PERIPHERALS
    File        : ptxPERIPH_APPTIMER.h

    Description :
*/

/**
 * \addtogroup grp_ptx_api_peripherals_timer PTX Peripherals Application Timer API
 *
 * @{
 */

#ifndef STACK_COMPS_PERIPHERALS_PTXPERIPH_APPTIMER_H_
#define STACK_COMPS_PERIPHERALS_PTXPERIPH_APPTIMER_H_

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
typedef struct TimerStatus
{
    uint8_t IsElapsed;
    uint32_t ElapsedTime;
} TimerStatus_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes and starts the timer for a given duration.
 *
 * \param[in]   ms  Milliseconds until the timer elapses.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_APPTIMER_Start(uint32_t ms);

/**
 * \brief Retrieves status information from the timer.
 *
 * \param[in,out]   timerStatus Pointer to \ref TimerStatus_t.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_APPTIMER_Status(TimerStatus_t *timerStatus);

/**
 * \brief Stops and deinitializes the timer.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_APPTIMER_Stop();

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */

