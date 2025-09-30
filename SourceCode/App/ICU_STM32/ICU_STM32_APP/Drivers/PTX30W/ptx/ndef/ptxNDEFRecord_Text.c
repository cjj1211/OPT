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
    File        : ptxNDEFRecord_Text.c

    Description : NDEF Text record implementation
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNDEFRecord_Text.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxNDEFRecordText_Create (ptxNDEFRecord_t *record, uint8_t *workBuffer, size_t *workBufferLen, char *languageCode, char *text)
{
    ptxStatus_t status = ptxStatus_Success;

    /** Parameter checking. */
    if((NULL != record) && (NULL != workBuffer) && (NULL != languageCode) && (NULL != text))
    {
        /** Get length of language code (e.g. "en", "de"). */
        size_t lang_code_len = strlen(languageCode);
        /** Get length of actual text payload. */
        size_t text_len = strlen(text);
        /** Calculate the total payload length. */
        size_t total_len = 1u + lang_code_len + text_len;

        /** Check if the workbuffer is large enough. */
        if(*workBufferLen >= total_len)
        {
            /** Set the payload length. */
            *workBufferLen = total_len;

            /** Copy text record payload data together. */
            workBuffer[0] = 0; // UTF-8 only
            workBuffer[0] = (uint8_t) (workBuffer[0] | (uint8_t) (lang_code_len & 0x3F));

            /** Copy language code (has to be zero terminated). */
            strcpy((char *) &(workBuffer[1]), languageCode);
            /** Copy text. */
            strcpy((char *) &(workBuffer[1 + lang_code_len]), text);

            /** Create the record struct (including record header). */
            status = ptxNDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) TYPE_RTD_TEXT, TYPE_RTD_TEXT_LEN, NULL, 0, workBuffer, total_len);
        }
        else
        {
            /** Workbuffer not large enough. */
            status = ptxStatus_InsufficientResources;
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}
