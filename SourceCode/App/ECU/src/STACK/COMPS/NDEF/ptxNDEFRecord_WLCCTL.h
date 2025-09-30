/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : NDEF WLC CONTROL RECORD API
    File        : ptxNDEFRecord_WLCCTL.h

    Description : WLC control record API
*/

/**
 * \addtogroup grp_ptx_api_ndef_record_wlcctl NDEF WLCCTL Record API
 *
 * @{
 */

#ifndef APIS_PTX_NDEF_RECORD_WLCCTL_H_
#define APIS_PTX_NDEF_RECORD_WLCCTL_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptxNDEFRecord.h"
#include "ptxStatus.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */
/**
* \brief Record type definition for WLC Listen Control record.
*/
#define TYPE_RTD_WLCCTL         ("WLCCTL")
#define TYPE_RTD_WLCCTL_LEN     (6u)

#define MASK_WLCCTL_STATINF_ERROR       0x80 /**< STATUS_INFO ERROR mask */
#define MASK_WLCCTL_STATINF_ERROR_POS   (7u) /**< STATUS_INFO ERROR position */
#define MASK_WLCCTL_STATINF_BATSTAT     0x18 /**< STATUS_INFO BATTSTAT mask */
#define MASK_WLCCTL_STATINF_BATSTAT_POS (3u) /**< STATUS_INFO BATTSTAT position */
#define MASK_WLCCTL_STATINF_CNT         0x07 /**< STATUS_INFO COUNTER mask */
#define MASK_WLCCTL_STATINF_CNT_POS     (0)  /**< STATUS_INFO COUNTER position */

#define MASK_WLCCTL_WPTCONF_REQ         0xC0 /**< WPT_CONF REQUEST mask */
#define MASK_WLCCTL_WPTCONF_REQ_POS     (6u) /**< WPT_CONF REQUEST position */
#define MASK_WLCCTL_WPTCONF_DUR         0x3E /**< WPT_CONF DURATION mask */
#define MASK_WLCCTL_WPTCONF_DUR_POS     (1u) /**< WPT_CONF DURATION position */
#define MASK_WLCCTL_WPTCONF_DUR_MAX     0x13 /**< WPT_CONF DURATION maximum value */
#define MASK_WLCCTL_WPTCONF_INFOREQ     0x01 /**< WPT_CONF INFO REQUEST mask */
#define MASK_WLCCTL_WPTCONF_INFOREQ_POS (0)  /**< WPT_CONF INFO REQUEST position */

#define MASK_WLCCTL_DVRINFO_SUP         0xC0 /**< DVRINFO INFO SUPPORT mask */
#define MASK_WLCCTL_DVRINFO_SUP_POS     (6u) /**< DVRINFO INFO SUPPORT position */
#define MASK_WLCCTL_DVRINFO_INT         0x3F /**< DVRINFO INTEGER mask */
#define MASK_WLCCTL_DVRINFO_INT_POS     (0)  /**< DVRINFO INTEGER position */

#define MASK_WLCCTL_HOLDOFF_WT_INT      0x0F /**< HOLDOFF_WT INTEGER mask */
#define MASK_WLCCTL_HOLDOFF_WT_INT_MAX  0x0F /**< HOLDOFF_WT INTEGER mximum value */
#define MASK_WLCCTL_HOLDOFF_WT_INT_POS  (0)  /**< HOLDOFF_WT INTEGER position */

#define MASK_WLCCTL_ERRINFO_TEMP        0x01 /**< ERROR_INFO TEMP ERROR mask */
#define MASK_WLCCTL_ERRINFO_PROT_ERR    0x02 /**< ERROR_INFO PROTOCOL ERROR mask */

#define MASK_WLCCTL_PWRADJ_MAX          (20) /**< PWR_ADJ maximum value */
#define MASK_WLCCTL_PWRADJ_MIN          (-10)/**< PWR_ADJ minimum value (signed) */
#define MASK_WLCCTL_PWRADJ_DEFAULT      0x00 /**< PWR_ADJ default value */
#define WLCCTL_PWRADJ_PERCENT_PER_STEP  (5)  /**< PWR_ADJ percent conversion factor */

#define MASK_WLCCTL_ERROR_INFO_LEN          (1u)    /**< Length in bytes of the ERROR_INFO field */
#define MASK_WLCCTL_PTX_ERROR_INF_MASK      (0x1F)  /**< Proprietary PTX ERROR_INFO mask */
#define MASK_WLCCTL_PTX_ERROR_INF_POS       (0x00)  /**< Proprietary PTX ERROR_INFO position */

typedef enum ptxWLCCTL_Error
{
    Error_NoError           = 0,    /**< OK */
    Error_IcTemperature     = 1,    /**< IC temperature too high */
    Error_Protocol          = 2,    /**< WLC protocol error */
    Error_BattNotConnected  = 4,    /**< Battery not connected error */
    Error_BattTemperature   = 8,    /**< Battery temperature error */
    Error_TcmTimeout        = 12,   /**< Trickle charge timeout */
    Error_CcmTimeout        = 16,   /**< Constant current timeout error */
    Error_CvmTimeout        = 20    /**< Constant voltage timeout error */
} ptxWLCCTL_Error_t;

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/
/**
 * \brief Structure representing the WLC Listen Control Record's payload.
 */
typedef struct ptxWLCCTL
{
    uint8_t STATUS_INFO;
    uint8_t WPT_CONFIG;
    int8_t  POWER_ADJ_REQ;
    uint8_t BATTERY_LEVEL;
    uint8_t DVR_INFO;
    uint8_t HOLD_OFF_WT_INT;
    uint8_t ERROR_INFO;
} ptxWLCCTL_t;

/**
 * \brief Battery status information.
 */
typedef enum ptxWLCCTL_BattStat
{
    NoInformation       = 0x00,
    BatteryCharging     = 0x01,
    NoBatteryCharging   = 0x20
} ptxWLCCTL_BattStat_t;

/**
 * \brief DVR support status.
 */
typedef enum ptxWLCCTL_DvrSupport
{
    DVRSupport_No       = 0x00,
    DVRSupport_Yes_NA   = 0x01,
    DVRSupport_Yes      = 0x02
} ptxWLCCTL_DvrSupport_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCCTL container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized WLCCTL container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_Init (ptxWLCCTL_t *container);

/**
 * \brief Create an NDEF record container from the given ptxWLCCTL_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCCTL container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_Create(ptxNDEFRecord_t *record, ptxWLCCTL_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptxWLCCTL_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCCTL container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_Parse(ptxNDEFRecord_t *record, ptxWLCCTL_t *container);

/**
 * \brief Getter for the WLCCTL length of the WLCCTL record.
 *        ERROR_INFO is optional, hence length can change.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   len         Valid pointer to store the container length.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetLength(ptxWLCCTL_t *container, uint8_t *len);

/**
 * \brief Setter for the WLCCTL error flag within STATUS_INFO field.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   error       Set/clear error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetError(ptxWLCCTL_t *container, uint8_t error);

/**
 * \brief Getter for the WLCCTL error flag within the STATUS_INFO field.
 *        If this flag is set, additional ERROR_INFO field must be set.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   error         Valid pointer to store the error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetError(ptxWLCCTL_t *container, uint8_t *error);

/**
 * \brief Setter for the WLCCTL battery status information.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   battStatus  Current battery status.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetBatteryStatus(ptxWLCCTL_t *container, ptxWLCCTL_BattStat_t battStatus);

/**
 * \brief Getter for the WLCCTL battery status field.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   battStatus  Valid pointer to store the battery status.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetBatteryStatus(ptxWLCCTL_t *container, ptxWLCCTL_BattStat_t *battStatus);

/**
 * \brief Setter for the WLCCTL counter.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   counter     Counter value (3 Bit).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetCounter(ptxWLCCTL_t *container, uint8_t counter);

/**
 * \brief Getter for the WLCCTL counter field. If the counter changes between
 *        readouts, this indicates to the poller that the listener updated its
 *        WLCCTL record.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   counter     Valid pointer to store the counter value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetCounter(ptxWLCCTL_t *container, uint8_t *counter);

/**
 * \brief Setter for the WLCCTL WPT duration integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   duration    WPT duration integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetWptReq(ptxWLCCTL_t *container, uint8_t wptRequest);

/**
 * \brief Getter for the WLCCTL WPT request flag.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   wptRequest     Valid pointer to store the WPT request flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetWptReq(ptxWLCCTL_t *container, uint8_t *wptRequest);

/**
 * \brief Setter for the WLCCTL WPT duration integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   duration    WPT duration integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetWptDuration(ptxWLCCTL_t *container, uint8_t duration);

/**
 * \brief Getter for the WLCCTL WPT duration integer.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   duration    Valid pointer to store the WPT duration integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetWptDuration(ptxWLCCTL_t *container, uint8_t *duration);

/**
 * \brief Getter for the WLCCTL WPT duration time in milliseconds.
 *
 * \param[in]       container       Pointer to the WLCCTL container.
 * \param[in,out]   wptDurationMs   Valid pointer to store the WPT duration in milliseconds.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetWptDurationMillis(ptxWLCCTL_t *container, uint32_t *wptDurationMs);

/**
 * \brief Setter for the WLCCTL INFO_REQ flag
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   request     Set/clear request flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetWptInfoReq(ptxWLCCTL_t *container, uint8_t request);

/**
 * \brief Getter for the WLCCTL INFO_REQ flag.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   request     Valid pointer to store the INFO_REQ flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetWptInfoReq(ptxWLCCTL_t *container, uint8_t *request);

/**
 * \brief Setter for the WLCCTL power adjust level.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   adjustLevel Power adjust level.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetPowerAdjust(ptxWLCCTL_t *container, int8_t adjustLevel);

/**
 * \brief Getter for the WLCCTL power adjust request of the listener.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   adjustLevel Valid pointer to store the power adjust integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetPowerAdjustReq(ptxWLCCTL_t *container, int8_t *adjustLevel);

/**
 * \brief Setter for the WLCCTL battery level.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   battLevel   Battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetBatteryLevel(ptxWLCCTL_t *container, uint8_t battLevel);

/**
 * \brief Getter for the WLCCTL battery level.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   battLevel   Valid pointer to store the battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetBatteryLevel(ptxWLCCTL_t *container, uint8_t *battLevel);

/**
 * \brief Setter for the WLCCTL DVR support value.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   support     DVR support value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetDvrSupport(ptxWLCCTL_t *container, ptxWLCCTL_DvrSupport_t support);

/**
 * \brief Getter for the WLCCTL DVR_INFO support.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   support     Valid pointer to store the support information.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetDvrSupport(ptxWLCCTL_t *container, ptxWLCCTL_DvrSupport_t *support);

/**
 * \brief Setter for the WLCCTL DVR integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   currDvr     Current DVR integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetDvrInt(ptxWLCCTL_t *container, uint8_t currDvr);

/**
 * \brief Getter for the WLCCTL DVR integer.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   currDvr     Valid pointer to store the DVR integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetDvrInt(ptxWLCCTL_t *container, uint8_t *currDvr);

/**
 * \brief Getter for the WLCCTL HOLD_OFF_WT integer.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   holdOffWt   Valid pointer to store HOLD_OFF_WT integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetHoldOffWt(ptxWLCCTL_t *container, uint8_t *holdOffWt);

/**
 * \brief Getter for the WLCCTL HOLD_OFF_WT in milliseconds.
 *
 * \param[in]       container       Pointer to the WLCCTL container.
 * \param[in,out]   holdOffWtMillis Valid pointer to store HOLD_OFF_WT in milliseconds.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetHoldOffWtMillis(ptxWLCCTL_t *container, uint32_t *holdOffWtMillis);

/**
 * \brief Setter for the WLCCTL HOLD_OFF_WT integer.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   holdOffWt   HOLD_OFF_WT integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetHoldOffWt(ptxWLCCTL_t *container, uint8_t holdOffWt);

/**
 * \brief Setter for the WLCCTL over temperature error.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   ovTemp      Set/Clear temperature error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetErrorOvTemp(ptxWLCCTL_t *container, uint8_t ovTemp);

/**
 * \brief Getter for the WLCCTL over temperature error.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   ovTemp      Valid pointer to store flag, if an error has occured.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetErrorOvTemp(ptxWLCCTL_t *container, uint8_t *ovTemp);

/**
 * \brief Setter for the WLCCTL protocol error.
 *
 * \param[in]   container   Pointer to the WLCCTL container.
 * \param[in]   protError   Set/Clear protocol error flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_SetErrorProt(ptxWLCCTL_t *container, uint8_t protError);

/**
 * \brief Getter for the WLCCTL protocol error.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   protError   Valid pointer to store flag, if an error has occured.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetErrorProt(ptxWLCCTL_t *container, uint8_t *protError);

/**
 * \brief Getter for the proprietary PTX error codes.
 *
 * \param[in]       container   Pointer to the WLCCTL container.
 * \param[in,out]   err         Valid pointer to store the PTX error code.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCTL_GetErrorPtx(ptxWLCCTL_t *container, ptxWLCCTL_Error_t *err);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* Guard */
