/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : Generic NDEF MESSAGE API
    File        : ptxNDEFMessage.h

    Description : API for creating and parsing NDEF messages into
                  individual records functions.
*/

/**
 * \addtogroup grp_ptx_api_ndef_message Generic NDEF Message API
 *
 * @{
 */

#ifndef APIS_PTX_NDEF_MESSAGE_H_
#define APIS_PTX_NDEF_MESSAGE_H_

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

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Parses an NDEF Message from a buffer, into its individual NDEF records.
 *
 * \param[in]       srcBuffer       Pointer to the buffer to parse the NDEF massage from.
 * \param[in]       bufferLen       Length of the buffer that needs to be parsed.
 * \param[out]      recordBuffer    Buffer of NDEF records to store the parsed data.
 * \param[in,out]   recordLen       Length of the NDEF record buffer.
 * \param[in]       ignoreMBME      Ignore message begin and message end flags during parsing.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFMessage_Parse (uint8_t *srcBuffer, uint32_t bufferLen, ptxNDEFRecord_t *recordBuffer, uint32_t *recordLen, uint8_t ignoreMBME);

/**
 * \brief Creates an NDEF Message from individual NDEF records and stores it in a buffer.
 *
 * \param[in]       recordBuffer    Pointer to an existing (and initialized) NDEF record buffer array.
 * \param[in]       recordSize      Size of the NDEF record buffer.
 * \param[in]       dstBuffer       Pointer to an existing buffer, where the data gets written to.
 * \param[in,out]   bufferLen       Pointer to an integer describing the available 'dstBuffer' length (in). Actual written length (out).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFMessage_Create (ptxNDEFRecord_t *recordBuffer, uint32_t recordSize, uint8_t *dstBuffer, uint32_t *bufferLen);

/**
 * \brief Filters a given array of NDEF Records by its record type.
 *
 * \param[in]   recordBuffer    Pointer to an existing NDEF record buffer array.
 * \param[in]   recordsLen      Amount of records to filter.
 * \param[in]   type            Pointer to the record type to search for.
 * \param[in]   typeLen         Length of the given record type.
 * \param[in]   foundRecords    Pointer to an existing array, to store the IDs of the found/filtered records.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFMessage_FilterRecordsByType (ptxNDEFRecord_t *recordBuffer, uint32_t *recordsLen, char* type, uint8_t typeLen, uint32_t *foundRecords);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* Guard */
