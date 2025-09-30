/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : NDEF WLC STATUS INFORMATION RECORD API
    File        : ptxNDEFRecord_WLCSTAI.h

    Description : WLC Status and Info Record API
*/

/**
 * \addtogroup grp_ptx_api_ndef_record_wlcstai NDEF WLCSTAI Record API
 *
 * @{
 */

#ifndef APIS_PTX_NDEF_RECORD_WLCSTAI_H_
#define APIS_PTX_NDEF_RECORD_WLCSTAI_H_

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
/**
* \brief Record type definition for WLC Status and Information record.
*/
#define TYPE_RTD_WLCSTAI        ("WLCSTAI")
#define TYPE_RTD_WLCSTAI_LEN    (7u)

/** CONTROL_BYTE_1 Field */
#define MASK_WLCSTAI_CTRL1_BATLVL   0x01 /**< CTRL1 BATTLVL set mask */
#define MASK_WLCSTAI_CTRL1_RECPWR   0x02 /**< CTRL1 RECPWR set mask */
#define MASK_WLCSTAI_CTRL1_RECVLT   0x04 /**< CTRL1 RECVLT set mask */
#define MASK_WLCSTAI_CTRL1_RECCUR   0x08 /**< CTRL1 RECCUR set mask */
#define MASK_WLCSTAI_CTRL1_TEMPBAT  0x10 /**< CTRL1 TEMPBAT set mask */
#define MASK_WLCSTAI_CTRL1_TEMPWLCL 0x20 /**< CTRL1 TEMPWLCL set mask */
#define MASK_WLCSTAI_CTRL1_RFU      0x40 /**< CTRL1 RFU */
#define MASK_WLCSTAI_CTRL1_CTRL2    0x80 /**< CTRL1 CTRL2 set mask */

/** CONTROL_BYTE_2 Field */
#define MASK_WLCSTAI_CTRL2_BATVLT   0x01 /**< CTRL2 BATTVLT set mask */
#define MASK_WLCSTAI_CTRL2_BATCUR   0x02 /**< CTRL2 BATTCUR set mask */

/** BATTERY_LEVEL Field */
#define MASK_WLCSTAI_BATLVL         0x7F /**< BATLVL mask */
#define MASK_WLCSTAI_BATLVL_MAX     0x64 /**< BATLVL maximum value */
#define MASK_WLCSTAI_BATLVL_POS     0x00 /**< BATLVL position */

/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Structure representing the WLC Status and Info Record payload.
 */
typedef struct ptxWLCSTAI
{
    uint8_t CONTROL;
    uint8_t BATTERY_LEVEL;
    uint8_t RECEIVE_POWER;
    uint8_t RECEIVE_VOLTAGE;
    uint8_t RECEIVE_CURRENT;
    int8_t  TEMPERATURE_BATTERY;
    int8_t  TEMPERATURE_WLCL;
    uint8_t RFU;
    uint8_t CONTROL2;
    uint8_t BATTERY_VOLTAGE;
    uint8_t BATTERY_CURRENT;
} ptxWLCSTAI_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
/**
 * \brief Initializes the WLCSTAI container with zeros.
 *
 * \param[in]   container   Pointer to the uninitialized ptxWLCSTAI_t container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_Init (ptxWLCSTAI_t *container);

/**
 * \brief Create an NDEF record container from the given ptxWLCSTAI_t struct.
 *
 * \param[in,out]   record      Pointer to an uninitialized NDEF record structure.
 * \param[in]       container   Pointer to the initialized WLCSTAI container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_Create(ptxNDEFRecord_t *record, ptxWLCSTAI_t *container);

/**
 * \brief Extracts the NDEF record payload into the ptxWLCSTAI_t container.
 *
 * \param[in]       record      Pointer to an NDEF record structure.
 * \param[in,out]   container   Pointer to the uninitialized WLCSTAI container.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_Parse(ptxNDEFRecord_t *record, ptxWLCSTAI_t *container);

/**
 * \brief Setter for the WLCSTAI battery level.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       percent     Battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetBatteryLevel(ptxWLCSTAI_t *container, uint8_t percent);

/**
 * \brief Getter for the WLCSTAI battery level measured by the listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   percent     Valid pointer to store the battery level in percent.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetBatteryLevel(ptxWLCSTAI_t *container, uint8_t *percent);

/**
 * \brief Setter for the WLCSTAI power level.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       power       Power level.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetReceivePower(ptxWLCSTAI_t *container, uint8_t power);

/**
 * \brief Getter for the WLCSTAI received power by the listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   power       Valid pointer to store the power.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetReceivePower(ptxWLCSTAI_t *container, uint8_t *power);

/**
 * \brief Setter for the WLCSTAI voltage level.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       voltage     Voltage level.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetReceiveVoltage(ptxWLCSTAI_t *container, uint8_t voltage);

/**
 * \brief Getter for the WLCSTAI received voltage by the listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   voltage     Valid pointer to store the voltage.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetReceiveVoltage(ptxWLCSTAI_t *container, uint8_t *voltage);

/**
 * \brief Setter for the WLCSTAI current.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       current     Current value.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetReceiveCurrent(ptxWLCSTAI_t *container, uint8_t current);

/**
 * \brief Getter for the WLCSTAI received current by the listener.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   current     Valid pointer to store the current.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetReceiveCurrent(ptxWLCSTAI_t *container, uint8_t *current);

/**
 * \brief Setter for the WLCSTAI battery temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       current     Temperature in celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetTemperatureBat(ptxWLCSTAI_t *container, int8_t celsius);

/**
 * \brief Getter for the WLCSTAI battery temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   celsius     Valid pointer to store the temperature in degree celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetTemperatureBat(ptxWLCSTAI_t *container, int8_t *celsius);

/**
 * \brief Setter for the WLCSTAI listener temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       current     Temperature in celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetTemperatureWlcl(ptxWLCSTAI_t *container, int8_t celsius);

/**
 * \brief Getter for the WLCSTAI listener temperature.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   celsius     Valid pointer to store the temperature in degree celsius.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetTemperatureWlcl(ptxWLCSTAI_t *container, int8_t *celsius);

/**
 * \brief Setter for the WLCSTAI battery voltage.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       voltage     Battery voltage.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetBatteryVoltage(ptxWLCSTAI_t *container, uint8_t voltage);

/**
 * \brief Getter for the WLCSTAI battery voltage.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   voltage     Valid pointer to store the battery voltage.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetBatteryVoltage(ptxWLCSTAI_t *container, uint8_t *voltage);

/**
 * \brief Setter for the WLCSTAI battery voltage.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in]       current     Battery charge current.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_SetBatteryCurrent(ptxWLCSTAI_t *container, uint8_t current);

/**
 * \brief Getter for the WLCSTAI battery current.
 *
 * \param[in]       container   Pointer to the WLCSTAI container.
 * \param[in,out]   current     Valid pointer to store the battery charge current.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxWLCSTAI_GetBatteryCurrent(ptxWLCSTAI_t *container, uint8_t *current);

#ifdef __cplusplus
}
#endif

/** @} */

#endif /* Guard */
