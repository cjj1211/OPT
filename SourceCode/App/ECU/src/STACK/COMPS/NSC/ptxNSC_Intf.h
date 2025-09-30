/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : NSC
    File        : ptxNSC_Intf.h

    Description :
*/

/**
 * \addtogroup grp_ptx_api_nsc_hal PTX NSC Stack HAL
 *
 * @{
 */

#ifndef COMPS_NSC_PTXNSC_INTF_H_
#define COMPS_NSC_PTXNSC_INTF_H_


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
 * DEFINES / TYPES / INTERNALS
 * ####################################################################################################################
 */

/**
 * Forward declaration of NSC Component
 */
struct ptxNSC;


/*
 * ####################################################################################################################
 * FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Asynchronous reception callback.
 *
 * This function is called from ISR. Typical usage e.g. in GPIO IRQ for SPI, or in Rx ISR for UART.
 * It performs reading of PTX buffer to get NSC notification data.
 *
 * \param[in]   nscCtx           Pointer to NSC Context.
 *
 * \return No status returned.
 */
void ptxNSC_GetRx (struct ptxNSC *nscCtx);


#if defined (PTX_INTF_UART)
/**
 * \brief Set Rx control parameters to initial values.
 *
 * This is typically called before starting Tx operation, so that asynchronous Rx operation after Tx could
 * start with expected state.
 *
 * \param[in]   nscCtx           Pointer to NSC Context.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxNSC_UARTSetCleanStateRx (struct ptxNSC *nscCtx);

/**
 * \brief Synchronize host and PTX communication interface speed.
 *
 * This is typically called before the call to reset NSC. There might be the case where the host UART interface
 * changes speed although the speed on NSC chip UART interface has not changed. Use this API to set proper speed on
 * the host interface in order to continue with NSC initialization.
 *
 * \param[in]   nscCtx           Pointer to NSC Context.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxNSC_UARTComSync(struct ptxNSC *nscCtx);

/**
 * \brief Set default UART speed for the host.
 *
 * This is typically called after NSC reset has been performed.
 * After reset the host interface will operate on the default UART baudrate. However, there might happen that
 * after the call to \ref ptxNSC_UartComSync host UART speed is not the default one.
 * Call this API then to set the default UART speed.
 *
 * \param[in]   nscCtx           Pointer to NSC Context.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxNSC_UARTSetDefaultSpeed(struct ptxNSC *nscCtx);

#endif


#ifdef __cplusplus
}
#endif

#endif /* Guard */

/** @} */

