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
    File        : ptxWLCN.c

    Description :
*/

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxWLCN.h"
#include "ptxStatus.h"
#include "ptxPLAT.h"
#include <string.h>

const uint8_t DEFAULT_WB_LU_TABLE[] = PTX_WAVEBANK_DEFAULT_LU_TABLE;

/**
 * \brief Maximum count of records allowed on a tag.
 */
#define POLLER_NDEF_RECORD_BUFFER_SIZE (32)

/**
 * \brief Callback function for WPT process.
 */
static void ptxWLCN_WPT_Callback(void *ctx, void *event, uint8_t *pld, size_t pldLen);

/**
 * \brief Calculates the required power step for the listener and
 *        implements the necessary boundary checks.
 */
static ptxStatus_t ptxWLCN_UpdatePwrLevel (ptxWLCN_t *wlcn);

/**
 * \brief Adds an additional 10% timeout margin to the given timeout parameter.
 */
static void ptxWLCN_AddTimeoutSafetyMargin(uint32_t *timeout);

/**
 * \brief Calculates the required wavebank for the next WB cycle depending on the content of WLC_CTL.
 */
static ptxStatus_t ptxWLCN_WptWbScaling(ptxWLCN_t *wlcn);

/**
* \brief Calculates the required wavebank for NF-communication.
*/
static ptxStatus_t ptxWLCN_NfcWbScaling (ptxWLCN_t *wlcn);

/**
 * \brief Initializes internal members of the wlcn struct.
 */
static ptxStatus_t ptxWLCN_InitInternals(ptxWLCN_t *wlcn);

/**
 * \brief Checks if the given card is available in card registry.
 */
static ptxStatus_t ptxWLCN_ListenerAvailable(ptxWLCN_t *wlcn, ptxIoTRd_CardParams_t cardToFind, uint8_t *isAvailable);

/**
 * \brief Selects the communication protocol, to communicate with a given listener/card.
 */
static ptxStatus_t ptxWLCN_SelectProtocol(ptxIoTRd_CardParams_t cardParams,  ptxIoTRd_CardProtocol_t *protocol);

/**
 * \brief Helper function for reading & parsing an NDEF message into individual records.
 */
static ptxStatus_t ptxWLCN_RetrieveNdefMessage(ptxWLCN_t *wlcn, ptxNDEFRecord_t *records, uint32_t *recordsLen);

/**
 * \brief Helper function for building & writing an NDEF message from a composition of individual records.
 */
static ptxStatus_t ptxWLCN_WriteNdefMessage(ptxWLCN_t *wlcn, ptxNDEFRecord_t *records, uint32_t recordsLen);

/**
 * \brief Wrapper for application callbacks.
 */
static void ptxWLCN_AppCallback_event(ptxWLCN_t *wlcn, ptxWLCN_Appevent_t appEvent, void *data, size_t dataLen);

/**
 * \brief Internal function for retrieving WLCN NSC events.
 */
static ptxNSC_WLC_Events_t ptxWLCN_WPT_GetEvent(ptxWLCN_t *wlcn);


/**
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptxStatus_t ptxWLCN_Init (ptxWLCN_t *wlcn, ptxWLCN_InitParams_t *initParam)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (NULL != wlcn) && (NULL != initParam) )
    {
        /** Let's initialize the IoT Reader */
        ptxIoTRd_InitPars_t init_params;
        ptxIoTRd_ComInterface_Params_t com_interface;

        (void) memset(&init_params, 0, sizeof(ptxIoTRd_InitPars_t));

        /** Define communication interface settings: speed and flow control. Flow control is relevant only for Uart. */
        com_interface.Speed = PTX_IOTRD_HOST_SPEED_SPI_1M;

        /** Initial parameters for temperature sensor are ready. */
        init_params.TemperatureSensor = &initParam->TempSens;
        init_params.ComInterface = &com_interface;
        init_params.ExtProtection = &initParam->ExternalProtection;

        status = ptxIoTRd_Init(&wlcn->IotRd, &init_params);

        if (ptxStatus_Success == status)
        {
            status = ptxWPT_Init(&wlcn->Wpt, wlcn->IotRd.Nsc, &ptxWLCN_WPT_Callback, wlcn);

            if (ptxStatus_Success == status)
            {
                memcpy(&wlcn->WlcnInitParams, initParam, sizeof(ptxWLCN_InitParams_t));

                wlcn->CompId = ptxStatus_Comp_WLCN;

                ptxWLCN_InitInternals(wlcn);

                /** Set default wavebank lookup table (sine wave). */
                ptxWLCN_SetWptWbLuTable(wlcn, DEFAULT_WB_LU_TABLE);
                ptxWLCN_SetNfcWbLuTable(wlcn, DEFAULT_WB_LU_TABLE);

                /** Calculate wavebanks from lookup table. */
                ptxWLCN_WptWbScaling(wlcn);
                ptxWLCN_NfcWbScaling(wlcn);
            }
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_UpdateChipConfig (ptxWLCN_t *wlcn, ptxNSC_WLC_Config_Parameters wlcConfigParams)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) )
    {
        /** Let's get the MISC Section where the WLC parameters are going to be written. */
        status = ptxWPT_UpdateChipConfig(&wlcn->Wpt, wlcConfigParams);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_NFCDeInit (ptxWLCN_t *wlcn)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) )
    {
        (void) ptxPERIPH_APPTIMER_Stop();

        status = ptxIoTRd_Reader_Deactivation(&wlcn->IotRd, 0u);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_Deinit (ptxWLCN_t *wlcn)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN))
    {
        /**
         * Depending on in which state the user deinitializes the the stack, the APPTIMER may still be running.
         */
        ptxPERIPH_APPTIMER_Stop();

        /** First of all NSC SW Reset. */
        (void) ptxIoTRd_SWReset(&wlcn->IotRd);

        /** Let's de-init WPT. */
        (void) ptxWPT_Deinit(&wlcn->Wpt);

        /** Let's de-init IoT Reader. */
        (void) ptxIoTRd_Deinit(&wlcn->IotRd);

        /** At this point, Let's clear wlcn. */
        (void) memset(wlcn, 0, sizeof(ptxWLCN_t));

    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_SetWptWbLuTable (ptxWLCN_t *wlcn, const uint8_t *table)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != table) )
    {
        for(uint8_t i = 0; i < PTX_WAVEBANK_LEN; ++i)
        {
            wlcn->WptWbLuTable[(PTX_WAVEBANK_LEN - 1) - i] = table[i];
        }
        status = ptxWLCN_WptWbScaling(wlcn);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_SetNfcWbLuTable (ptxWLCN_t *wlcn, const uint8_t *table)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != table) )
    {
        for(uint8_t i = 0; i < PTX_WAVEBANK_LEN; ++i)
        {
            wlcn->NfcWbLuTable[(PTX_WAVEBANK_LEN - 1) - i] = table[i];
        }
        status = ptxWLCN_NfcWbScaling(wlcn);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_GetWptWbLuTable (ptxWLCN_t *wlcn, uint8_t *table)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != table) )
    {
        for(uint8_t i = 0; i < PTX_WAVEBANK_LEN; ++i)
        {
            table[(PTX_WAVEBANK_LEN - 1) - i] = wlcn->WptWbLuTable[i];
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_GetNfcWbLuTable (ptxWLCN_t *wlcn, uint8_t *table)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != table) )
    {
        for(uint8_t i = 0; i < PTX_WAVEBANK_LEN; ++i)
        {
            table[(PTX_WAVEBANK_LEN - 1) - i] = wlcn->NfcWbLuTable[i];
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_GetWptWb (ptxWLCN_t *wlcn, uint8_t *wb)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != wb) )
    {
        for(uint8_t i = 0; i < PTX_WAVEBANK_LEN; ++i)
        {
            wb[(PTX_WAVEBANK_LEN - 1) - i] = wlcn->WptWavebank[i];
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_GetNfcWb (ptxWLCN_t *wlcn, uint8_t *wb)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != wb) )
    {
        for(uint8_t i = 0; i < PTX_WAVEBANK_LEN; ++i)
        {
            wb[(PTX_WAVEBANK_LEN - 1) - i] = wlcn->NfcWavebank[i];
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_IsPtxListener (ptxWLCN_t *wlcn, uint8_t *isPtx)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != isPtx) )
    {
        *isPtx = 0u;
        /** Check, if there are any cards available. */
        if( (0u != wlcn->IotRd.CardRegistry->NrCards)
         && (Tech_TypeA == wlcn->IotRd.CardRegistry->ActiveCard->TechType)
         && (PTX_MANUFACTURER_CODE == wlcn->IotRd.CardRegistry->ActiveCard->TechParams.CardAParams.NFCID1[0]) )
        {
            *isPtx = 1u;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

/*
 * ####################################################################################################################
 * CALLBACK FUNCTION
 * ####################################################################################################################
 */

static void ptxWLCN_WPT_Callback (void *ctx, void *event, uint8_t *pld, size_t pldLen)
{
    if ( (NULL != ctx) && (NULL != event) )
    {
        ptxNSC_WLC_Events_t *event_loc = (ptxNSC_WLC_Events_t*)event;
        ptxWLCN_t *wlcn = (ptxWLCN_t *)ctx;

        if (PowerTransfer_NoEvent != *event_loc)
        {
            wlcn->NscWlcEvent = *event_loc;
        }

        (void) pld;
        (void) pldLen;
    }
}

/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

ptxStatus_t ptxWLCN_State_WaitForListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        /** RF-Discover configuration */
        ptxIoTRd_DiscConfig_t rf_disc_config;
        (void) memset(&rf_disc_config, 0, sizeof(ptxIoTRd_DiscConfig_t));

        rf_disc_config.PollTypeA        = 1u;
        rf_disc_config.PollTypeB        = 1u;
        rf_disc_config.PollTypeF212     = 1u;
        rf_disc_config.PollTypeF424     = 1u;
        rf_disc_config.PollTypeV        = 1u;
        rf_disc_config.PollGuardTime    = 0u;
        rf_disc_config.EnableStandBy    = wlcn->WlcnInitParams.PollingLpcdStandbyEnable;
        rf_disc_config.Discover_Mode    = wlcn->WlcnInitParams.PollingLpcdLoops;
        rf_disc_config.EnableLPCDNotification = wlcn->WlcnInitParams.PollingLpcdNtfEnable;
        rf_disc_config.PollGuardTime    = wlcn->WlcnInitParams.StartUpDelay;

        if (0 == wlcn->WlcnInitParams.PollingLpcdLoops)
        {
            rf_disc_config.IdleTime = wlcn->WlcnInitParams.PollingCycleIdleTime;
        }
        else
        {
            rf_disc_config.IdleTime = wlcn->WlcnInitParams.PollingLpcdIdleTime;
        }

        status = ptxIoTRd_Initiate_Discovery (&wlcn->IotRd, &rf_disc_config);

        if(ptxStatus_Success == status)
        {
            *chargingState = PollListener;
        }
        else
        {
            *chargingState = SystemError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_PollListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        uint8_t disc_status = RF_DISCOVER_STATUS_NO_CARD;

        /** Get discovered status */
        (void) ptxIoTRd_Get_Status_Info (&wlcn->IotRd, StatusType_Discover, &disc_status);

        if(wlcn->IotRd.CardRegistry->NrCards > 0u)
        {
            ptxWLCN_AppCallback_event(wlcn, ListenerDetected, &wlcn->IotRd.CardRegistry->NrCards, sizeof(uint8_t));
        }

        if(RF_DISCOVER_STATUS_CARD_ACTIVE == disc_status)
        {
            ptxWLCN_InitInternals(wlcn);
            wlcn->CardRegistryId = 0u;

            *chargingState = GetCapRecord;
        }
        else if ( (RF_DISCOVER_STATUS_DISCOVER_RUNNING == disc_status) || (RF_DISCOVER_STATUS_DISCOVER_DONE == disc_status) )
        {
            *chargingState = ActivateListener;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_ActivateListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {

        /** Check, if we went through all existing cards in the card registry. */
        if(wlcn->CardRegistryId == (wlcn->IotRd.CardRegistry->NrCards - 1))
        {
            /** Reset variable */
            wlcn->CardRegistryId = PTX_CARD_REGISTRY_UNDEFINED;
        }

        for(uint8_t i = (uint8_t) (wlcn->CardRegistryId + 1); i < wlcn->IotRd.CardRegistry->NrCards; i++)
        {
            ptxIoTRd_CardProtocol_t protocol;
            ptxIoTRd_CardParams_t card = wlcn->IotRd.CardRegistry->Cards[i];

            /** Select the protocol, to talk to the listener. */
            ptxWLCN_SelectProtocol(card, &protocol);

            if(Prot_Undefined != protocol)
            {
                status = ptxIoTRd_Activate_Card(&wlcn->IotRd, &card, protocol);

                if(ptxStatus_Success == status)
                {
                    ptxWLCN_InitInternals(wlcn);
                    /** Activation succeeded; Store current ID and try to retrieve CAP field. */
                    wlcn->CardRegistryId = i;

                    *chargingState = GetCapRecord;
                }
                else
                {
                    /** In case activation fails, start from the beginning*/
                    wlcn->CardRegistryId = PTX_CARD_REGISTRY_UNDEFINED;

                    *chargingState = DeactivateListener;
                    ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                }
                break;
            }
        }

        /** If chargingState is still set to ActivateListener, none of the existent cards support the right
         * protocol -> deactivate again. */
        if(ActivateListener == *chargingState)
        {
            *chargingState = DeactivateListener;
            ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
        }

    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_GetCapRecord (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {

        /** Check if an NDEF message is present on the listener. */
        status = ptxNDEF_CheckMessage(&wlcn->NdefComp);

        if (ptxStatus_Success == status)
        {
            /** Let's reserve memory for the NDEF records located on the listener */
            ptxNDEFRecord_t record_buffer[POLLER_NDEF_RECORD_BUFFER_SIZE];
            uint32_t records_len = sizeof(record_buffer) / sizeof(record_buffer[0]);

            /** Read and parse NDEF message of listener */
            status = ptxWLCN_RetrieveNdefMessage(wlcn, record_buffer, &records_len);

            if (ptxStatus_Success == status)
            {
                /** Reserve some memory for storing the IDs of records of interest (WLCCAP, WLCSTAI, etc.) */
                uint32_t filtered_rec_ids[POLLER_NDEF_RECORD_BUFFER_SIZE];
                uint32_t filtered_rec_len = (records_len < POLLER_NDEF_RECORD_BUFFER_SIZE ? records_len : POLLER_NDEF_RECORD_BUFFER_SIZE);

                /** Find the WLCCAP record within the read NDEF records */
                status = ptxNDEFMessage_FilterRecordsByType(record_buffer, &filtered_rec_len, TYPE_RTD_WLCCAP, TYPE_RTD_WLCCAP_LEN, filtered_rec_ids);

                /** Verify that only one single record of type 'WLCCAP' was found */
                if ((ptxStatus_Success == status) && (1u == filtered_rec_len))
                {
                    /** Parse the WLCCAP record */
                    uint32_t cap_record_id = filtered_rec_ids[0];
                    status = ptxWLCCAP_Parse(&record_buffer[cap_record_id], &wlcn->RecordWlcCap);

                    if (ptxStatus_Success == status)
                    {
                        /** Copy current card/listener information */
                        memcpy(&wlcn->Card, wlcn->IotRd.CardRegistry->ActiveCard, sizeof(ptxIoTRd_CardParams_t));

                        /** Execute 'CapFieldReceived' application callback */
                        ptxWLCN_AppCallback_event(wlcn, CapFieldReceived, &wlcn->RecordWlcCap, sizeof(ptxWLCCAP_t));

                        /** Read any optional records from listener. Optional records are always located after the mandatory ones. */
                        ptxNDEFRecord_t *optional_records = &record_buffer[cap_record_id + 1];
                        uint32_t optional_records_len = records_len - cap_record_id - 1;

                        /** Parse WLCSTAI record (if existent). */
                        ptxStatus_t optional_status;
                        optional_status = ptxNDEFMessage_FilterRecordsByType(optional_records, &optional_records_len, TYPE_RTD_WLCSTAI, TYPE_RTD_WLCSTAI_LEN, filtered_rec_ids);
                        if ((ptxStatus_Success == optional_status) && (1u == optional_records_len))
                        {
                            optional_status = ptxWLCSTAI_Parse(&optional_records[filtered_rec_ids[0]], &wlcn->RecordWlcStai);
                            if(ptxStatus_Success == optional_status)
                            {
                                wlcn->RecordWlcStaiSet = 1u;
                                /** Execute application callback. */
                                ptxWLCN_AppCallback_event(wlcn, StaiFieldReceived, &wlcn->RecordWlcStai, sizeof(ptxWLCSTAI_t));
                            }
                        }
                        optional_records_len = records_len - cap_record_id - 1u;

                        /** Parse DeviceInformation record (if existent). */
                        optional_status = ptxNDEFMessage_FilterRecordsByType(optional_records, &optional_records_len, TYPE_RTD_DEVICE_INFO, TYPE_RTD_DEVICE_INFO_LEN, filtered_rec_ids);
                        if ((ptxStatus_Success == optional_status) && (1u == optional_records_len))
                        {
                            optional_status = ptxNDEFRecordDi_Parse(&optional_records[filtered_rec_ids[0]], &wlcn->RecordDevInfo);
                            if (ptxStatus_Success == optional_status)
                            {
                                wlcn->RecordDevInfoSet = 1u;
                                /** Execute application callback */
                                ptxWLCN_AppCallback_event(wlcn, DeviceInfoFieldReceived, &wlcn->RecordDevInfo, sizeof(ptxNDEFRecord_DI_t));
                            }
                        }

                        /** Start interpreting WLCCAP record. */
                        uint8_t major_prot_version;
                        uint8_t minor_prot_version;
                        ptxWLCCAP_GetProtVersion(&wlcn->RecordWlcCap, &major_prot_version, &minor_prot_version);
                        /** If poller and listener support the same major WLC protocol version number (regardless of minor protocol number), continue. */
                        if (wlcn->WlcnInitParams.ProtVersionMajor == major_prot_version)
                        {
                            ptxWLCMode_t mode;
                            status = ptxWLCCAP_GetMode(&wlcn->RecordWlcCap, &mode);
                            if (ptxStatus_Success == status)
                            {
                                switch(mode)
                                {
                                   case ptxWLCMode_Static:
                                   {
                                       /** go to static charging for CAP_WT, then re-read capability record again. */
                                       *chargingState = StaticCharging;
                                       break;
                                   }
                                   case ptxWLCMode_Negotiated:
                                   {
                                       /** Negotiated mode is only allowed if one single listener is in the field. */
                                       if(1u == wlcn->IotRd.CardRegistry->NrCards)
                                       {
                                           uint8_t nego_wait;
                                           ptxWLCCAP_GetNegWait(&wlcn->RecordWlcCap, &nego_wait);
                                           if (0u == nego_wait)
                                           {
                                               /** Set CAP retry counter to zero. */
                                               wlcn->CntrNwt = 0;

                                               /** Waiting time will be between 1ms and 11ms */
                                               uint8_t wr_wt_int;
                                               ptxWLCCAP_GetWrWtInt(&wlcn->RecordWlcCap, &wr_wt_int);
                                               wr_wt_int++; /**< safety margin */

                                               ptxPLAT_Sleep(wlcn->IotRd.Plat, (uint32_t) wr_wt_int);

                                               *chargingState = SetInfoRecord;
                                           }
                                           else
                                           {
                                               uint8_t n_wt_retries;
                                               ptxWLCCAP_GetNWtRetries(&wlcn->RecordWlcCap, &n_wt_retries);
                                               if (wlcn->CntrNwt > n_wt_retries)
                                               {
                                                   /** Reached maximum number of allowed retries. */
                                                   *chargingState = DeactivateListener;
                                                   ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                                               }
                                               else
                                               {
                                                   /** Listener not ready yet, go to wait state. */
                                                   *chargingState = WaitListenerReady;
                                               }
                                           }
                                       }
                                       else
                                       {
                                           /** In case WLC-L requested negotiated mode, but other WLC-Ls were found during discovery
                                            *  we are not allowed to increase the field strength --> stay in static mode
                                            */
                                           *chargingState = StaticCharging;
                                       }
                                       break;
                                   }
                                   case ptxWLCMode_BattFull:
                                   {
                                       /** Shut down RF field, wait for CAP_WT before turning it on again (except for periodic discovery).*/
                                       *chargingState = FieldOff;
                                       break;
                                   }
                                   case ptxWLCMode_RFU: /** illegal state --> protocol error */
                                   {
                                       *chargingState = DeactivateListener;
                                       ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                                       break;
                                   }
                                }
                            }
                        }
                        else
                        {
                            /** Major Protocol version not matching */
                            *chargingState = DeactivateListener;
                            ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                        }
                    }
                }
                else
                {
                    /** No/Multiple WLCCAP messages existent. */
                    *chargingState = DeactivateListener;
                    ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                }
            }
        }

        if (ptxStatus_Success != status)
        {
            *chargingState = DeactivateListener;
            ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
        }

    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_WaitForListenerReady (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        uint32_t waiting_time_ms;
        ptxWLCCAP_GetCapWtMillis(&wlcn->RecordWlcCap, &waiting_time_ms);

        status = ptxPERIPH_APPTIMER_Start(waiting_time_ms);

        if (ptxStatus_Success == status)
        {
            /** Polling for listener ready. */
            *chargingState = PollListenerReady;
        }
        else
        {
            *chargingState = SystemError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_StaticCharging (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        /** Query cap_wt field */
        uint8_t cap_wt;
        ptxWLCCAP_GetCapWt(&wlcn->RecordWlcCap, &cap_wt);

        /** clear WLC events before starting WPT cycle */
        wlcn->NscWlcEvent = PowerTransfer_NoEvent;

        wlcn->PowerLevel = wlcn->WlcnInitParams.PollerPowerLevelMin;
        /** Static charging uses PollerPowerLevelMin as power level. */
        status = ptxWLCN_WptWbScaling(wlcn);
        status |= ptxWLCN_NfcWbScaling(wlcn);

        if (ptxStatus_Success == status)
        {
            /** Options for WPT cycle. */
            ptxNSC_WLC_PowerTransferOptions_t options;
            options.WbCharging       = wlcn->WptWavebank;
            options.WbPostCharging   = wlcn->NfcWavebank;
            options.WptDuration      = cap_wt;
            options.RffSyncDuration  = wlcn->WlcnInitParams.RffSyncTime;
            options.Bfod             = wlcn->WlcnInitParams.WptBfodEnable;
            options.Bfod_Gt          = wlcn->WlcnInitParams.WptBfodGuardTime;
            options.StopRequest      = wlcn->WlcnInitParams.WptStopReqEnable;

            /** Start power cycle */
            status = ptxWPT_Start(&wlcn->Wpt, &options);

            if (ptxStatus_Success == status)
            {
                /** Power cycle started, let's report to the application. */
                ptxWLCN_AppCallback_event(wlcn, WptCycleStarted, &wlcn->WlcnInitParams.PollerPowerLevelMin, 1u);

                uint32_t timeout_ms;
                /** Retrieve waiting time (=charging time) in milliseconds */
                ptxWLCCAP_GetCapWtMillis(&wlcn->RecordWlcCap, &timeout_ms);
                /** Add 10% safety margin to timeout */
                ptxWLCN_AddTimeoutSafetyMargin(&timeout_ms);

                status = ptxPERIPH_APPTIMER_Start(timeout_ms);

                *chargingState = PollCharging;
            }
        }

        if (ptxStatus_Success != status)
        {
            /**
             * Possible reasons why we got here: ptxWPT_Start failed, or
             *  timer failure
             */
            *chargingState = SystemError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_SetInfoRecord (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        ptxNDEFRecord_t records[2];

        /** Build WLCINF record */
        ptxNDEFRecord_Init(&records[0]);
        ptxWLCINF_Init(&wlcn->RecordWlcInf);
        ptxWLCINF_SetPTX(&wlcn->RecordWlcInf, wlcn->PowerLevel);
        ptxWLCINF_SetWptStopSupport(&wlcn->RecordWlcInf, wlcn->WlcnInitParams.WptStopReqEnable);
        ptxWLCINF_SetPowerClass(&wlcn->RecordWlcInf, wlcn->WlcnInitParams.ProtPowerClass);
        ptxWLCINF_SetTotPowerSteps(&wlcn->RecordWlcInf, 0u); // 0 --> unlimited number of power steps
        ptxWLCINF_SetCurPowerStep(&wlcn->RecordWlcInf, 0u); // 0 --> unlimited number of power steps
        ptxWLCINF_SetNextMinStepInc(&wlcn->RecordWlcInf, wlcn->WlcnInitParams.ProtMinStepIncrease);
        ptxWLCINF_SetNextMinStepDec(&wlcn->RecordWlcInf, wlcn->WlcnInitParams.ProtMinStepDecrease);
        ptxWLCINF_Create(&records[0], &wlcn->RecordWlcInf);

        status = ptxWLCN_WriteNdefMessage(wlcn, records, 1u);

        if (ptxStatus_Success == status)
        {
            uint8_t rd_wt;
            ptxWLCCAP_GetRdWt(&wlcn->RecordWlcCap, &rd_wt);
            /** max possible waiting time is ~2.56s (incl. POLLER_WT_MARGIN_DELAY)*/
            uint32_t waiting_time_ms = (rd_wt * 10u) + wlcn->WlcnInitParams.ProtWtMarginDelay;

            ptxPLAT_Sleep(wlcn->IotRd.Plat, waiting_time_ms);

            *chargingState = GetControlRecord;
        }
        else
        {
            *chargingState = DeactivateListener;
            ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_GetControlRecord (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;
    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        ptxNDEFRecord_t record_buffer[POLLER_NDEF_RECORD_BUFFER_SIZE];
        uint32_t records_len = POLLER_NDEF_RECORD_BUFFER_SIZE;

        /** Read and parse NDEF message of listener.
         * 'records_len' gets updated with the real number of read records. */
        status = ptxWLCN_RetrieveNdefMessage (wlcn, record_buffer, &records_len);

        if (ptxStatus_Success == status)
        {
            uint32_t filtered_rec_pos[POLLER_NDEF_RECORD_BUFFER_SIZE];
            uint32_t filtered_rec_len = (records_len < POLLER_NDEF_RECORD_BUFFER_SIZE ? records_len : POLLER_NDEF_RECORD_BUFFER_SIZE);

            /** Find WLCCTL record within retrieved NDEF message. */
            status = ptxNDEFMessage_FilterRecordsByType(record_buffer, &filtered_rec_len, TYPE_RTD_WLCCTL, TYPE_RTD_WLCCTL_LEN, filtered_rec_pos);

            /** Verify that only one single record of type 'WLCCTL' was found. */
            if ((ptxStatus_Success == status) && (1u == filtered_rec_len))
            {
                /** 'ctl_rec_pos' contains position of CTL record within NDEF message. */
                uint32_t ctl_rec_pos = filtered_rec_pos[0];
                status = ptxWLCCTL_Parse (&record_buffer[ctl_rec_pos], &wlcn->RecordWlcCtl);

                if (ptxStatus_Success == status)
                {
                    ptxWLCN_AppCallback_event (wlcn, CtrlFieldReceived, &wlcn->RecordWlcCtl, sizeof(ptxWLCCTL_t));

                    /** Start searching for optional records (like 'WLCSTAI'). */
                    /** Optional records are only interpreted when occuring AFTER the CTL record (as per WLC standard). */
                    ptxNDEFRecord_t *optional_records = &record_buffer[ctl_rec_pos + 1];
                    uint32_t optional_rec_len = records_len - ctl_rec_pos - 1;

                    /** Try to parse WLCSTAI record */
                    ptxStatus_t optional_status;
                    optional_status = ptxNDEFMessage_FilterRecordsByType(optional_records, &optional_rec_len, TYPE_RTD_WLCSTAI, TYPE_RTD_WLCSTAI_LEN, filtered_rec_pos);
                    if ((ptxStatus_Success == optional_status) && (1u == optional_rec_len))
                    {
                        optional_status = ptxWLCSTAI_Parse(&optional_records[filtered_rec_pos[0]], &wlcn->RecordWlcStai);
                        if (ptxStatus_Success == optional_status)
                        {
                            wlcn->RecordWlcStaiSet = 1u;
                            /** Execute 'StaiFieldReceived' application callback */
                            ptxWLCN_AppCallback_event (wlcn, StaiFieldReceived, &wlcn->RecordWlcStai, sizeof(ptxWLCSTAI_t));
                        }
                    }

                    /** Start interpreting WLCCTL record. */
                    uint8_t error;
                    ptxWLCCTL_GetError(&wlcn->RecordWlcCtl, &error);

                    /** Did the WLC-L report an error? */
                    if (!error)
                    {
                        /** The WLC-L's CTL record contains a counter. If this counter changes,
                         *  we know that the CTL record was updated ba the WLC-l and contains 'fresh'
                         *  data. */
                        uint8_t ctl_ctr;
                        ptxWLCCTL_GetCounter(&wlcn->RecordWlcCtl, &ctl_ctr);
                        /** Compare current counter value with previous one. */
                        if (((0xFF != wlcn->CntrCtl) && (wlcn->CntrCtl != ctl_ctr)) || ((wlcn->CntrCtl == 0xFF) && (0x00 == ctl_ctr)))
                        {
                            /** Save current counter value for next cycle. */
                            wlcn->CntrCtl = ctl_ctr;

                            uint8_t rd_conf;
                            ptxWLCCAP_GetRdConf(&wlcn->RecordWlcCap, &rd_conf);
                            /** Check if WLC-L requires a read confirmation (implemented by writing empty NDEF record) */
                            if (rd_conf)
                            {
                                /** Waiting time will be between 1ms and 11ms */
                                uint8_t wr_wt_int;
                                ptxWLCCAP_GetWrWtInt(&wlcn->RecordWlcCap, &wr_wt_int);
                                wr_wt_int++;

                                ptxPLAT_Sleep(wlcn->IotRd.Plat, (uint32_t) wr_wt_int);

                                /** Write empty record to confirm that data has been read. */
                                ptxNDEFRecord_t empty_record;
                                ptxNDEFRecord_CreateEmptyRecord(&empty_record);
                                status = ptxWLCN_WriteNdefMessage(wlcn, &empty_record, 1u);
                            }

                            if (ptxStatus_Success == status)
                            {
                                uint8_t wpt_request;
                                ptxWLCCTL_GetWptReq(&wlcn->RecordWlcCtl, &wpt_request);
                                /** Check if WPT request is set. */
                                if (wpt_request)
                                {
                                    *chargingState = NegoCharging;
                                }
                                else
                                {
                                    /** Deactivate WLC-L & turn off RF-field for waiting time duration. */
                                    *chargingState = FieldOff;
                                }
                            }
                            else
                            {
                                /** Write empty record failed */
                                *chargingState = DeactivateListener;
                                ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                            }
                        }
                        else
                        {
                            /** In case the WLC-L didn't manage to update it's NDEF message in time the
                             * WLC-P has to retry after a specified waiting time. */
                            wlcn->CntrNwcc++; /**< increase retry counter */
                            if (wlcn->CntrNwcc <= wlcn->WlcnInitParams.ProtNwccMaxRetries)
                            {
                                uint8_t waiting_time_ms = wlcn->WlcnInitParams.ProtNwccRetryDelay;
                                ptxPLAT_Sleep(wlcn->IotRd.Plat, waiting_time_ms);

                                *chargingState = GetControlRecord;
                            }
                            else
                            {
                                /** Maximum number of retries reached */
                                *chargingState = DeactivateListener;
                                ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                            }
                        }
                    }
                    else
                    {
                        /** WLC-L indicated protocol error */
                        *chargingState = DeactivateListener;
                        ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                    }
                }
            }
            else
            {
                filtered_rec_len = (records_len < POLLER_NDEF_RECORD_BUFFER_SIZE ? records_len : POLLER_NDEF_RECORD_BUFFER_SIZE);
                status = ptxNDEFMessage_FilterRecordsByType(record_buffer, &filtered_rec_len, TYPE_RTD_WLCINF, TYPE_RTD_WLCINF_LEN, filtered_rec_pos);

                /** Verify that only one single record of type 'WLCCTL' was found. */
                if ((ptxStatus_Success == status) && (1u == filtered_rec_len))
                {
                    /** In case the WLC-L didn't manage to update it's NDEF message in time the
                    * WLC-P has to retry after a specified waiting time. */
                    wlcn->CntrNwcc++; /**< increase retry counter */
                    if (wlcn->CntrNwcc <= wlcn->WlcnInitParams.ProtNwccMaxRetries)
                    {
                        uint8_t waiting_time_ms = wlcn->WlcnInitParams.ProtNwccRetryDelay;
                        ptxPLAT_Sleep(wlcn->IotRd.Plat, waiting_time_ms);

                        *chargingState = GetControlRecord;
                    }
                }

                if( (ptxStatus_Success != status) || (1u != filtered_rec_len) ||
                        (wlcn->CntrNwcc > wlcn->WlcnInitParams.ProtNwccMaxRetries) )
                {
                    /** No/Multiple CTL field available */
                    *chargingState = DeactivateListener;
                    ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                }
            }
        }
        else
        {
            /** Error in the RF Communication. */
            *chargingState = DeactivateListener;
            ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_FieldOff (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        ptxWLCMode_t mode;
        uint32_t wait_duration_ms;

        ptxWLCCAP_GetMode(&wlcn->RecordWlcCap, &mode);

        /** When 'mode' field in CAP is set to 'BAT_FULL' wait for CAP_WT, otherwise wait for WPT_DUR. */
        if (ptxWLCMode_BattFull == mode)
        {
            ptxWLCCAP_GetCapWtMillis(&wlcn->RecordWlcCap, &wait_duration_ms);
        }
        else
        {
            ptxWLCCTL_GetWptDurationMillis(&wlcn->RecordWlcCtl, &wait_duration_ms);
        }

        /** Permanently turn off RF-field */
        status = ptxIoTRd_Reader_Deactivation(&wlcn->IotRd, 0u);

        if(ptxStatus_Success == status)
        {
            status = ptxPERIPH_APPTIMER_Start(wait_duration_ms);
            wlcn->PresenceCheckTimestamp = 0u;
            *chargingState = PollFieldOff;
        }
        else
        {
            *chargingState = SystemError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_NegoCharging (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        uint32_t wpt_duration_ms;
        uint32_t hold_off_duration_ms;

        /** Query wpt_duration and calculate time in milliseconds */
        ptxWLCCTL_GetWptDurationMillis(&wlcn->RecordWlcCtl, &wpt_duration_ms);

        /** Query hold_off_duration and calculate time in milliseconds */
        ptxWLCCTL_GetHoldOffWtMillis(&wlcn->RecordWlcCtl, &hold_off_duration_ms);

        /** Protocol error, if hold_off_duration_ms > 25% of wpt_duration_ms */
        if ( (0u == hold_off_duration_ms) || (((wpt_duration_ms * 100u) / hold_off_duration_ms) <= 400u) )
        {
            /** wait for hold_off_duration before changing power level */
            ptxPLAT_Sleep(wlcn->IotRd.Plat, hold_off_duration_ms);

            /** query wpt_duration integer (this is not equal to wpt_duration_ms --> see formula )*/
            uint8_t wpt_duration;
            ptxWLCCTL_GetWptDuration(&wlcn->RecordWlcCtl, &wpt_duration);

            /** Calculating requested power level */
            ptxWLCN_UpdatePwrLevel(wlcn);
            printf("Power level: %d\n", wlcn->PowerLevel);

            /** Clear WLC events before starting WPT cycle */
            wlcn->NscWlcEvent = PowerTransfer_NoEvent;

            /** Recalculate new wavebank settings. */
            status = ptxWLCN_WptWbScaling(wlcn);
            status |= ptxWLCN_NfcWbScaling(wlcn);

            if (ptxStatus_Success == status)
            {
                /** Options for WPT cycle. */
               ptxNSC_WLC_PowerTransferOptions_t options;
               options.WbCharging       = wlcn->WptWavebank;
               options.WbPostCharging   = wlcn->NfcWavebank;
               options.WptDuration      = wpt_duration;
               options.RffSyncDuration  = wlcn->WlcnInitParams.RffSyncTime;
               options.Bfod             = wlcn->WlcnInitParams.WptBfodEnable;
               options.Bfod_Gt          = wlcn->WlcnInitParams.WptBfodGuardTime;
               options.StopRequest      = wlcn->WlcnInitParams.WptStopReqEnable;

                /** Start power cycle */
                status = ptxWPT_Start(&wlcn->Wpt, &options);

                if (ptxStatus_Success == status)
                {
                    /** Wait for the process to be finished. */
                    uint32_t wpt_timeout_ms = wpt_duration_ms;
                    /** Add additional 10% margin to timeout. */
                    ptxWLCN_AddTimeoutSafetyMargin(&wpt_timeout_ms);

                    /** Power cycle started, let's report to the application. */
                    ptxWLCN_AppCallback_event(wlcn, WptCycleStarted, &wlcn->PowerLevel, 1u);

                    status = ptxPERIPH_APPTIMER_Start(wpt_timeout_ms);

                    /** Polling for static charging. */
                    *chargingState = PollCharging;
                }
            }

            if(ptxStatus_Success != status)
            {
               *chargingState = SystemError;
            }
        }
        else
        {
            /** hold_off_duration violation */
            *chargingState = DeactivateListener;
            ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_PollCharging (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != chargingState) )
    {
        ptxNSC_WLC_Events_t wlc_event = ptxWLCN_WPT_GetEvent(wlcn);

        /**< Check if charging cycle successfully terminated. */
        if ( (PowerTransfer_OK == wlc_event) || (PowerTransfer_WPTStop == wlc_event) )
        {
            /** Deinit the timer */
            ptxPERIPH_APPTIMER_Stop();

            ptxWLCMode_t mode;
            ptxWLCCAP_GetMode(&wlcn->RecordWlcCap, &mode);

            if(ptxWLCMode_Static == mode)
            {
                *chargingState = GetCapRecord;
            }
            else
            {
                uint32_t t_settle = wlcn->WlcnInitParams.WptPostSettleTime;

                ptxPLAT_Sleep(wlcn->IotRd.Plat, t_settle); /**< Tsettle */

                uint8_t skip_info;
                ptxWLCCTL_GetWptInfoReq(&wlcn->RecordWlcCtl, &skip_info);

                /** Check if the WLC-L wants us to omit the WLCINF field in the next negotiation cycle. */
                if(skip_info)
                {
                    *chargingState = SetInfoRecord;
                }
                else
                {
                    *chargingState = GetControlRecord;
                }
            }

            ptxWLCN_AppCallback_event(wlcn, WptCycleFinished, NULL, 0);
        }
        else if(PowerTransfer_FoD == wlc_event) /**< FOD detected. */
        {
            ptxWLCN_AppCallback_event(wlcn, WptCycleFinished, NULL, 0);

            ptxPERIPH_APPTIMER_Stop();

            *chargingState = DeactivateListener;

            ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
        }
        else
        {
            /**< No WPT event happened. */
            TimerStatus_t timer_status;
            status = ptxPERIPH_APPTIMER_Status(&timer_status);

            if( (ptxStatus_Success != status) || (0 != timer_status.IsElapsed) )
            {
                /** Timeout, no response from PTX130. */
                ptxPERIPH_APPTIMER_Stop();

                status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InternalError);

                *chargingState = SystemError;
            }
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_PollReady (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN)) && (NULL != chargingState) )
    {
        TimerStatus_t timer_status;
        status = ptxPERIPH_APPTIMER_Status(&timer_status);

        if(timer_status.IsElapsed)
        {
            wlcn->CntrNwt++;
            *chargingState = GetCapRecord;
        }

        if(ptxStatus_Success != status)
        {
            *chargingState = SystemError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_PollFieldOff (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN)) && (NULL != chargingState) )
    {
        TimerStatus_t tstat;

        status = ptxPERIPH_APPTIMER_Status(&tstat);

        if(ptxStatus_Success == status)
        {
            if(tstat.IsElapsed)
            {
                status = ptxPERIPH_APPTIMER_Stop();
                /** Restart polling for new devices. */
                *chargingState = WaitForListener;
            }
            else
            {
                if((tstat.ElapsedTime - wlcn->PresenceCheckTimestamp) >= wlcn->WlcnInitParams.PollingFieldOffInterval)
                {
                    /** Set last presence check timestamp. */
                    wlcn->PresenceCheckTimestamp = tstat.ElapsedTime;
                    uint8_t is_available;
                    status = ptxWLCN_PresenceCheck(wlcn, &is_available);

                    if(0u == is_available)
                    {
                        status = ptxPERIPH_APPTIMER_Stop();
                        /** Restart polling for new devices. */
                        *chargingState = WaitForListener;

                        ptxWLCN_AppCallback_event(wlcn, ListenerRemoved, NULL, 0);
                    }
                }
            }
        }

        if(ptxStatus_Success != status)
        {
            *chargingState = SystemError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_State_DeactivateListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN)) && (NULL != chargingState) )
    {
        status = ptxIoTRd_Reader_Deactivation(&wlcn->IotRd, 0u);

        if(ptxStatus_Success == status)
        {
            ptxPLAT_Sleep(wlcn->IotRd.Plat, wlcn->WlcnInitParams.PollingCycleIdleTime);

            *chargingState = WaitForListener;
        }
        else
        {
            *chargingState = SystemError;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxWLCN_PresenceCheck(ptxWLCN_t *wlcn, uint8_t *isAvailable)
{
    ptxStatus_t status = ptxStatus_Success;
    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != isAvailable) )
    {
        *isAvailable = 0u;

        ptxIoTRd_DiscConfig_t rf_disc_config;
        /** Set all params of the config to zero. */
        (void) memset(&rf_disc_config, 0, sizeof(ptxIoTRd_DiscConfig_t));

        /** Only poll for the RF-technology of the tag that was previously connected. */
        switch(wlcn->Card.TechType)
        {
            case Tech_TypeA:
                rf_disc_config.PollTypeA = 1u;
                break;
            case Tech_TypeB:
                rf_disc_config.PollTypeB = 1u;
                break;
            case Tech_TypeF:
                rf_disc_config.PollTypeF212 = 1u;
                rf_disc_config.PollTypeF424 = 1u;
                break;
            case Tech_TypeV:
                rf_disc_config.PollTypeV = 1u;
                break;
            default:
                break;
        }

        status = ptxIoTRd_Initiate_Discovery(&wlcn->IotRd, &rf_disc_config);

        if (ptxStatus_Success == status)
        {
            struct ptxPlatTimer *timer = NULL;
            status = ptxPLAT_GetInitializedTimer(wlcn->IotRd.Plat, &timer);

            if (ptxStatus_Success == status)
            {

                /** Let's start a Timer for stopping the discovery after a certain timeout. */
                status = ptxPLAT_TimerStart(wlcn->IotRd.Plat, timer, 50u, 0u, NULL, NULL);

                if (ptxStatus_Success == status)
                {

                    uint8_t discover_status = RF_DISCOVER_STATUS_NO_CARD;
                    uint8_t is_elapsed = 0u;
                    do
                    {
                        /** Get discovery status */
                        (void) ptxIoTRd_Get_Status_Info(&wlcn->IotRd, StatusType_Discover, &discover_status);

                        /** Check if the timeout is already reached. */
                        (void) ptxPLAT_TimerIsElapsed(wlcn->IotRd.Plat, timer, &is_elapsed);

                    } while ( (RF_DISCOVER_STATUS_NO_CARD == discover_status) && (0 == is_elapsed) );

                    (void) ptxPLAT_TimerDeinit(wlcn->IotRd.Plat, timer);

                    /** Search for the listener in the card registry. */
                    (void) ptxWLCN_ListenerAvailable(wlcn, wlcn->Card, isAvailable);

                    status = ptxIoTRd_Reader_Deactivation(&wlcn->IotRd, 0u);
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

static ptxStatus_t ptxWLCN_UpdatePwrLevel (ptxWLCN_t *wlcn)
{
    ptxStatus_t status = ptxStatus_Success;

    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN))
    {
        int8_t power_adj_req;
        /** Query the desired power adjust level. */
        (void) ptxWLCCTL_GetPowerAdjustReq(&wlcn->RecordWlcCtl, &power_adj_req);

        int8_t power_adj_percent;
        /** Multiply the value with conversion factor to get correction value in percent. */
        power_adj_percent = (int8_t) power_adj_req;// * WLCCTL_PWRADJ_PERCENT_PER_STEP);

        /** Checks if the required adjusted is smaller than the minimum allowed adjustment
         *  in either direction (increase/decrease). */
        if(power_adj_percent < 0)
        {
            int8_t min_dec = wlcn->WlcnInitParams.ProtMinStepDecrease;

            if(power_adj_percent > min_dec)
            {
                power_adj_percent = min_dec;
            }
        }
        else if(power_adj_req > 0)
        {
            int8_t min_inc = wlcn->WlcnInitParams.ProtMinStepIncrease;

            if(power_adj_percent < min_inc)
            {
                power_adj_percent = min_inc;
            }
        }

        /** Compute required power level for next WPT cycle. */
        int8_t required_pwr = (int8_t) (wlcn->PowerLevel + power_adj_percent);

        /** Check if power level is within bounds, defined by customer. */
        int8_t min_allowed_pwr = (int8_t) (wlcn->WlcnInitParams.PollerPowerLevelMin);
        if(required_pwr < min_allowed_pwr)
        {
            required_pwr = min_allowed_pwr;
        }

        /** Check if power level is within bounds, defined by customer. */
        int8_t max_allowed_pwr = (int8_t) (wlcn->WlcnInitParams.PollerPowerLevelMax);
        if(required_pwr > max_allowed_pwr)
        {
            required_pwr = max_allowed_pwr;
        }

        /** Set the power level, to be used for next WPT cycle. */
        wlcn->PowerLevel = (uint8_t) required_pwr;

    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static void ptxWLCN_AddTimeoutSafetyMargin (uint32_t *timeout)
{
    uint32_t safetyMargin = *timeout / 10u;

    if (safetyMargin < 1u)
    {
        safetyMargin = 1u;
    }

    *timeout = *timeout + safetyMargin;
}

static ptxStatus_t ptxWLCN_WbScaling (uint8_t *wbLuTable, uint8_t powerLevel, uint8_t *wb)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (NULL != wbLuTable) && (NULL != wb) && (powerLevel <= 100) )
    {
        for(uint8_t i = 0; i < PTX_WAVEBANK_LEN; ++i)
        {
            uint16_t prescaling = (uint16_t)((((uint16_t)wbLuTable[i] * (uint16_t)powerLevel) + 50u) / 100u);
            wb[i] = (uint8_t)((((uint16_t)prescaling * (uint16_t)PTX_WAVEBANK_SCALING) + 128u) / 255u);
        }
    }

    return status;
}

static ptxStatus_t ptxWLCN_WptWbScaling (ptxWLCN_t *wlcn)
{
    ptxStatus_t status = ptxStatus_Success;
    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN))
    {
        status = ptxWLCN_WbScaling(wlcn->WptWbLuTable, wlcn->PowerLevel, wlcn->WptWavebank);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxWLCN_NfcWbScaling (ptxWLCN_t *wlcn)
{
    ptxStatus_t status = ptxStatus_Success;
    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN))
    {
        uint8_t pwr_lvl;

        if(0 == wlcn->WlcnInitParams.WptPostPwrLevel)
        {
            pwr_lvl = wlcn->PowerLevel;
        }
        else
        {
            pwr_lvl = wlcn->WlcnInitParams.WptPostPwrLevel;
        }

        status = ptxWLCN_WbScaling(wlcn->NfcWbLuTable, pwr_lvl, wlcn->NfcWavebank);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxWLCN_InitInternals (ptxWLCN_t *wlcn)
{
    ptxStatus_t status = ptxStatus_Success;
    if (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN))
    {
        wlcn->CntrNwcc         = 0u;
        wlcn->CntrNwt          = 0u;
        wlcn->CntrCtl          = 255u;
        wlcn->RecordWlcStaiSet = 0u;
        wlcn->RecordDevInfoSet = 0u;
        wlcn->PowerLevel = wlcn->WlcnInitParams.PollerPowerLevelInitial;

        wlcn->NdefInitParams.IotRd = &(wlcn->IotRd);
        wlcn->NdefInitParams.TxBuffer = wlcn->NdefTxBuffer;
        wlcn->NdefInitParams.TxBufferSize = sizeof(wlcn->NdefTxBuffer);
        wlcn->NdefInitParams.RxBuffer = wlcn->NdefRxBuffer;
        wlcn->NdefInitParams.RxBufferSize = sizeof(wlcn->NdefRxBuffer);
        wlcn->NdefInitParams.WorkBuffer = wlcn->NdefWorkBuffer;
        wlcn->NdefInitParams.WorkBufferSize = sizeof(wlcn->NdefWorkBuffer);

        status = ptxNDEF_Open(&wlcn->NdefComp, &wlcn->NdefInitParams);
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxWLCN_ListenerAvailable (ptxWLCN_t *wlcn, ptxIoTRd_CardParams_t cardToFind, uint8_t *isAvailable)
{
    ptxStatus_t status = ptxStatus_Success;
    if ( PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN) && (NULL != isAvailable) )
    {
        *isAvailable = 0;
        /** Iterate through all available card params (this includes tag IDs). */
        for(uint8_t i = 0; i < wlcn->IotRd.CardRegistry->NrCards; ++i)
        {
            int32_t cmp = memcmp(&wlcn->IotRd.CardRegistry->Cards[i], &cardToFind, sizeof(ptxIoTRd_CardParams_t));
            if(0u == cmp)
            {
                *isAvailable = 1;
                break;
            }
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxWLCN_SelectProtocol (ptxIoTRd_CardParams_t cardParams,  ptxIoTRd_CardProtocol_t *protocol)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != protocol)
    {
        *protocol = Prot_Undefined;

        switch (cardParams.TechType)
        {
            case Tech_TypeA:
                if (0u != (cardParams.TechParams.CardAParams.SEL_RES & 0x20u))
                {
                    *protocol = Prot_ISODEP;
                }
                else
                {
                    *protocol = Prot_T2T;
                }
                break;

            case Tech_TypeB:
                if (0u != (cardParams.TechParams.CardBParams.SENSB_RES[10] & 0x01u))
                {
                    *protocol = Prot_ISODEP;
                }
                break;

            case Tech_TypeF:
                *protocol = Prot_T3T;
                break;

            case Tech_TypeV:
                *protocol = Prot_T5T;
                break;

            default:
                break;
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxWLCN_RetrieveNdefMessage (ptxWLCN_t *wlcn, ptxNDEFRecord_t *records, uint32_t *recordsLen)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN)) && (NULL != records) && (NULL != recordsLen) )
    {
        uint32_t len = sizeof(wlcn->NdefScratchpad);

        /** Read the message from the listener's memory. */
        status = ptxNDEF_ReadMessage(&wlcn->NdefComp, &wlcn->NdefScratchpad[0], &len);

        if (ptxStatus_Success == status)
        {
            /** Parse the read content. */
            (void) ptxNDEFMessage_Parse(wlcn->NdefScratchpad, len, records, recordsLen, false);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxWLCN_WriteNdefMessage (ptxWLCN_t *wlcn, ptxNDEFRecord_t *records, uint32_t recordsLen)
{
    ptxStatus_t status = ptxStatus_Success;

    if ( (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN)) && (NULL != records) )
    {
        uint32_t len = sizeof(wlcn->NdefScratchpad);

        /** Create one contiguous NDEF message from individual records and store it in the scratchpad. */
        status = ptxNDEFMessage_Create(records, recordsLen, wlcn->NdefScratchpad, &len);

        if (ptxStatus_Success == status)
        {
            /** Write the scratchpad to the listener's memory. */
            status = ptxNDEF_WriteMessage(&wlcn->NdefComp, wlcn->NdefScratchpad, len);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN, ptxStatus_InvalidParameter);
    }

    return status;
}

static void ptxWLCN_AppCallback_event (ptxWLCN_t *wlcn, ptxWLCN_Appevent_t appEvent, void *data, size_t dataLen)
{
    if ( (PTX_COMP_CHECK(wlcn, ptxStatus_Comp_WLCN)) && (NULL != wlcn->WlcnInitParams.AppCb) )
    {
        (void) wlcn->WlcnInitParams.AppCb(wlcn->WlcnInitParams.Ctx, appEvent, data, dataLen);
    }
}

static ptxNSC_WLC_Events_t ptxWLCN_WPT_GetEvent (ptxWLCN_t *wlcn)
{
    ptxNSC_WLC_Events_t nsc_wlc_event = PowerTransfer_NoEvent;
    ptxStatus_t status = ptxStatus_Success;

    /** Let's check first if there has been anything received. */
    status = ptxPLAT_TriggerRx(wlcn->IotRd.Plat);

    if (ptxStatus_Success == status)
    {
        nsc_wlc_event = wlcn->NscWlcEvent;
    }

    return nsc_wlc_event;
}
