/** \file
    ---------------------------------------------------------------
    SPDX-License-Identifier: BSD-3-Clause

    Copyright (c) 2024, Renesas Electronics Corporation and/or its affiliates


    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list of
       conditions and the following disclaimer in the documentation and/or other
       materials provided with the distribution.

    3. Neither the name of Renesas nor the names of its
       contributors may be used to endorse or promote products derived from this
       software without specific prior written permission.



    THIS SOFTWARE IS PROVIDED BY Renesas "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
    OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL RENESAS OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    ---------------------------------------------------------------

    Project     : PTX30W
    Module      : NDEF Text Record
    File        : ptxNDEFRecord_Text.h

    Description : NDEF Text record API
*/


#ifndef APIS_PTX_NDEF_RECORD_TEXT_H_
#define APIS_PTX_NDEF_RECORD_TEXT_H_

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
* \brief Record type definition for text record.
*/
#define TYPE_RTD_TEXT           ("T")
#define TYPE_RTD_TEXT_LEN       (1u)

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
 * \brief Creates a text record from a specified language code and a given text.
 *
 * \param[in,out]   record          Pointer to an uninitialized NDEF record structure.
 * \param[in,out]   workBuffer      Pointer to a buffer for storing the assembled payload.
 * \param[in,out]   workBufferLen   Pointer to a variable containing the available buffer length.
 * \param[in]       languageCode    Language code of the record.
 * \param[in]       text            The actual text content.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordText_Create (ptxNDEFRecord_t *record, uint8_t *workBuffer, size_t *workBufferLen, char *languageCode, char *text);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
