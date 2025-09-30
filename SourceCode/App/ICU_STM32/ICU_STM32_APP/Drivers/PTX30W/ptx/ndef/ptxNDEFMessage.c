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
    Module      : Generic NDEF Message API
    File        : ptxNDEFMessage.c

    Description : Generic NDEF Message API
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNDEFMessage.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxNDEFMessage_Create (ptxNDEFRecord_t *recordBuffer, size_t recordSize, uint8_t *dstBuffer, size_t *bufferLen)
{
    ptxStatus_t status = ptxStatus_Success;

    /** Parameter checking. */
    if( (NULL != recordBuffer) && (NULL != dstBuffer) && (NULL != bufferLen) )
    {
        size_t record_num = 0u;
        size_t data_written = 0u;
        uint8_t *buffer_cpy = dstBuffer;
        size_t bufferLenCpy = *bufferLen;

        /** Iterate over available records. */
        while(record_num < recordSize)
        {
            /** In case of the very first record, we have to set the Message Begin (MB) flag in its header.  */
            if(0u == record_num)
            {
                ptxNDEFRecord_SetMB(&recordBuffer[record_num], 1u);
            }

            size_t record_size;
            /** Write the record to the destination buffer. */
            status = ptxNDEFRecord_Write(&recordBuffer[record_num], &buffer_cpy[data_written], bufferLenCpy - data_written, &record_size);

            /** Check if everything went well. */
            if(ptxStatus_Success != status)
            {
                break;
            }

            /** Increment the amount of successfully written data. */
            data_written += record_size;
            ++record_num;
        }

        if(ptxStatus_Success == status)
        {
            *bufferLen = data_written;
        }
    }
    else
    {
       status = ptxStatus_InvalidParameter;
    }

    return status;
}
