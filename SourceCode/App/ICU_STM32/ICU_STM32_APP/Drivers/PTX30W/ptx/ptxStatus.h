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
    Module      : STATUS
    File        : ptxStatus.h

    Description : Status codes
*/
#ifndef COMPS_PTXSTATUS_H_
#define COMPS_PTXSTATUS_H_

#include <stdint.h>

/**
 * \brief Status Code Definitions
 */
typedef enum ptxStatus_Values
{
    ptxStatus_Success,                 /**< Internal The operation completed successfully. */
    ptxStatus_InvalidParameter,        /**< Invalid value(s) for function parameter(s). */
    ptxStatus_InternalError,           /**< There has been internal error in the function processing. */
    ptxStatus_NotImplemented,          /**< The function/command is not implemented. */
    ptxStatus_TimeOut,                 /**< The operation has timed out. */
    ptxStatus_InterfaceError,          /**< The interface (I/O line, UART, ...) is not accessible or an error
                                            has occurred. */
    ptxStatus_NotPermitted,            /**< The operation is not permitted. */
    ptxStatus_NscProtocolError,        /**< Error at NSC protocol. */
    ptxStatus_InsufficientResources,   /**< Insufficient Resources Error. */
    ptxStatus_ProtocolError,           /**< General protocol error. */
    ptxStatus_InvalidCommand,          /**< Command not supported at this point in time. */
    ptxStatus_NvmError,                /**< Error writing into the NVM. */
    ptxStatus_WrongHardware,           /**< The hardware version of the PTX30W doesn't match. */
    ptxStatus_NoAcknowledge,           /**< No ACK-Frame received. */
    ptxStatus_MAX                      /**< Maximum count. */
} ptxStatus_t;

#endif /* Guard */
