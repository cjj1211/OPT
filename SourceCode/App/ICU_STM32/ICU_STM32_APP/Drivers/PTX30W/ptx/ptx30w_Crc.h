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
    File        : ptx30w_Crc.h
*/
#ifndef PTX_PTX30W_CRC_H_
#define PTX_PTX30W_CRC_H_

#include <stdbool.h>
#include <stddef.h>
#include "ptxStatus.h"

#ifdef __cplusplus
extern "C"
{
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
typedef enum ptx30wCrcType
{
    ptx30wCrcType_NfcA,
    ptx30wCrcType_NfcB
} ptx30wCrcType_t;

typedef struct
{
    ptx30wCrcType_t Type;
    uint16_t        State;
} ptx30wCrc_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the internal context data structure.
 *
 * \param[in] ctx  Valid pointer to a \ref ptx30wCrc_t data structure.
 * \param[in] type CRC type used for calculation (\ref ptx30wCrcType_t).
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30wCrc_Init(ptx30wCrc_t *ctx, ptx30wCrcType_t type);

/**
 * \brief Used to feed new data into the CRC calculation.
 *
 * \param[in] ctx     Pointer to the \ref ptx30wCrc_t data structure.
 * \param[in] data    Pointer to the data, which shall be processed.
 * \param[in] dataLen Length of the data to be processed.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30wCrc_Update(ptx30wCrc_t *ctx, uint8_t *data, size_t dataLen);

/**
 * \brief Retrieves the result of the CRC calculation.
 *
 * \param[in]  ctx    Pointer to the \ref ptx30wCrc_t data structure.
 * \param[out] result Pointer to store the CRC result.
 *
 * \return Status of the operation see \ref ptxStatus_t
 */
ptxStatus_t ptx30wCrc_Result(ptx30wCrc_t *ctx, uint16_t *result);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
