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
    Module      : NDEF Bluetooth Record Simple Pairing
    File        : ptxNDEFRecord_BT_SP.c

    Description : NDEF BT SP record API
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxNDEFRecord_BT_SP.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxNDEFRecordBTSP_Create (ptxNDEFRecord_t* record, uint8_t* workBuffer, size_t* workBufferLen, uint8_t* btDeviceAddress, char* btDeviceName)
{
   ptxStatus_t status = ptxStatus_MAX;

    if((NULL != record) && (NULL != btDeviceAddress) && (NULL != workBuffer))
    {
        uint8_t payload_len = (uint8_t) (BT_OOB_DATA_LEN_BYTES + MAC_ADDRESS_LEN + BT_EIR_DATA_BYTES + ((uint8_t) strlen(btDeviceName)));
        uint8_t index = 0;

        if (*workBufferLen >= payload_len)
        {
            /* Flip oob data length. 2 bytes needed but only one in use, sufficient here. (device name would have to exceed 245 bytes) */
            workBuffer[index] = payload_len;
            index++;
            workBuffer[index] = 0x00;
            index++;

            /* Flip BT MAC address */
            for (uint8_t i = 0; i < MAC_ADDRESS_LEN; ++i)
            {
                workBuffer[index] = btDeviceAddress[MAC_ADDRESS_LEN - (i + 1)];
                index++;
            }

            /* EIR data length: length BT name + 1 (data type) */
            workBuffer[index] = (uint8_t) (strlen(btDeviceName) + 1);
            index++;

            /* EIR data type: Complete local name */
            workBuffer[index] = 0x09;
            index++;

            /* BT local device name */
            strcpy((char *) &workBuffer[index], btDeviceName);
            index = (uint8_t) (index + strlen(btDeviceName));

            status = ptxNDEFRecord_Create(record, TNF_MEDIA_TYPE, (uint8_t*) TYPE_RTD_BT, TYPE_RTD_BT_LEN, (uint8_t*) PAYLOAD_ID, 1u, workBuffer, payload_len);
        }
        else 
        {
            status = ptxStatus_InsufficientResources;
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}
