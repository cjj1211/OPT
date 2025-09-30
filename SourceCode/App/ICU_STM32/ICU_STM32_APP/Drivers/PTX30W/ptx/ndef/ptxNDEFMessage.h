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
    Module      : Generic NDEF MESSAGE API
    File        : ptxNDEFMessage.h

    Description : API for creating and parsing NDEF messages into
                  individual records functions.
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
 * \brief Creates an NDEF Message from individual NDEF records and stores it in a buffer.
 *
 * \param[in]       recordBuffer    Pointer to an existing (and initialized) NDEF record buffer array.
 * \param[in]       recordSize      Size of the NDEF record buffer.
 * \param[in]       dstBuffer       Pointer to an existing buffer, where the data gets written to.
 * \param[in,out]   bufferLen       Pointer to an integer describing the available 'dstBuffer' length (in). Actual written length (out).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFMessage_Create (ptxNDEFRecord_t *recordBuffer, size_t recordSize, uint8_t *dstBuffer, size_t *bufferLen);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
