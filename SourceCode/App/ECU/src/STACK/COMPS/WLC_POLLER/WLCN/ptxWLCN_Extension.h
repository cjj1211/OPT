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
    File        : ptxWLCN_Extension.h

    Description : This part of the API provides all the PTX proprietary
                  functionality like the Transparent Data Channel (TDC),
                  the control of the PTX30Ws GPIOs, and the readout of
                  the temperature status of the PTX30W.
*/
/**
 * \addtogroup grp_wlcn_api_ext PTX WLC EXT API
 *
 * @{
 */
#ifndef COMPS_WLC_POLLER_WLCN_WLCN_EXTENSION_H_
#define COMPS_WLC_POLLER_WLCN_WLCN_EXTENSION_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxStatus.h"
#include "ptxWLCN.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define CMD_NSC_DATA_MSG        (0x80)
#define CMD_NFC_FORUM_T2T_READ  (0x30)
#define CMD_PTX_PROP_READ       (0x3F)

#define CMD_NFC_FORUM_T2T_WRITE (0xA2)
#define CMD_PTX_PROP_WRITE      (0xAF)

#define PTX_READ_LEN            (64u)
#define PTX_WRITE_LEN           (64u)

#define BLOCK_SIZE              (4u)
#define BLOCK_NUM_POL_BUFF      (48u)
#define BLOCK_NUM_LIS_BUFF      (64u)
#define BLOCK_NUM_GPIO_CTRL     (47u)
#define BLOCK_NUM_STATUS_INFO   (80u)

#define PTX_BUFFER_LEN_MASK     (0x3F)
#define PTX_MAX_BUFFER_LEN      (0x3F)

#define PTX_HEADER_LEN          (4u)
#define LEN_NFC_FORUM_T2T_READ  (16u)
#define DEFAULT_TIMEOUT         (20u)

/**
 * \brief PTX30W NTC Status
 */
typedef enum ptxWLCN_NTC_Status
{
    E_Hot   = 0x0C, /**< NTC Status: Extra Hot */
    Hot     = 0x04, /**< NTC Status: Hot */
    Normal  = 0x00, /**< NTC Status: Normal */
    Cold    = 0x02, /**< NTC Status: Cold */
    E_Cold  = 0x03  /**< NTC Status: Extra Cold */
} ptxWLCN_NTC_Status_t;

/**
 * \brief Writes messages to the listener's buffer. Customer can choose between NFC Forum compliant write access
 *          and proprietary write access (= faster) via compile switch (#define TDC_NFC_FORUM_COMPLIANT).
 *
 * \param[in]       wlcn            Pointer to the WLCN Component.
 * \param[in]       txData          Pointer to the data which shall be transmitted.
 * \param[in]       txLen           Size of the buffer.
 * \param[in]       ackTimeoutMs    Timeout for the message to be received.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TDC_Write(ptxWLCN_t *wlcn, uint8_t *txData, uint8_t txLen, uint32_t ackTimeoutMs);

/**
 * \brief Reads messages from the listener's buffer. Customer can choose between NFC Forum compliant read access
 *          and proprietary read access (= faster) via compile switch (#define TDC_NFC_FORUM_COMPLIANT).
 *
 * \param[in]       wlcn        Pointer to WLCN Component.
 * \param[in,out]   rxData      Pointer to a buffer for storing the received data.
 * \param[in,out]   rxDataLen   Size of the buffer.
 * \param[in]       rxTimeoutMs Timeout in ms for the transaction.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TDC_Read(ptxWLCN_t *wlcn, uint8_t *rxData, uint8_t *rxDataLen, uint32_t rxTimeoutMs);

/**
 * \brief Checks if the Listener's host MCU has read the previously sent message.
 *
 * \param[in]       wlcn        Pointer to WLCN Component.
 * \param[in,out]   received    Pointer to store the status, if the message has been read by the listener.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TDC_IsReceived(ptxWLCN_t *wlcn, uint8_t *received);

/**
 * \brief Assembles and transmits the NFC Forum compliant T2T_READ_CMD capable of reading 16 bytes at once from the listener.
 *
 * \param[in]       wlcn        Pointer to WLCN Component.
 * \param[in]       block       Block number to read the data from.
 * \param[in,out]   rxData      Pointer to a buffer for storing the received data.
 * \param[in,out]   rxDataLen   Pointer to store the received length.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TDC_ReadT2T(ptxWLCN_t *wlcn, uint8_t block, uint8_t *rxData, uint32_t *rxDataLen);

/**
 * \brief Assembles and transmits the NFC Forum compliant T2T_WRITE_CMD capable of writing 4 bytes at once to the listener.
 *
 * \param[in]   wlcn    Pointer to WLCN Component.
 * \param[in]   block   Block number to write the data to.
 * \param[in]   txData  Pointer to the data to be written.
 * \param[in]   txLen   Length of the data to be written.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TDC_WriteT2T(ptxWLCN_t *wlcn, uint8_t block, uint8_t *txData, uint32_t txLen);

/**
 * \brief Assembles and transmits the proprietary PTX_READ_CMD capable of reading 64 bytes at once from the listener.
 *
 * \param[in]       wlcn        Pointer to WLCN Component.
 * \param[in]       block       Block number to read the data from.
 * \param[in,out]   rxData      Pointer to a buffer for storing the received data.
 * \param[in,out]   rxDataLen   Pointer to store the received length.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TDC_ReadPTX(ptxWLCN_t *wlcn, uint8_t block, uint8_t *rxData, uint32_t *rxDataLen);

/**
 * \brief Assembles and transmits the proprietary PTX_WRITE_CMD capable of writing 64 bytes at once to the listener.
 *
 * \param[in]   wlcn    Pointer to WLCN Component.
 * \param[in]   block   Block number to write the data to.
 * \param[in]   txData  Pointer to the data to be written.
 * \param[in]   txLen   Length of the data to be written.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_TDC_WritePTX(ptxWLCN_t *wlcn, uint8_t block, uint8_t *txData, uint32_t txLen);

/**
 * \brief Controls the output level of the PTX30W's GPIOs. The PTX30W's GPIOs must be configured to "WlcpControlled".
 *
 *
 * \param[in]       wlcn            Pointer to WLCN component.
 * \param[in]       setGpio0High    Sets PTX30W's GPIO_0 to HIGH when true.
 * \param[in]       setGpio1High    Sets PTX30W's GPIO_1 to HIGH when true.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_SetListenerGpios(ptxWLCN_t *wlcn, bool setGpio0High, bool setGpio1High);

/**
 * \brief Retrieves detailed temperature information from the PTX30W.
 *
 * \param[in]       wlcn            Pointer to WLCN component.
 * \param[in,out]   statusInfo      Pointer to store the temperature information.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_GetNtcStatus(ptxWLCN_t *wlcn, ptxWLCN_NTC_Status_t *statusInfo);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */
