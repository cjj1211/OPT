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
    File        : ptxNDEFRecord_BT_SP.h

    Description : NDEF BT SP record API
*/

#ifndef APIS_PTX_NDEF_RECORD_BT_H_
#define APIS_PTX_NDEF_RECORD_BT_H_

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
#define TYPE_RTD_BT             ("application/vnd.bluetooth.ep.oob")
#define TYPE_RTD_BT_LEN         (32u)
#define PAYLOAD_ID              ("0")
#define MAC_ADDRESS_LEN         (6u) /* expected size of MAC address*/
#define BT_OOB_DATA_LEN_BYTES   (2u) /* bytes used for OOB data length */
#define BT_EIR_DATA_BYTES       (2u) /* bytes used for one EIR, data length and data type */   

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

ptxStatus_t ptxNDEFRecordBTSP_Create (ptxNDEFRecord_t *record, uint8_t *workBuffer, size_t *workBufferLen, uint8_t* btDeviceAddressLen, char* btDeviceName);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
