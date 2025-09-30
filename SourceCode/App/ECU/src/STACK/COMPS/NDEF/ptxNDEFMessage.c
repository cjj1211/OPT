/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
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
ptxStatus_t ptxNDEFMessage_Parse (uint8_t *srcBuffer, uint32_t bufferLen, ptxNDEFRecord_t *recordBuffer, uint32_t *recordLen, uint8_t ignoreMBME)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != srcBuffer) && (NULL != recordBuffer) && (NULL != recordLen))
    {
        uint32_t record_len_cpy = *recordLen;
        uint8_t  record_num = 0;
        uint8_t *src_buffer_cpy = srcBuffer;
        uint8_t *src_buffer_end = (uint8_t*)(src_buffer_cpy + bufferLen);
        uint8_t  is_chunked = 0;
        uint8_t  me = 0;

        while(!me && (src_buffer_cpy < src_buffer_end))
        {

            if(record_len_cpy <= record_num)
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InsufficientResources);
                break;
            }

            uint8_t header_byte = *src_buffer_cpy;
            src_buffer_cpy++;

            uint8_t mb = header_byte & FLAG_MASK_MB;
            me = header_byte & FLAG_MASK_ME;
            uint8_t cf = header_byte & FLAG_MASK_CF;
            uint8_t sr = header_byte & FLAG_MASK_SR;
            uint8_t il = header_byte & FLAG_MASK_IL;
            uint8_t tnf = header_byte & FLAG_MASK_TNF;

            if((0 == record_num) && !mb && !ignoreMBME)
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
                break;
            }

            if((0 != record_num) && mb && !ignoreMBME)
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
                break;
            }

            if(me && cf && mb && !ignoreMBME)
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
                break;
            }

            if(!is_chunked && cf && (TNF_UNCHANGED == tnf))
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
                break;
            }

            if(is_chunked && (TNF_UNCHANGED != tnf))
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
                break;
            }

            if(is_chunked && il)
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
                break;
            }

            ptxNDEFRecord_SetMB(&recordBuffer[record_num], mb);
            ptxNDEFRecord_SetME(&recordBuffer[record_num], me);
            ptxNDEFRecord_SetCF(&recordBuffer[record_num], cf);
            ptxNDEFRecord_SetSR(&recordBuffer[record_num], sr);
            ptxNDEFRecord_SetIL(&recordBuffer[record_num], il);
            ptxNDEFRecord_SetTNF(&recordBuffer[record_num], tnf);

            recordBuffer[record_num].Type.Length = (*src_buffer_cpy) & 0xFF;
            src_buffer_cpy++;

            if(is_chunked && (recordBuffer[record_num].Type.Length != 0))
            {
                status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
                break;
            }

            uint32_t payload_length = 0;
            if (recordBuffer[record_num].HeaderFlags.SR)
            {
                payload_length = *src_buffer_cpy & 0xFF;
                src_buffer_cpy++;
            } else
            {
                payload_length = *((uint32_t*)src_buffer_cpy);
                src_buffer_cpy += 4;
            }

            recordBuffer[record_num].Payload.Length = payload_length;

            if(recordBuffer[record_num].HeaderFlags.IL)
            {
                recordBuffer[record_num].ID.Length = *src_buffer_cpy;
                src_buffer_cpy++;
            } else
            {
                recordBuffer[record_num].ID.Length = 0;
            }

            is_chunked = cf;

            if (!is_chunked)
            {
                if(0 != recordBuffer[record_num].Type.Length)
                {
                    recordBuffer[record_num].Type.bData = src_buffer_cpy;
                    src_buffer_cpy += recordBuffer[record_num].Type.Length;
                } else
                {
                    recordBuffer[record_num].Type.bData = NULL;
                }

                if(0 != recordBuffer[record_num].ID.Length)
                {
                    recordBuffer[record_num].ID.Data = src_buffer_cpy;
                    src_buffer_cpy += recordBuffer[record_num].ID.Length;
                } else
                {
                    recordBuffer[record_num].ID.Data = NULL;
                }
            }

            recordBuffer[record_num].Payload.Data = src_buffer_cpy;
            src_buffer_cpy += recordBuffer[record_num].Payload.Length;
            record_num++;
        }

        uint32_t read_len = (uint32_t)(src_buffer_cpy - srcBuffer);
        if(read_len < bufferLen)
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
        }

        *recordLen = record_num;
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFMessage_Create (ptxNDEFRecord_t *recordBuffer, uint32_t recordSize, uint8_t *dstBuffer, uint32_t *bufferLen)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != recordBuffer) && (NULL != dstBuffer) && (NULL != bufferLen))
    {
        uint32_t record_num = 0;
        uint32_t data_written = 0;
        uint8_t *buffer_cpy = dstBuffer;
        uint32_t bufferLenCpy = *bufferLen;

        while(record_num < recordSize)
        {
            if(0 == record_num)
            {
                ptxNDEFRecord_SetMB(&recordBuffer[record_num], 1);
            }
            if(recordSize == (record_num + 1))
            {
                ptxNDEFRecord_SetME(&recordBuffer[record_num], 1);
            }

            uint32_t record_size;
            status = ptxNDEFRecord_Write(&recordBuffer[record_num], &buffer_cpy[data_written], bufferLenCpy - data_written, &record_size);
            if(ptxStatus_Success != status)
            {
                break;
            }

            data_written += record_size;
            record_num++;
        }

        if(ptxStatus_Success == status)
        {
            *bufferLen = data_written;
        }
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFMessage_FilterRecordsByType (ptxNDEFRecord_t *recordBuffer, uint32_t *recordsLen, char* type, uint8_t typeLen, uint32_t *foundRecords)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != recordBuffer) && (NULL != recordsLen) && (NULL != type) && (NULL != foundRecords))
    {
        uint32_t num_found_records = 0;
        for(uint32_t i = 0; i < *recordsLen; ++i)
        {
            if( (recordBuffer[i].Type.Length == typeLen) && (0 == strncmp(recordBuffer[i].Type.sData, type, typeLen)) )
            {
                foundRecords[num_found_records] = i;
                num_found_records++;
            }
        }
        *recordsLen = num_found_records;
    } else
    {
       status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}
