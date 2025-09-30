/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : WPT
    File        : ptxWPT.h

    Description :
*/
/**
 * \addtogroup grp_ptx_api_wpt PTX WPT API
 *
 * @{
 */
#ifndef STACK_COMPS_WLC_POLLER_WPT_PTXWPT_H_
#define STACK_COMPS_WLC_POLLER_WLC_PTXWPT_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxStatus.h"
#include "ptxNSC_WLC.h"
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
 * \brief Main WPT component structure.
 */
typedef struct ptxWPT
{
    ptxStatus_Comps_t   CompId;     /**< Component Id. */
    struct ptxNSC       *nscCtx;    /**< Reference to NSC Component, used for NSC WLC extension. */
} ptxWPT_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */


/**
 * \brief Initialize the WPT (Wireless Power Transfer) component.
 *
 * This function has to be called before any other API functions at this component.
 *
 * \param[in]   wpt          Pointer to WPT component to be initialized.
 * \param[in]   nscCtx       Pointer to NSC component already initialized.
 * \param[in]   callbackExt  Callback function for asynchronous events.
 * \param[in]   ctxExt       Context to be used by callbackExt.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWPT_Init( ptxWPT_t *wpt, struct ptxNSC *nscCtx, pptxNSC_ExtCallBack_t callbackExt, void *ctxExt);

/**
 * \brief Update the wavebanks for power levels.
 *
 * This function has to be called before any other API functions at this component.
 *
 * \param[in]   wpt                     Pointer to WPT Component to be initialized.
 * \param[in]   wlcConfigParams         WLC Config Parameter.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWPT_UpdateChipConfig( ptxWPT_t *wpt, ptxNSC_WLC_Config_Parameters wlcConfigParams);

/**
 * \brief Start a WPT (Wireless Power Transfer) cycle.
 *
 * \param[in]   wpt          Pointer to WPT component.
 * \param[in]   powerLevel   Power Level for the WPT cycle.
 * \param[in]   durationInt  WPT duration integer.
 * \param[in]   options      WPT transfer options.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWPT_Start( ptxWPT_t *wpt, ptxNSC_WLC_PowerTransferOptions_t *options);

/**
 * \brief Stop a WPT (Wireless Power Transfer) cycle.
 *
 * \param[in]   wpt          Pointer to WPT component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWPT_Stop( ptxWPT_t *wpt, uint8_t *postWptWb);

/**
 * \brief De-Initialize the the WPT (Wireless Power Transfer) component.
 *
 * This function has to be called last one at this component.
 *
 * \param[in]   wpt           Pointer to WPT component.
 *  *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWPT_Deinit( ptxWPT_t *wpt);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */
