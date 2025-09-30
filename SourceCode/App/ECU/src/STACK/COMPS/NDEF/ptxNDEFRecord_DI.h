/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : NDEF Device Information Record
    File        : ptxNDEFRecord_DI.h

    Description : NDEF Device Information record API
*/

/**
 * \addtogroup grp_ptx_api_ndef_record_di NDEF Device Information Record API
 *
 * @{
 */

#ifndef APIS_PTX_NDEF_RECORD_DEVICEINFORMATION_H_
#define APIS_PTX_NDEF_RECORD_DEVICEINFORMATION_H_

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
#define TYPE_RTD_DEVICE_INFO     ("Di")
#define TYPE_RTD_DEVICE_INFO_LEN (2u)

#define PTX_NDEF_RECORD_DEVICEINFORMATION_MAXLEN 1036
/*
* ####################################################################################################################
* TYPES
* ####################################################################################################################
*/

/**
 * \brief Device Information record TLV type.
 */
typedef enum ptxNDEFRecord_DI_Type
{
    DI_Type_ManufacturerName = 0x00,
    DI_Type_ModelName        = 0x01,
    DI_Type_DeviceUniqueName = 0x02,
    DI_Type_UUID             = 0x03,
    DI_Type_FirmwareVersion  = 0x04,
    DI_Type_RFU              = 0x05
} ptxNDEFRecord_DI_Type_t;

/**
 * \brief Device Information record generic Length Value type.
 */
typedef struct ptxNDEFRecord_DI_LV
{
    uint8_t length;
    char    *value;
} ptxNDEFRecord_DI_LV_t;

/**
 * \brief Device Information record UUID Length Value type.
 */
typedef struct ptxNDEFRecord_DI_LV_UUID
{
    uint8_t length;
    uint8_t *value;
} ptxNDEFRecord_DI_LV_UUID_t;

/**
 * \brief Device Information record container.
 */
typedef struct ptxNDEFRecord_DI
{
    ptxNDEFRecord_DI_LV_t      manufacturerName;   /**< required to be set */
    ptxNDEFRecord_DI_LV_t      modelName;          /**< required to be set */
    ptxNDEFRecord_DI_LV_t      deviceUniqueName;   /**< optional */
    ptxNDEFRecord_DI_LV_UUID_t uuid;               /**< optional */
    ptxNDEFRecord_DI_LV_t      firmwareVersion;    /**< optional */
} ptxNDEFRecord_DI_t;


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Sets the device information container to zero.
 *
 * \param[in]       container       Pointer to the device information record.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_Init (ptxNDEFRecord_DI_t *container);

/**
 * \brief Creates an NDEF record container from the given ptxNDEFRecord_DI_t struct.
 *
 * \param[in,out]   record          Pointer to an uninitialized NDEF record structure.
 * \param[in]       container       Pointer to the initialized ptxNDEFRecord_DI_t container.
 * \param[in,out]   payloadBuffer   Pointer to a payload buffer for storing the data of dynamic length.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_Create (ptxNDEFRecord_t *record, ptxNDEFRecord_DI_t *container, uint8_t payloadBuffer[PTX_NDEF_RECORD_DEVICEINFORMATION_MAXLEN]);

/**
 * \brief Parses a given NDEF record as Device Information record and stores it in the ptxNDEFRecord_DI_t container.
 *
 * \param[in]       srcRecord       Pointer to the NDEF record to be parsed.
 * \param[out]      dst             Pointer to the device information record for storing the parsed data.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_Parse (ptxNDEFRecord_t *srcRecord, ptxNDEFRecord_DI_t *dst);

/**
 * \brief Sets the mandatory manufacturer name field within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       manufacturerName    Pointer to the manufacturer character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_SetManufacturerName (ptxNDEFRecord_DI_t *container, char *manufacturerName);

/**
 * \brief Sets the mandatory model name field within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       modelName       Pointer to the manufacturer character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_SetModelName (ptxNDEFRecord_DI_t *container, char *modelName);

/**
 * \brief Sets the optional device unique name field within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       deviceUniqueName    Pointer to the unique name character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_SetDeviceUniqueName (ptxNDEFRecord_DI_t *container, char *deviceUniqueName);

/**
 * \brief Sets the optional UUID field within the device information container and updates its TLV.
 *
 * \param[in]       container   Pointer to the Device Information container to be updated.
 * \param[in]       uuid        Pointer to the 16 byte UUID.
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_SetUUID (ptxNDEFRecord_DI_t *container, uint8_t uuid[16]);

/**
 * \brief Sets the optional firmware version name within the device information container and updates its TLV.
 *
 * \param[in]       container           Pointer to the Device Information container to be updated.
 * \param[in]       firmwareVersion     Pointer to the unique name character array (must be null terminated).
 *
 * \return Status, indicating whether the operation was successful.
 */
ptxStatus_t ptxNDEFRecordDi_SetFirmwareVersion (ptxNDEFRecord_DI_t *record, char *firmwareVersion);


#ifdef __cplusplus
}
#endif

/** @} */

#endif /* Guard */
