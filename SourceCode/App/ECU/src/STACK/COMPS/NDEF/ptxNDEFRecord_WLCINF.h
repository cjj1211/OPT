/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : NDEF WLC POLL INFORMATION RECORD API
    File        : ptxNDEFRecord_WLCINF.h

    Description : WLC Information record API
*/

/**
 * \addtogroup grp_ptx_api_ndef_record_wlcinf NDEF WLCINF Record API
 *
 * @{
 */

#ifndef APIS_PTX_NDEF_RECORD_WLCINF_H_
#define APIS_PTX_NDEF_RECORD_WLCINF_H_

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
* \brief Record type definition for WLC Poll Information record.
*/
#define TYPE_RTD_WLCINF         ("WLCINF")
#define TYPE_RTD_WLCINF_LEN     (6u)

#define MASK_WLCINF_PTX_INT         0x7F    /**< P_TX Field mask */
#define MASK_WLCINF_PTX_INT_MAX     0x64    /**< P_TX Field max value */
#define MASK_WLCINF_PTX_INT_POS     0       /**< P_TX Field position */

#define MASK_WLCINF_WLCPCAP         0xF0    /**< WLCPCAP Field mask */
#define MASK_WLCINF_WLCPCAP_POS     4       /**< WLCPCAP Field position */
#define MASK_WLCINF_WLCPCAP_STOPSUP 0x10    /**< WLCPCAP stop support bit mask */

#define MASK_WLCINF_PCLASS          0x0F    /**< PCLASS Field mask */
#define MASK_WLCINF_PCLASS_POS      0       /**< PCLASS Field position */


/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Structure representing the WLC Poll Information Record payload.
 */
typedef struct ptxWLCINF
{
    uint8_t P_TX;
    uint8_t WLC_PCAP_POWER;
    uint8_t TOT_POWER_STEPS;
    uint8_t CUR_POWER_STEP;
    int8_t  NEXT_MIN_STEP_INC;
    int8_t  NEXT_MIN_STEP_DEC;
} ptxWLCINF_t;

/**
 * \brief Allowed values of the Power Class field within the WLCINF record.
 */
typedef enum ptxWLCINF_PowerClass
{
    PowerClass0 = 0x00, /**< POWER CLASS 0: 250 mW  */
    PowerClass1 = 0x01, /**< POWER CLASS 0: 500 mW  */
    PowerClass2 = 0x02, /**< POWER CLASS 0: 750 mW  */
    PowerClass3 = 0x03  /**< POWER CLASS 0: 1000 mW */
} ptxWLCINF_PowerClass_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCINF container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized ptxWLCINF_t container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_Init (ptxWLCINF_t *container);

/**
 * \brief Creates an NDEF record container from the given ptxWLCINF_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCINF container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_Create(ptxNDEFRecord_t *record, ptxWLCINF_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptxWLCINF_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCINF container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_Parse(ptxNDEFRecord_t *record, ptxWLCINF_t *container);

/**
 * \brief Setter for the WLCINF poller transmit power.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   percent     Current poller transmit power in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_SetPTX(ptxWLCINF_t *container, uint8_t percent);

/**
 * \brief Getter for the WLCINF poller transmit power.
 *
 * \param[in]       container       Pointer to the WLCINF container.
 * \param[in, out]  percent         Valid pointer to store the current poller transmit power in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_GetPTX(ptxWLCINF_t *container, uint8_t *percent);

/**
 * \brief Setter for the WLCINF WPT stop support.
 *
 * \param[in]   container       Pointer to the WLCINF container.
 * \param[in]   wptStopSupport  Enable/Disable WPT stop support.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_SetWptStopSupport(ptxWLCINF_t *container, uint8_t wptStopSupport);

/**
 * \brief Getter for WLCINF stop support.
 *
 * \param[in]       container       Pointer to the WLCINF container.
 * \param[in, out]  wptStopSupport  Valid pointer to store the stop support flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_GetWptStopSupport(ptxWLCINF_t *container, uint8_t *wptStopSupport);

/**
 * \brief Setter for the WLCINF power class.
 *
 * \param[in]   container       Pointer to the WLCINF container.
 * \param[in]   powerClass      Power class to be used for upcoming WPTs.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_SetPowerClass(ptxWLCINF_t *container, ptxWLCINF_PowerClass_t powerClass);

/**
 * \brief Getter for WLCINF power class.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  powerClass  Valid pointer to store current power class into.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_GetPowerClass(ptxWLCINF_t *container, ptxWLCINF_PowerClass_t *powerClass);

/**
 * \brief Setter for the WLCINF total power steps.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   step        Amount of power steps supported by the poller.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_SetTotPowerSteps(ptxWLCINF_t *container, uint8_t step);

/**
 * \brief Getter for WLCINF total amount of power steps.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  step        Valid pointer to store the amount of supported power steps.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_GetTotPowerSteps(ptxWLCINF_t *container, uint8_t *step);

/**
 * \brief Setter for the WLCINF currently used power step.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   step        Current power step in use.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_SetCurPowerStep(ptxWLCINF_t *container, uint8_t step);

/**
 * \brief Getter for the WLCINF current power step.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  step        Valid pointer to store the current power step.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_GetCurPowerStep(ptxWLCINF_t *container, uint8_t *step);

/**
 * \brief Setter for the WLCINF minimum power step increase.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   stepSize    Minimum power step increase.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_SetNextMinStepInc(ptxWLCINF_t *container, int8_t stepSize);

/**
 * \brief Getter for the WLCINF minimum step increase.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  stepSize    Valid pointer to store the step size.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_GetNextMinStepInc(ptxWLCINF_t *container, int8_t *stepSize);

/**
 * \brief Setter for the WLCINF minimum power step decrease.
 *
 * \param[in]   container   Pointer to the WLCINF container.
 * \param[in]   stepSize    Minimum power step decrease.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_SetNextMinStepDec(ptxWLCINF_t *container, int8_t stepSize);

/**
 * \brief Getter for the WLCINF minimum step decrease.
 *
 * \param[in]       container   Pointer to the WLCINF container.
 * \param[in, out]  stepSize    Valid pointer to store the step size.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCINF_GetNextMinStepDec(ptxWLCINF_t *container, int8_t *stepSize);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* Guard */
