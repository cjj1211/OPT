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
    Module      : URI NDEF RECORD API
    File        : ptxNDEFRecord_URI.h

    Description : API for URI NDEF record functions.
*/

#ifndef APIS_PTX_NDEF_RECORD_URI_H_
#define APIS_PTX_NDEF_RECORD_URI_H_

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
* \brief Record type definition for URI record.
*/
#define TYPE_RTD_URI            ("U")
#define TYPE_RTD_URI_LEN        (1u)

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/
/**
 * \brief URI Identifier Codes
 */
typedef enum ptxURIRecord_Identifier
{
    NONE,
    HTTP_WWW,
    HTTPS_WWW,
    HTTP,
    HTTPS,
    TEL,
    MAILTO,
    FTP_ANONYMOUS,
    FTP_FTP,
    FTPS,
    SFTP,
    SMB,
    NFS,
    FTP,
    DAV,
    NEWS,
    TELNET,
    IMAP,
    RTSP,
    URN,
    POP,
    SIP,
    SIPS,
    TFTP,
    BTSPP,
    BTL2CAP,
    BTGOEP,
    TCPOBEX,
    IRDAOBEX,
    FILE,
    URN_EPC_ID,
    URN_EPC_TAG,
    URN_EPC_PAT,
    URN_EPC_RAW,
    URN_EPC,
    URN_NFC
} ptxURIRecord_Identifier_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Creates an NDEF URI record from a specified URI and URI Record identifier.
 *
 * \param[in,out]   record          Pointer to an uninitialized NDEF record structure.
 * \param[in,out]   workBuffer      Pointer to a buffer for storing the assembled payload.
 * \param[in,out]   workBufferLen   Pointer to a variable containing the available buffer length.
 * \param[in]       idf         URI record identifier option.
 * \param[in]       uri         Pointer to the character array describing the URI.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordUri_Create (ptxNDEFRecord_t *record, uint8_t *workBuffer, size_t *workBufferLen, ptxURIRecord_Identifier_t idf, char *uri);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
