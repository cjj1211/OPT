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
    File        : ptxPERIPH_I2C.h

    Description :
*/

/**
 * \addtogroup grp_ptx_api_peripherals_I2C PTX Peripherals I2C
 *
 * @{
 */

#ifndef STACK_COMPS_PERIPHERALS_PTXPERIPH_I2C_H_
#define STACK_COMPS_PERIPHERALS_PTXPERIPH_I2C_H_


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
typedef enum ptxPERIPH_I2CTRxStatus
{
    TRX_NOT_STARTED     = 0,
    TRX_TX_INPROGRESS   = 1u,
    TRX_TX_DONE         = 2u,
    TRX_RX_INPROGRESS   = 3u,
    TRX_RX_DONE         = 4u,
    TRX_TIMEOUT         = 5u,
    TRX_ERROR           = 6u
} ptxPERIPH_I2CTRxStatus_t;

ptxStatus_t ptxPERIPH_I2C_Init(void);
ptxStatus_t ptxPERIPH_I2C_Deinit(void);
ptxStatus_t ptxPERIPH_I2C_Tx(const uint8_t *txBuf, const size_t txLen, uint8_t restart);
ptxStatus_t ptxPERIPH_I2C_Rx(uint8_t *txBuf, const size_t rxLen, uint8_t restart);
ptxStatus_t ptxPERIPH_I2C_TRx(const uint8_t *txBuf, const size_t txLen, uint8_t restart, uint8_t *rxBuf, const size_t rxLen);
ptxStatus_t ptxPERIPH_I2C_SetSlaveAddress(uint16_t slaveAddress);
ptxStatus_t ptxPERIPH_I2C_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */

