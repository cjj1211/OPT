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
    Module      : CRC
    File        : ptx30w_Crc.c

    Description : Module implementing CRC calculation.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptx30w_Crc.h"

#define NFC_A_CRC_INITIAL_STATE (0x6363)
#define NFC_B_CRC_INITIAL_STATE (0xFFFF)

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptxStatus_t ptx30wCrc_Init(ptx30wCrc_t *ctx, ptx30wCrcType_t type)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != ctx)
    {
        ctx->Type = type;

        switch(ctx->Type)
        {
            case ptx30wCrcType_NfcA:
                ctx->State = NFC_A_CRC_INITIAL_STATE;
                break;
            case ptx30wCrcType_NfcB:
                ctx->State = NFC_B_CRC_INITIAL_STATE;
                break;
            default:
               status = ptxStatus_InvalidParameter;
               break;
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}

ptxStatus_t ptx30wCrc_Update(ptx30wCrc_t *ctx, uint8_t *data, size_t dataLen)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (NULL != ctx) && (NULL != data) )
    {
        size_t temp_len = dataLen;

        do
        {
            uint8_t b = *data;
            ++data;

            /** Feed the input into the shift registers and XOR with CRC. */
            b = (b ^ (uint8_t)(ctx->State & 0x00FF));
            b = (uint8_t)(b ^ (b << 4));

            /** Calculate the CRC value */
            ctx->State = (uint16_t) ((ctx->State >> 8) ^ (uint32_t)((b << 8) ^ (b << 3) ^ (b >> 4)));
            --temp_len;
        }

        while(0 != temp_len);
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}

ptxStatus_t ptx30wCrc_Result(ptx30wCrc_t *ctx, uint16_t *result)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (NULL != ctx) && (NULL != result) )
    {
        switch(ctx->Type)
        {
            case ptx30wCrcType_NfcA:
                *result = ctx->State;
                break;
            case ptx30wCrcType_NfcB:
                *result = ~ctx->State;
                break;
            default:
                status = ptxStatus_InvalidParameter;
                break;
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}
