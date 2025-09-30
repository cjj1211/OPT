/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : WLCN
    File        : ptxWLCN_TDC_HLP.h

    Description: This file shows a usage example of the TDC with
                 a very light weight protocol on top.
*/
/**
 * \addtogroup grp_wlcn_api_tdc_hlp PTX WLC TDC HLP API
 *
 * @{
 */
#ifndef COMPS_WLC_POLLER_WLCN_WLCN_TDC_HLP_H_
#define COMPS_WLC_POLLER_WLCN_WLCN_TDC_HLP_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxStatus.h"
#include "ptxWLCN.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * \brief Example implementation of a very simple and light weight protocol on top of the Transparent Data Channel (TDC)
 *         to receive larger amounts of data than 63 bytes. The protocol implements an initial handshake and chunks
 *         the data into frames of 62 bytes of payload.
 *
 *
 * \param[in]       wlcn        Pointer to WLCN Component.
 * \param[in,out]   rxData      Pointer to store the received data.
 * \param[in,out]   rxDataLen   Pointer to store the amount of bytes received.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
 ptxStatus_t ptxWLCN_TDC_HlpRx(ptxWLCN_t *wlcn, uint8_t *rxData, uint32_t *rxDataLen);

 /**
  * \brief Example implementation of a very simple and light weight protocol on top of the Transparent Data Channel (TDC)
  *         to send larger amounts of data than 63 bytes. The protocol implements an initial handshake and chunks
  *         the data into frames of 62 bytes of payload.
  *
  *
  * \param[in]  wlcn        Pointer to WLCN Component.
  * \param[in]  txData      Pointer to a buffer to be transmitted to the listener.
  * \param[in]  txDataLen   Amount of bytes to be transmitted.
  *
  * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
  */
ptxStatus_t ptxWLCN_TDC_HlpTx(ptxWLCN_t *wlcn, uint8_t *txData, uint32_t txDataLen);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */
