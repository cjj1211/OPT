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
    File        : ptxNDEFRecord_DI.c

    Description : NDEF Device Information record implementation
*/


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */

#include "ptxNDEFRecord_DI.h"

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */
ptxStatus_t ptxNDEFRecordDi_Init (ptxNDEFRecord_DI_t *container)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != container)
    {
        memset(container, 0, sizeof(ptxNDEFRecord_DI_t));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFRecordDi_Create (ptxNDEFRecord_t *record, ptxNDEFRecord_DI_t *container, uint8_t *payloadBuffer)
{
    ptxStatus_t status = ptxStatus_Success;

    uint8_t length;
    uint8_t *ptr = payloadBuffer;

    if((record != NULL) && (container != NULL) && (payloadBuffer != NULL)) /**< check if source and destination are valid */
    {
        if((container->manufacturerName.length > 0) && (container->modelName.length > 0)) /**< check if mandatory parameters are specified */
        {
            /** Prepare NDEF Record Header */
            ptxNDEFRecord_Create(record, TNF_WELL_KNOWN_TYPE, (uint8_t*) TYPE_RTD_DEVICE_INFO, TYPE_RTD_DEVICE_INFO_LEN, NULL, 0, NULL, 0);

            *ptr = DI_Type_ManufacturerName;                    /**< specify type */
            ptr++;
            length = container->manufacturerName.length;
            *ptr = length;                                      /**< specify length */
            ptr++;
            strncpy((char *) ptr, container->manufacturerName.value, length); /**< copy value into new memory region */
            ptr += length;

            *ptr = DI_Type_ModelName;                           /**< specify type */
            ptr++;
            length = container->modelName.length;               /**< specify length */
            *ptr = length;
            ptr++;
            strncpy((char *) ptr, container->modelName.value, length); /**< copy value into new memory region */
            ptr += length;


            if(0 < container->deviceUniqueName.length)         /**< check if optional device unique name is specified */
            {
                /** Create datastructure for device unique name within available memory area (TLV) */
                *ptr = DI_Type_DeviceUniqueName;                /**< specify type */
                ptr++;
                length = container->deviceUniqueName.length;
                *ptr = length;                                  /**< specify length */
                ptr++;
                strncpy((char *) ptr, container->deviceUniqueName.value, length); /**< copy value into new memory region */
                ptr += length;
            }

            if(0 < container->uuid.length)                     /**< check if optional device unique name is specified */
            {
                /** Create datastructure for UUID within available memory area (TLV) */
                *ptr = DI_Type_UUID;                            /**< specify type */
                ptr++;
                length = container->uuid.length;               /**< UUID has fixed length of 16 byte */
                *ptr = length;                                  /**< specify length */
                ptr++;
                memcpy(ptr, container->uuid.value, length);    /**< copy value into new memory region */
                ptr += length;
            }

            if(0 < container->firmwareVersion.length)         /**< check if optional device unique name is specified */
            {
                /** Create datastructure for UUID version */
                *ptr = DI_Type_FirmwareVersion;                 /**< specify type */
                ptr++;
                length = container->firmwareVersion.length;
                *ptr = length;                                  /**< specify length */
                ptr++;
                strncpy((char *) ptr, container->firmwareVersion.value, length); /**< copy value into new memory region */
                ptr += length;
            }
            record->Payload.Data = payloadBuffer;
            record->Payload.Length = (uint32_t) (ptr - payloadBuffer);
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFRecordDi_Parse (ptxNDEFRecord_t *srcRecord, ptxNDEFRecord_DI_t *dstRecord)
{
    ptxStatus_t status = ptxStatus_Success;

    uint8_t manufacturer_name_set = 0;
    uint8_t model_name_set = 0;

    if((NULL != srcRecord) && (NULL != srcRecord->Payload.Data) && (NULL != dstRecord))
    {
        uint8_t *ptr = srcRecord->Payload.Data;
        while(((uint32_t)(ptr - srcRecord->Payload.Data)) < srcRecord->Payload.Length)
        {
            ptxNDEFRecord_DI_Type_t type = *ptr;
            ptr++;
            switch(type)
            {
                case DI_Type_ManufacturerName:
                    dstRecord->manufacturerName.length = *ptr;
                    ptr++;
                    dstRecord->manufacturerName.value = (char*) ptr;
                    ptr += dstRecord->manufacturerName.length;
                    manufacturer_name_set = 1;
                    break;

                case DI_Type_ModelName:
                    dstRecord->modelName.length = *ptr;
                    ptr++;
                    dstRecord->modelName.value = (char*) ptr;
                    ptr += dstRecord->modelName.length;
                    model_name_set = 1;
                    break;

                case DI_Type_DeviceUniqueName:
                    dstRecord->deviceUniqueName.length = *ptr;
                    ptr++;
                    dstRecord->deviceUniqueName.value = (char*) ptr;
                    ptr += dstRecord->deviceUniqueName.length;
                    break;

                case DI_Type_UUID:
                    dstRecord->uuid.length = *ptr;
                    ptr++;
                    dstRecord->uuid.value = ptr;
                    ptr += dstRecord->uuid.length;
                    break;

                case DI_Type_FirmwareVersion:
                    dstRecord->firmwareVersion.length = *ptr;
                    ptr++;
                    dstRecord->firmwareVersion.value = (char*) ptr;
                    ptr += dstRecord->firmwareVersion.length;
                    break;

                default: /** Ignore TLV elements that have type codes we don't support. */
                {
                    uint8_t length = *ptr;
                    ptr += length + 1;
                    break;
                }
            }
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InternalError);
    }

    if(0 == manufacturer_name_set || 0 == model_name_set) /** check if mandatory parameters are specified */
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFRecordDi_SetManufacturerName (ptxNDEFRecord_DI_t *container, char *manufacturerName)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != manufacturerName))
    {
        uint32_t len = strlen(manufacturerName);
        if(len < 256u)
        {
            container->manufacturerName.value = manufacturerName;
            container->manufacturerName.length = (uint8_t) len;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFRecordDi_SetModelName (ptxNDEFRecord_DI_t *container, char *modelName)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != modelName))
    {
        uint32_t len = strlen(modelName);
        if(len < 256u)
        {
            container->modelName.value = modelName;
            container->modelName.length = (uint8_t) len;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFRecordDi_SetDeviceUniqueName (ptxNDEFRecord_DI_t *container, char *deviceUniqueName)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != deviceUniqueName))
    {
        uint32_t len = strlen(deviceUniqueName);
        if(len < 256u)
        {
            container->deviceUniqueName.value = deviceUniqueName;
            container->deviceUniqueName.length = (uint8_t) len;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
        }
    }

    return status;
}

ptxStatus_t ptxNDEFRecordDi_SetUUID (ptxNDEFRecord_DI_t *container, uint8_t uuid[16])
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != uuid))
    {
        container->uuid.value = uuid;
        container->uuid.length = (uint8_t) 16;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxNDEFRecordDi_SetFirmwareVersion (ptxNDEFRecord_DI_t *container, char *firmwareVersion)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != container) && (NULL != firmwareVersion))
    {
        uint32_t len = strlen(firmwareVersion);
        if(len < 256u)
        {
            container->firmwareVersion.value = firmwareVersion;
            container->firmwareVersion.length = (uint8_t) len;
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InsufficientResources);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_NDEF, ptxStatus_InvalidParameter);
    }

    return status;
}
