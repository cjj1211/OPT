/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : WLCN
    File        : ptxWLCN_Extension.c

    Description :
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxWLCN_Extension.h"
#include "ptxStatus.h"
#include <string.h>

//#define TDC_NFC_FORUM_COMPLIANT

ptxStatus_t ptxWLCN_TDC_Write(ptxWLCN_t *wlcn, uint8_t *txData, uint8_t txLen, uint32_t ackTimeoutMs)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != txData) && (PTX_MAX_BUFFER_LEN >= txLen) )
    {
#ifdef TDC_NFC_FORUM_COMPLIANT
        size_t buffer_pos = BLOCK_SIZE - 1u; /**< Subtract header byte. */
        uint8_t block = BLOCK_NUM_POL_BUFF + 1u;
        size_t req_writ_op = txLen / BLOCK_SIZE;

        /**
         * The poller buffer within the listener's memory starts on block 48. As soon as block 48 gets modified,
         * by the poller, the listener will indicate the modification to the it's host MCU by throwing an interrupt.
         * Hence, we need to make sure, to write the complete remaining payload FIRST, and only afterwards we update
         * block 48!
         * Using the T2T write command, we can only write 4 bytes at once!
         */
        while( (ptxStatus_Success == status) &&  (0u != req_writ_op) )
        {
            /** Write the data in chunks of 4 bytes (NFC_FORUM compliant). */
            status = ptxWLCN_TDC_WriteT2T(wlcn, block++, &txData[buffer_pos], BLOCK_SIZE);
            /** Increment block number. */
            buffer_pos += BLOCK_SIZE;
            /** Decrement number of remaining write operations. */
            --req_writ_op;
        }

        /** Write the remaining data into block 48. */
        if(ptxStatus_Success == status)
        {
            uint8_t header_block[BLOCK_SIZE] = {0};
            /** The first byte in block 48 indicates the number of written bytes to the listener. */
            header_block[0] = CMD_NSC_DATA_MSG | txLen;
            /** Copy remaining data into reserved memory. */
            memcpy(&header_block[1], &txData[0], BLOCK_SIZE - 1u);
            /** Write the data in chunks of 4 bytes (NFC_FORUM compliant). */
            status = ptxWLCN_TDC_WriteT2T(wlcn, BLOCK_NUM_POL_BUFF, &header_block[0], 4);
        }
#else
        /** Reserve 64 bytes at once. */
        uint8_t header_block[PTX_WRITE_LEN] = {0};
        /** The first byte in block 48 indicates the number of written bytes to the listener. */
        header_block[0] = CMD_NSC_DATA_MSG | txLen;
        /** Copy remaining data into reserved memory. */
        memcpy(&header_block[1], &txData[0], txLen);

        /** Write the data using one single write operation (PTX proprietary command). */
        status = ptxWLCN_TDC_WritePTX(wlcn, BLOCK_NUM_POL_BUFF, header_block, txLen + 1u);
#endif

        /** Check if we want to wait for the read acknowledge. */
        if( (ptxStatus_Success == status) && (0 != ackTimeoutMs) )
        {
            uint8_t received;
            TimerStatus_t timer_status;
            /** Prepare the timer. */
            (void) ptxPERIPH_APPTIMER_Start(ackTimeoutMs);
            do
            {
                /** Did the listener host MCU read the package? */
                status = ptxWLCN_TDC_IsReceived(wlcn, &received);
                /** Check the timer status. */
                (void) ptxPERIPH_APPTIMER_Status(&timer_status);
            } while( (ptxStatus_Success == status) && (0 == received) && (0 == timer_status.IsElapsed) );

            (void) ptxPERIPH_APPTIMER_Stop();

            /** Did the listener's host MCU read the package within the given timout? */
            if(0 == received)
            {
                status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_TimeOut);
            }
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_TDC_IsReceived(ptxWLCN_t *wlcn, uint8_t *received)
{
    ptxStatus_t status = ptxStatus_MAX;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != received))
    {
        /** Explicitly set "received" variable to zero. */
        *received = 0;


        /** Reserve memory for read operation. */
        uint8_t read_data[LEN_NFC_FORUM_T2T_READ];
        uint32_t read_data_len = sizeof(read_data);
#ifdef TDC_NFC_FORUM_COMPLIANT

        /** Read the first block of the poller buffer on the listener device. */
        status = ptxWLCN_TDC_ReadT2T (wlcn, BLOCK_NUM_POL_BUFF, read_data, &read_data_len);
#else
        read_data_len = 4;
        /** Do the proprietary read operation (reads 4 bytes per transaction instead of 16 byte). */
        status = ptxWLCN_TDC_ReadPTX(wlcn, BLOCK_NUM_POL_BUFF, read_data, &read_data_len);
#endif
        /**
         * In case the host MCU (of the listener) has read the message, the first byte in the
         * poller buffer (indicating the message length) will be set to zero.
         */
        if ( (ptxStatus_Success == status) && (0u == read_data[0]) )
        {
            *received = 1;
        }

        return status;
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_TDC_Read(ptxWLCN_t *wlcn, uint8_t *rxData, uint8_t *rxDataLen, uint32_t rxTimeoutMs)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != rxData) && (NULL != rxDataLen))
    {
        uint8_t rx_data[PTX_READ_LEN];

        /** Do we actually need the timer? Customer has the choice. */
        if(0 != rxTimeoutMs)
        {
            status = ptxPERIPH_APPTIMER_Start(rxTimeoutMs);
        }

        uint32_t rx_data_len;
        TimerStatus_t timer_status;
        do
        {
#ifdef TDC_NFC_FORUM_COMPLIANT
            size_t buffer_pos = LEN_NFC_FORUM_T2T_READ;
            rx_data_len = LEN_NFC_FORUM_T2T_READ;
            uint8_t block_num = BLOCK_NUM_LIS_BUFF;

            /** Use the 'standard' T2T read command to retrieve the data (16 bytes per transaction). */
            status = ptxWLCN_TDC_ReadT2T(wlcn, block_num, rx_data, &rx_data_len);

            size_t req_read_op = (rx_data[0] & PTX_BUFFER_LEN_MASK) / LEN_NFC_FORUM_T2T_READ;
            while( (ptxStatus_Success == status) &&  (0u != req_read_op) )
            {
                block_num += (LEN_NFC_FORUM_T2T_READ / BLOCK_SIZE);
                rx_data_len = LEN_NFC_FORUM_T2T_READ;
                status = ptxWLCN_TDC_ReadT2T(wlcn, block_num, &rx_data[buffer_pos], &rx_data_len);

                buffer_pos += LEN_NFC_FORUM_T2T_READ;
                --req_read_op;
            }
#else
            rx_data_len = BLOCK_SIZE;
            /** Do the proprietary read operation (max. 64 bytes per transaction). */
            status = ptxWLCN_TDC_ReadPTX(wlcn, BLOCK_NUM_LIS_BUFF, rx_data, &rx_data_len);
            uint8_t req_read_op = (rx_data[0] & PTX_BUFFER_LEN_MASK);

            if(req_read_op > (BLOCK_SIZE - 1u))
            {
                rx_data_len = req_read_op + 1; // + header_byte
                status = ptxWLCN_TDC_ReadPTX(wlcn, BLOCK_NUM_LIS_BUFF, rx_data, &rx_data_len);
            }
#endif
            /** Check the timer status. Did it elapse?*/
            ptxPERIPH_APPTIMER_Status(&timer_status);

            /** Read out the until time elapses or until (rx_data[0] != 0). */
        } while(    (ptxStatus_Success == status)
                &&  (0 == (rx_data[0] & PTX_BUFFER_LEN_MASK))
                &&  (0 != rxTimeoutMs)
                &&  (0 == timer_status.IsElapsed) );

        /** If the timer was running, we need to stop it! */
        if(0 != rxTimeoutMs)
        {
            (void) ptxPERIPH_APPTIMER_Stop();
        }

        /** If everything was successful, copy the relevant information. */
        if(ptxStatus_Success == status)
        {
            *rxDataLen = rx_data[0] & PTX_BUFFER_LEN_MASK;
            memcpy(rxData, &rx_data[1], *rxDataLen);
        }
        else
        {
            *rxDataLen = 0; /**< Explicitly set to 0 in case of error. */
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_TDC_ReadT2T(ptxWLCN_t *wlcn, uint8_t block, uint8_t *rxData, uint32_t *rxDataLen)
{
    ptxStatus_t status = ptxStatus_MAX;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != rxData) && (NULL != rxDataLen) && (LEN_NFC_FORUM_T2T_READ <= *rxDataLen))
    {
        uint8_t tx_buffer[2];
        uint32_t tx_buffer_len = sizeof(tx_buffer);

        uint8_t rx_buffer[LEN_NFC_FORUM_T2T_READ + 1u];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        tx_buffer[0] = CMD_NFC_FORUM_T2T_READ;
        tx_buffer[1] = block;

        status = ptxIoTRd_Data_Exchange(&wlcn->IotRd, tx_buffer, tx_buffer_len, rx_buffer, &rx_buffer_len, DEFAULT_TIMEOUT);

        if ( (ptxStatus_Success == status) && ((LEN_NFC_FORUM_T2T_READ + 1u) == rx_buffer_len) && (0u == rx_buffer[rx_buffer_len - 1]) )
        {
            rx_buffer_len = rx_buffer_len - 1u;
            memcpy(rxData, rx_buffer, rx_buffer_len);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_TDC_WriteT2T(ptxWLCN_t *wlcn, uint8_t block, uint8_t *txData, uint32_t txLen)
{
    ptxStatus_t status = ptxStatus_MAX;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != txData))
    {
        uint8_t rx_buffer[2];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        uint8_t tx_buffer[6];
        uint32_t tx_buffer_len = sizeof(tx_buffer);
        tx_buffer[0] = CMD_NFC_FORUM_T2T_WRITE;
        tx_buffer[1] = block;
        memcpy(&tx_buffer[2], txData, txLen);

        status = ptxIoTRd_Data_Exchange(&wlcn->IotRd, tx_buffer, tx_buffer_len, rx_buffer, &rx_buffer_len, DEFAULT_TIMEOUT);

        if ( (ptxStatus_Success == status) && ((2u != rx_buffer_len) || (4u != rx_buffer[rx_buffer_len - 1])) )
        {
            status = ptxStatus_ProtocolError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_TDC_ReadPTX(ptxWLCN_t *wlcn, uint8_t block, uint8_t *rxData, uint32_t *rxDataLen)
{
    ptxStatus_t status = ptxStatus_MAX;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != rxData) && (NULL != rxDataLen))
    {
        uint8_t tx_buffer[3];
        uint32_t tx_buffer_len = sizeof(tx_buffer);

        tx_buffer[0] = CMD_PTX_PROP_READ;
        tx_buffer[1] = block;
        tx_buffer[2] = (uint8_t) ((*rxDataLen / BLOCK_SIZE) + (*rxDataLen % BLOCK_SIZE ? 1u : 0u));

        uint8_t rx_buffer[PTX_READ_LEN + 1u];
        uint32_t rx_buffer_len = sizeof(rx_buffer);
        status = ptxIoTRd_Data_Exchange(&wlcn->IotRd, tx_buffer, tx_buffer_len, rx_buffer, &rx_buffer_len, DEFAULT_TIMEOUT);

        if ( (ptxStatus_Success == status) && (((tx_buffer[2] * BLOCK_SIZE) + 1u) == rx_buffer_len))
        {
           *rxDataLen = rx_buffer_len - 1u;
           memcpy(rxData, rx_buffer, *rxDataLen);
        }
        else
        {
            *rxDataLen = 0;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_TDC_WritePTX(ptxWLCN_t *wlcn, uint8_t block, uint8_t *txData, uint32_t txLen)
{
    ptxStatus_t status = ptxStatus_MAX;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != txData) && (txLen <= PTX_WRITE_LEN))
    {
        uint8_t rx_buffer[2];
        uint32_t rx_buffer_len = sizeof(rx_buffer);

        uint8_t blocks = (uint8_t) ((txLen / BLOCK_SIZE) + (txLen % BLOCK_SIZE ? 1u : 0u));

        uint8_t tx_buffer[PTX_WRITE_LEN + 2u];
        memset(tx_buffer, 0, sizeof(tx_buffer));

        tx_buffer[0] = CMD_PTX_PROP_WRITE;
        tx_buffer[1] = block;
        memcpy(&tx_buffer[2], txData, txLen);

        uint8_t data_to_transmit = (uint8_t) (blocks * BLOCK_SIZE) + 2u; /** accommodate for the two additional header bytes */
        status = ptxIoTRd_Data_Exchange(&wlcn->IotRd, tx_buffer, data_to_transmit, rx_buffer, &rx_buffer_len, DEFAULT_TIMEOUT);

        if ( (ptxStatus_Success == status) && ((2u != rx_buffer_len) || (4u != rx_buffer[rx_buffer_len - 1])) )
        {
            status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_ProtocolError);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_SetListenerGpios(ptxWLCN_t *wlcn, bool setGpio0High, bool setGpio1High)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN))
    {
        uint8_t tx_buffer[BLOCK_SIZE];
        uint32_t tx_buffer_len = sizeof(tx_buffer);

        tx_buffer[0] = (setGpio0High ? 0x00 : 0x01); /** Payload byte 0 controlling GPIO0. */
        tx_buffer[1] = (setGpio1High ? 0x00 : 0x01); /** Payload byte 1 controlling GPIO1. */
        tx_buffer[2] = 0x00; /** Payload byte 2 unused. (Required for NFC Forum compliance.)  */
        tx_buffer[3] = 0x00; /** Payload byte 3 unused. (Required for NFC Forum compliance.)  */

        status = ptxWLCN_TDC_WriteT2T(wlcn, BLOCK_NUM_GPIO_CTRL, tx_buffer, tx_buffer_len);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_GetNtcStatus(ptxWLCN_t *wlcn, ptxWLCN_NTC_Status_t *statusInfo)
{
    ptxStatus_t status = ptxStatus_MAX;
    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != statusInfo))
    {
        /** Reserve memory for read operation. */
        uint8_t read_data[LEN_NFC_FORUM_T2T_READ];
        uint32_t read_data_len = sizeof(read_data);

#ifdef TDC_NFC_FORUM_COMPLIANT
        status = ptxWLCN_TDC_ReadT2T(wlcn, BLOCK_NUM_STATUS_INFO, read_data, &read_data_len);
#else
        read_data_len = BLOCK_SIZE;
        status = ptxWLCN_TDC_ReadPTX(wlcn, BLOCK_NUM_STATUS_INFO, read_data, &read_data_len);
#endif

        if (ptxStatus_Success == status)
        {
            *statusInfo = (ptxWLCN_NTC_Status_t) read_data[0];
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }
    return status;
}
