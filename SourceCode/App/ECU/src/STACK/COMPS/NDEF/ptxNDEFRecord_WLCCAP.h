/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : NDEF WLC CAPABILITY RECORD API
    File        : ptxNDEFRecord_WLCCAP.h

    Description : WLC capability record API
*/

/**
 * \addtogroup grp_ptx_api_ndef_record_wlccap NDEF WLCCAP Record API
 *
 * @{
 */
#ifndef APIS_PTX_NDEF_RECORD_WLCCAP_H_
#define APIS_PTX_NDEF_RECORD_WLCCAP_H_

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
* \brief Record type definition for WLC Capability record.
*/
#define TYPE_RTD_WLCCAP         ("WLCCAP")
#define TYPE_RTD_WLCCAP_LEN     (6u)

#define MASK_WLCCAP_PROTVERSION_MAJOR       0xF0    /**< Major protocol version mask. */
#define MASK_WLCCAP_PROTVERSION_MAJOR_POS   (4u)    /**< Major protocol version position. */
#define MASK_WLCCAP_PROTVERSION_MINOR       0x0F    /**< Minor protocol version mask. */
#define MASK_WLCCAP_PROTVERSION_MINOR_POS   (0)     /**< Minor protocol version position. */

#define MASK_WLCCAP_CONF_MODEREQ            0xC0    /**< MODE_REQ mask. */
#define MASK_WLCCAP_CONF_MODEREQ_POS        (6u)    /**< MODE_REQ position. */
#define MASK_WLCCAP_CONF_NWTRET             0x3C    /**< NWT_RETRIES mask. */
#define MASK_WLCCAP_CONF_NWTRET_POS         (2u)    /**< NWT_RETRIES position. */
#define MASK_WLCCAP_CONF_NEGWAIT            0x02    /**< NEGO_WAIT flag mask. */
#define MASK_WLCCAP_CONF_NEGWAIT_POS        (1u)    /**< NEGO_WAIT flag position. */
#define MASK_WLCCAP_CONF_RDCONF             0x01    /**< RD_CONF flag mask. */
#define MASK_WLCCAP_CONF_RDCONF_POS         (0)     /**< RD_CONF flag position. */

/** CAP_WT_INT Field */
#define MASK_WLCCAP_CAPWT_INT               0x1F    /**< CAP_WT_INT mask. */
#define MASK_WLCCAP_CAPWT_INT_MAX           0x13    /**< CAP_WT_INT maximum value. */
#define MASK_WLCCAP_CAPWT_INT_POS           (0)     /**< CAP_WT_INT position. */

/** NDEF_RD_WT Field */
#define MASK_WLCCAP_RDWT_INT                0xFF    /**< RD_WT_INT mask. */
#define MASK_WLCCAP_RDWT_INT_MAX            0xFE    /**< RD_WT_INT maximum value. */
#define MASK_WLCCAP_RDWT_INT_POS            (0)     /**< RD_WT_INT position. */

/** WLC_WR_TO_INT Field */
#define MASK_WLCCAP_WRTOINT_INT             0x07    /**< WR_TO_INT mask. */
#define MASK_WLCCAP_WRTOINT_INT_MAX         0x04    /**< WR_TO_INT maximum value. */
#define MASK_WLCCAP_WRTOINT_INT_POS         (0)     /**< WR_TO_INT position. */

/** WLC_WR_WT_INT Field */
#define MASK_WLCCAP_WRWTINT_INT             0x0F    /**< WR_WT_INT mask. */
#define MASK_WLCCAP_WRWTINT_INT_MAX         0x0A    /**< WR_WT_INT maximum value. */
#define MASK_WLCCAP_WRWTINT_INT_POS         (0)     /**< WR_WT_INT position. */
/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Structure representing the WLC capability container record payload.
 */
typedef struct ptxWLCCAP
{
    uint8_t WLC_PROTOCOL_VER;
    uint8_t WLC_CONFIG;
    uint8_t CAP_WT_INT;
    uint8_t NDEF_RD_WT;
    uint8_t NDEF_WR_TO_INT;
    uint8_t NDEF_WR_WT_INT;
} ptxWLCCAP_t;

/**
 * \brief Defines the possible value of the mode field within the capability record.
 */
typedef enum ptxWLCMode
{
    ptxWLCMode_Static       = 0x00,
    ptxWLCMode_Negotiated   = 0x01,
    ptxWLCMode_BattFull     = 0x02,
    ptxWLCMode_RFU          = 0x03
} ptxWLCMode_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCCAP container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized ptxWLCCAP_t container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_Init (ptxWLCCAP_t *container);

/**
 * \brief Creates an NDEF record container from the given ptxWLCCAP_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCCAP container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_Create(ptxNDEFRecord_t *record, ptxWLCCAP_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptxWLCCAP_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCCAP container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_Parse(ptxNDEFRecord_t *record, ptxWLCCAP_t *container);

/**
 * \brief Setter for the WLCCAP protocol version, split into major and minor version number.
 *
 * \param[in]   container       Pointer to the WLCCAP container.
 * \param[in]   majorVersion    Major version number.
 * \param[in]   minorVersion    Minor version number.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetProtVersion (ptxWLCCAP_t *container, uint8_t majorVersion, uint8_t minorVersion);

/**
 * \brief Getter for the WLCCAP protocol version, split into major and minor version number.
 *
 * \param[in]       container       Pointer to the WLCCAP container.
 * \param[in,out]   majorVersion    Valid pointer to store major version number.
 * \param[in,out]   minorVersion    Valid pointer to store minor version number.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetProtVersion (ptxWLCCAP_t *container, uint8_t *majorVersion, uint8_t *minorVersion);

/**
 * \brief Setter for the WLCCAP mode.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   mode        Mode of operation.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetMode (ptxWLCCAP_t *container, ptxWLCMode_t mode);

/**
 * \brief Getter for the WLCCAP mode.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in,out]   mode       Valid pointer to mode of operation.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetMode (ptxWLCCAP_t *container, ptxWLCMode_t *mode);

/**
 * \brief Setter for the maximum number of repetitions of CAP_WT in WLCCAP.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   retries     Maximum number of repetitions.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetNWtRetries (ptxWLCCAP_t *container, uint8_t retries);

/**
 * \brief Getter for the maximum number of repetitions of CAP_WT in WLCCAP.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   retries     Valid pointer to store retries value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetNWtRetries (ptxWLCCAP_t *container, uint8_t *retries);

/**
 * \brief Setter for the NEGO_WAIT flag in WLCCAP.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   negoWait    Enable/Disable NEGO_WAIT.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetNegWait (ptxWLCCAP_t *container, uint8_t negoWait);

/**
 * \brief Getter for the NEGO_WAIT flag in WLCCAP.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   negoWait    Valid pointer to store NEGO_WAIT flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetNegWait (ptxWLCCAP_t *container, uint8_t *negoWait);

/**
 * \brief Setter for the read confirmation flag in WLCCAP.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   rdConf      Enable/Disable read confirmation flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetRdConf (ptxWLCCAP_t *container, uint8_t rdConf);

/**
 * \brief Getter for the read confirmation flag in WLCCAP.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   rdConf      Valid Pointer to store the read confirmation flag.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetRdConf (ptxWLCCAP_t *container, uint8_t *rdConf);

/**
 * \brief Setter for the CAP_WT integer value.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   capWt       CAP_WT integer value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetCapWt (ptxWLCCAP_t *container, uint8_t capWt);

/**
 * \brief Getter for the CAP_WT integer value.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   capWt       Valid pointer to store the CAP_WT value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetCapWt (ptxWLCCAP_t *container, uint8_t *capWt);

/**
 * \brief Getter for the CAP_WT value in milliseconds.
 *
 * \param[in]       container       Pointer to the WLCCAP container.
 * \param[in,out]   capWtMillis     Valid pointer to store the CAP_WT value in milliseconds.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetCapWtMillis (ptxWLCCAP_t *container, uint32_t *capWtMillis);

/**
 * \brief Setter for the response waiting time.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   rdWT        RD_WT integer value to be set.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetRdWt (ptxWLCCAP_t *container, uint8_t rdWT);

/**
 * \brief Getter for the response waiting time.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   rdWT        Valid pointer to store the RD_WT integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetRdWt (ptxWLCCAP_t *container, uint8_t *rdWT);

/**
 * \brief Setter for the NDEF write timeout integer.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   timeout     WR_TO integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetWrToInt (ptxWLCCAP_t *container, uint8_t timeout);

/**
 * \brief Getter for the NDEF write timeout integer.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   timeout     Valid pointer to store the WR_TO integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetWrToInt (ptxWLCCAP_t *container, uint8_t *timeout);

/**
 * \brief Setter for the NDEF write waiting time integer.
 *
 * \param[in]   container   Pointer to the WLCCAP container.
 * \param[in]   waitingTime Write waiting time integer value to be set.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_SetWrWtInt (ptxWLCCAP_t *container, uint8_t waitingTime);

/**
 * \brief Getter for the NDEF write waiting time integer.
 *
 * \param[in]       container   Pointer to the WLCCAP container.
 * \param[in,out]   waitingTime Valid pointer to store the write waiting time integer.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCCAP_GetWrWtInt (ptxWLCCAP_t *container, uint8_t *waitingTime);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* Guard */
