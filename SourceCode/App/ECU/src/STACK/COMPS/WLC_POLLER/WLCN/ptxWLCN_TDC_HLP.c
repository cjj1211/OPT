/** \file
   ---------------------------------------------------------------
   Copyright (C) 2022 Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : WLCN
    File        : ptxWLCN_TDC_HLP.h

    Description : Example Implementation of a very light weight high level protocol on top
                    of the TDC.
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxWLCN_Extension.h"
#include "ptxWLCN_TDC_HLP.h"
#include <string.h>

#define PTX_PAYLOAD_LEN         (62u)
#define PTX_REQ_FRAME_LEN       (0x05)
#define PTX_WRITE_REQ_OPC       (0xA5)
#define PTX_READ_REQ_OPC        (0xA6)
#define PTX_HLP_TIMEOUT         (35u)

ptxStatus_t ptxWLCN_TDC_HlpRx(ptxWLCN_t *wlcn, uint8_t *rxData, uint32_t *rxDataLen)
{
    ptxStatus_t status = ptxStatus_Success;

    uint8_t frame_buffer[63];
    uint8_t frame_buffer_len = sizeof(frame_buffer);

    /**
     * Assemble buffer for initial 'handshake'.
     * Indicate that we want to request data from the listener.
     */
    frame_buffer[0] = PTX_READ_REQ_OPC;
    /**
     * The poller sends the size information of his buffer to the listener.
     * Not evaluated by the listener YET!
     */
    frame_buffer[1] = (uint8_t) ((*rxDataLen >> 16) & 0xFF);
    frame_buffer[2] = (uint8_t) ((*rxDataLen >> 8)  & 0xFF);
    frame_buffer[3] = (uint8_t) ((*rxDataLen >> 0)  & 0xFF);
    /** End the frame again the request data opcode. */
    frame_buffer[4] = PTX_READ_REQ_OPC;

    /** Transmit the initial frame of the handshake.
     * We wait a maximum of 50ms for the listener to read the frame.
     */
    status = ptxWLCN_TDC_Write(wlcn, frame_buffer, PTX_REQ_FRAME_LEN, PTX_HLP_TIMEOUT);

    if(ptxStatus_Success == status)
    {
        /**
         * In case initial frame was successfully sent,
         * the listener must respond with the data length it
         * wants to send to us! */
        frame_buffer_len = PTX_REQ_FRAME_LEN;

        /**
         * We wait a maximum of 35ms again to receive the frame. */
        status = ptxWLCN_TDC_Read(wlcn, frame_buffer, &frame_buffer_len, PTX_HLP_TIMEOUT);

        /** Validate the frame (it has the same structure as the initial frame). */
        if(     (ptxStatus_Success == status)
            &&  (PTX_REQ_FRAME_LEN == frame_buffer_len)
            &&  (PTX_READ_REQ_OPC == frame_buffer[0])
            &&  (PTX_READ_REQ_OPC == frame_buffer[4]) )
        {
            uint32_t bytes_to_read = 0;
            /** Retrieve the 24-bit length information. */
            bytes_to_read |= ((frame_buffer[1] << 16) & 0xFF0000);
            bytes_to_read |= ((frame_buffer[2] << 8)  & 0x00FF00);
            bytes_to_read |= ((frame_buffer[3] << 0)  & 0x0000FF);

            uint32_t rx_data_offset = 0;
            uint32_t pending_len = bytes_to_read;

            /** Calculate the amount of frames to receive. */
            uint32_t frames_to_read = (pending_len / PTX_PAYLOAD_LEN) + 1u;

            /** Receive the data frame by frame. */
            while( (ptxStatus_Success == status) && (0 != frames_to_read) )
            {

                frame_buffer_len = sizeof(frame_buffer);
                status = ptxWLCN_TDC_Read(wlcn, frame_buffer, &frame_buffer_len, PTX_HLP_TIMEOUT);

                /**
                 * TDC frames have a maximum length of 63 bytes of payload. In this simple protocol the very first byte
                 * of each frame contains the actual frame number (wraps around every 256 frames). So, 62 bytes of 'real'
                 * payload are left for usage.
                 */
                if(         (ptxStatus_Success == status)
                        &&  (0 != frame_buffer_len)                                     /**< Check if we received something. */
                        &&  (frame_buffer[0] == (rx_data_offset / PTX_PAYLOAD_LEN)) )   /**< Validate frame number. */
                {
                    /** Minus 1 due to frame number byte. */
                    uint8_t actual_payload = (uint8_t) (frame_buffer_len - 1u);
                    /** Copy the frame into the correct destination buffer location. */
                    memcpy(&rxData[rx_data_offset], &frame_buffer[1], actual_payload);
                    /** Update all the helper variables. */
                    pending_len -= actual_payload;
                    rx_data_offset += actual_payload;
                    --frames_to_read;
                }
                else
                {
                    status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_ProtocolError);
                }
            }

            if(ptxStatus_Success == status)
            {
                /** Set the number of received bytes. */
                *rxDataLen = bytes_to_read;
            }
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_TimeOut);
    }

    if(ptxStatus_Success != status)
    {
        *rxDataLen = 0;
    }

    return status;
}


ptxStatus_t ptxWLCN_TDC_HlpTx(ptxWLCN_t *wlcn, uint8_t *txData, uint32_t txDataLen)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN)) && (NULL != txData) && (0xFFFFFF >= txDataLen) )
    {

        uint8_t frame_buffer[63];
        /**
         * Assemble buffer for initial 'handshake'.
         * Indicate that we want to send data to the listener.
         */
        frame_buffer[0] = PTX_WRITE_REQ_OPC;
        /** The poller sends the size information of the amount of data to the listener. */
        frame_buffer[1] = (uint8_t) ((txDataLen >> 16) & 0xFF);
        frame_buffer[2] = (uint8_t) ((txDataLen >> 8)  & 0xFF);
        frame_buffer[3] = (uint8_t) ((txDataLen >> 0)  & 0xFF);
        /** End the frame again the send data opcode. */
        frame_buffer[4] = PTX_WRITE_REQ_OPC;

        /** Transmit the initial frame of the handshake. */
        /** We wait a maximum of 50ms for the listener to read the frame. */
        status = ptxWLCN_TDC_Write(wlcn, frame_buffer, PTX_REQ_FRAME_LEN, PTX_HLP_TIMEOUT);
        printf("ptxWLCN_TDC_Write: status = 0x%x\n", status);

        if(ptxStatus_Success == status)
        {
            uint8_t frame_buffer_len = PTX_REQ_FRAME_LEN;
            status = ptxWLCN_TDC_Read(wlcn, frame_buffer, &frame_buffer_len, PTX_HLP_TIMEOUT);
            printf("ptxWLCN_TDC_Read: status = 0x%x\n", status);

            /** Validate the frame (it has the same structure as the initial frame). */
            if(         (ptxStatus_Success == status)
                    &&  (PTX_REQ_FRAME_LEN == frame_buffer_len)
                    &&  (PTX_WRITE_REQ_OPC == frame_buffer[0])
                    &&  (PTX_WRITE_REQ_OPC == frame_buffer[4]) )
            {

                /** Check the HLP buffer size of the listener. */
                uint32_t listener_buffer_size = 0;
                /** Retrieve the 24-bit length information. */
                listener_buffer_size |= ((frame_buffer[1] << 16) & 0xFF0000);
                listener_buffer_size |= ((frame_buffer[2] << 8)  & 0x00FF00);
                listener_buffer_size |= ((frame_buffer[3] << 0)  & 0x0000FF);

                /** Only start wiht the transmission if listeners HLP buffer is large enough. */
                if(listener_buffer_size >= txDataLen)
                {
                    uint32_t tx_data_offset = 0;
                    uint32_t pending_len = txDataLen;
                    /** Calculate the amount of frames to send. */
                    uint32_t frames_to_write = (pending_len / PTX_PAYLOAD_LEN) + 1u;
                    /** Write frame by frame in a loop. */
                    while ( (ptxStatus_Success == status) && (0 != frames_to_write) )
                    {
                        uint8_t payload_len = (uint8_t)(pending_len > PTX_PAYLOAD_LEN ? PTX_PAYLOAD_LEN : pending_len);
                        /** Frame number. */
                        frame_buffer[0] = (uint8_t) (tx_data_offset / PTX_PAYLOAD_LEN);
                        /** Copy payload. */
                        memcpy(&frame_buffer[1], &txData[tx_data_offset], payload_len);
                        /** Write to listener. */
                        status = ptxWLCN_TDC_Write(wlcn, frame_buffer, payload_len + 1u, PTX_HLP_TIMEOUT);
                        /** Update all the helper variables. */
                        pending_len -= payload_len;
                        tx_data_offset += payload_len;
                        --frames_to_write;
                    }
                }
                else
                {
                    status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InsufficientResources);  /**< Buffer size on listener side too small. */
                }
            }
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}
