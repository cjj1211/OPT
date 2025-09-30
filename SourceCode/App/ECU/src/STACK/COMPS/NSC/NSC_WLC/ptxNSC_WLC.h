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
    File        : ptxNSC_WLC.h

    Description :
*/

/**
 * \addtogroup grp_ptx_api_nsc_wlc PTX NSC WLC Extension
 *
 * @{
 */

#ifndef COMPS_NSC_NSC_CE_PTXNSC_WLC_H_
#define COMPS_NSC_NSC_CE_PTXNSC_WLC_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxStatus.h"
#include "ptxNSC.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_NSC_WLC_START_CMD_LEN               (69u)   /**< NSC WLC Start command length */
#define PTX_NSC_WLC_START_CMD_OPCODE            0x32    /**< NSC WLC Start command opcode */
#define PTX_NSC_WLC_START_RSP_LEN               (2u)    /**< NSC WLC Start response length */
#define PTX_NSC_WLC_START_RSP_OPCODE            0x72    /**< NSC WLC Start response opcode */

#define PTX_NSC_WLC_STOP_CMD_LEN                (34u)   /**< NSC WLC Stop command length */
#define PTX_NSC_WLC_STOP_CMD_OPCODE             0x33    /**< NSC WLC Stop command opcode */
#define PTX_NSC_WLC_STOP_RSP_LEN                (2u)    /**< NSC WLC Stop response length */
#define PTX_NSC_WLC_STOP_RSP_OPCODE             0x73    /**< NSC WLC Stop response opcode */

#define PTX_NSC_WLC_WPT_FINISHED_NTF_OPCODE     0x96    /**< NSC WLC WPT Finished NTF opcode */
#define PTX_NSC_WLC_WPT_FINISHED_NTF_LEN        (3u)    /**< NSC WLC WPT Finished NTF length */
#define PTX_NSC_WLC_WPT_NTF_OP_WPT_FINISHED     0x01    /**< NSC WLC WPT NTF OP WPT Finished */
#define PTX_NSC_WLC_WPT_NTF_OP_WPT_STOPREQ      0x02    /**< NSC WLC WPT NTF OP WPT STOPREQ */
#define PTX_NSC_WLC_WPT_NTF_OP_WPT_POWUP        0x03    /**< NSC WLC WPT NTF OP WPT POWUP */
#define PTX_NSC_WLC_WPT_NTF_OP_WPT_POWDOWN      0x04    /**< NSC WLC WPT NTF OP WPT POWDOWN */
#define PTX_NSC_WLC_WPT_NTF_OP_WPT_FOD          0x05    /**< NSC WLC WPT NTF OP WPT FOD */

/*
 * Forward declaration
 */
struct ptxNSC;

/**
 * \brief NSC WLC events
 */
typedef enum
{
    PowerTransfer_NoEvent,  /**< Power Transfer. No event. */
    PowerTransfer_OK,       /**< Power Transfer Cycle Finished successfully. */
    PowerTransfer_WPTStop,  /**< Power Transfer Cycle Finished due to WPT Stop Condition. */
    PowerTransfer_FoD       /**< Power Transfer Cycle Finished due to Foreign Object detected. */
}ptxNSC_WLC_Events_t;

/**
 * \brief NSC WLC power transfer options
 */
typedef struct ptxNSC_WLC_PowerTransferOptions
{

    uint8_t *WbCharging;        /**< Wavebank used for the WPT */
    uint8_t *WbPostCharging;    /**< Wavebank used after the WPT (can be NULL). */
    uint8_t WptDuration;        /**< WPT duration integer defining the length of the WPT phase. */
    uint8_t RffSyncDuration;    /**< Duration of the RFF sync pulse in the middle pf the WPT. */
    uint8_t Bfod;               /**< Boolean to enable Background Foreign Object Detection. (If 1 enabled) */
    uint8_t StopRequest;        /**< Boolean to enable Stop Request Detection.(If 1 enabled)  */
    uint8_t Bfod_Gt;            /**< Guard time after WPT is started until Bfod detection is activated. >*/
}ptxNSC_WLC_PowerTransferOptions_t;

/**
 * \brief NSC WLC CONFIG POWER LEVEL
 */
typedef struct ptxNSC_WLC_Config_PowerLevel
{
    uint8_t *WbNum_PowLevel1;   /**< Wavebank number to be applied for Power Level 1  */
    uint8_t *WbNum_PowLevel2;   /**< Wavebank number to be applied for Power Level 2  */
    uint8_t *WbNum_PowLevel3;   /**< Wavebank number to be applied for Power Level 3  */
    uint8_t *WbNum_PowLevel4;   /**< Wavebank number to be applied for Power Level 4  */
    uint8_t *WbNum_PowLevel5;   /**< Wavebank number to be applied for Power Level 5  */
} ptxNSC_WLC_Config_PowerLevel_t;

/**
 * \brief NSC WLC CONFIG PARAMETERS
 */
typedef struct ptxNSC_WLC_Config_Parameters
{
    ptxNSC_WLC_Config_PowerLevel_t  WlcConfigPowerLevel;    /**< WLC Config Power Level */
    uint8_t                        *WlcAdcConstThld;        /**< WLC ADC CONST THLD */
    uint8_t                        *WlcPga2Gain;            /**< WLC PGA2 GAIN */
} ptxNSC_WLC_Config_Parameters;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */


/**
 * \brief Initialize the NSC WLC subcomponent.
 *
 * This function has to be called before any other API functions at this subcomponent,
 * and after a successful call to \ref ptxNSC_Init.
 *
 * \param[in]   nscCtx           Pointer to an initialized NSC component.
 * \param[in]   callbackExt      Callback function for asynchronous events.
 * \param[in]   ctxExt           Context to be used by callbackExt.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxNSC_WLC_Init( struct ptxNSC *nscCtx, pptxNSC_ExtCallBack_t callbackExt, void *ctxExt);

/**
 * \brief De-initialize the NSC WLC component.
 *
 * This function has to be called last one at this subcomponent,
 * and before \ref ptxNSC_Deinit.
 *
 * \param[in]   nscCtx           Pointer to an initialized instance of the NSC.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxNSC_WLC_Deinit( struct ptxNSC *nscCtx);

/**
* \brief This function updates WLC config parameters (wavebanks, pga_gain, etc.).
*
* \param[in]   nscCtx                  Pointer to an initialized instance of the NSC.
* \param[in]   wlcConfigParams         WLC config parameters.
*
* \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
*/
ptxStatus_t ptxNSC_UpdateWLCConfig( struct ptxNSC *nscCtx, ptxNSC_WLC_Config_Parameters wlcConfigParams);

/**
 * \brief NSC_WLC_WPT_START_CMD Command
 *
 * \param[in]   nscCtx      Pointer to an initialized instance of the NSC.
 * \param[in]   chargingWb  Wavebank (32 entries) used for current WPT cycle.
 * \param[in]   postWptWb   Wavebank (32 entries) used after/between the WPT cycles.
 * \param[in]   durationInt WPT duration integer.
 * \param[in]   options     Config options for WPT cycle.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxNSC_WLC_WPTStartCmd(struct ptxNSC *nscCtx, ptxNSC_WLC_PowerTransferOptions_t *options);

/**
 * \brief NSC_WLC_WPT_STOP_CMD Command
 *
 * \param[in]   nscCtx      Pointer to an initialized instance of the NSC.
 * \param[in]   postWptWb   Wavebank (32 entries) used after/between the WPT cycles.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxNSC_WLC_WPTStopCmd(ptxNSC_t *nscCtx, uint8_t *postWptWb);

#ifdef __cplusplus
}
#endif

#endif /* Guard */

/** @} */

