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
    File        : ptxWLCN.h

    Description: This API provides all the necessary APIs required to
                 implement a Wireless Charging (WLC) Poller.
*/
/**
 * \addtogroup grp_ptx_api_wlcn PTX WLC API
 *
 * @{
 */
#ifndef COMPS_WLC_POLLER_WLCN_WLCN_H_
#define COMPS_WLC_POLLER_WLCN_WLCN_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxStatus.h"
#include "ptx_IOT_READER.h"
#include "ptxWPT.h"
#include "ptxNSC.h"
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "ptxNDEF.h"
#include "ptxNDEFMessage.h"
#include "ptxNDEFRecord.h"
#include "ptxNDEFRecord_DI.h"
#include "ptxNDEFRecord_WLCCAP.h"
#include "ptxNDEFRecord_WLCCTL.h"
#include "ptxNDEFRecord_WLCINF.h"
#include "ptxNDEFRecord_WLCSTAI.h"
#include "ptxPERIPH_APPTIMER.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

#define PTX_MANUFACTURER_CODE           (0x80)  /**< PTX Manufacturer code (= first byte of UID) */
#define PTX_WAVEBANK_LEN                (32u)   /**< Length of a wavebank */
#define PTX_WAVEBANK_SCALING            (0x1F)  /**< Scaling factor for a wavebank lookup table */
#define PTX_WAVEBANK_DEFAULT_LU_TABLE   {0, 25, 50, 74, 98, 120, 142, 162, 180, 197, 212, 225, 236, 244, 250, 254, 255, 254, 250, 244, 236, 225, 212, 197, 180, 162, 142, 120, 98, 74, 50, 25} /**< Default wavebank (sine) */
#define PTX_CARD_REGISTRY_UNDEFINED     (255u)

/**
 * \brief WLCN charging states
 */
typedef enum ptxWLCN_ChargeState
{
    WaitForListener,    /**< Wait for listener device. */
    ActivateListener,   /**< This state activates one of multiple listeners found during discovery. */
    GetCapRecord,       /**< Get capability record state. */
    WaitListenerReady,  /**< Wait until the listener is ready to accept a charging cycle. */
    StaticCharging,     /**< Static charging state. */
    SetInfoRecord,      /**< Set info record state. */
    GetControlRecord,   /**< Get control record state. */
    NegoCharging,       /**< Negotiated charging state. */
    FieldOff,           /**< Field off e.g. due to battery fully charged */
    DeactivateListener, /**< Deactivate listener state. */
    PollListener,       /**< Non-blocking wait for LPCD & Discovery. */
    PollListenerReady,  /**< Non-blocking wait for the listener to accept a charging cycle. */
    PollCharging,       /**< Non-blocking wait for the WPT to be finished. */
    PollFieldOff,       /**< Non-blocking wait during RF-field off-duration. */
    SystemError         /**< System error state --> causes abort of state-machine loop. */
} ptxWLCN_ChargeStates_t;

/**
 * \brief WLCN application events
 */
typedef enum ptxWLCN_Appevent
{
    ListenerDetected,           /**< Application event. Listener entered RF-field. Payload: Number of detected listeners (1 byte length) */
    WptCycleStarted,            /**< Application event. WPT cycle started. Payload: Power Level (1 byte length) */
    WptCycleFinished,           /**< Application event. WPT cycle stopped. Payload: None */
    ListenerRemoved,            /**< Application event. Listener removed. Payload: None */
    CapFieldReceived,           /**< Application event. Capability record received from listener. Payload: \ref ptxWLCCAP_t */
    StaiFieldReceived,          /**< Application event. Status & Information record received from listener. Payload: \ref ptxWLCSTAI_t */
    DeviceInfoFieldReceived,    /**< Application event. Device Information record received from listener. Payload: \ref ptxNDEFRecord_DI_t */
    CtrlFieldReceived           /**< Application event. CTRL field received from listener. Payload: \ref ptxWLCCTL_t */
} ptxWLCN_Appevent_t;

/**
 * \brief Callback function for reporting WLC events to main application.
 *
 */
typedef void (*pptxWLCN_AppCb_t) (void *ctx, ptxWLCN_Appevent_t wlcnApp, void *data, size_t dataLen);

/**
 * \brief WLCN Init Params.
 */
typedef struct ptxWLCN_InitParams
{
    ptxIoTRd_TempSense_Params_t     TempSens;                   /**< Temperature Sensor. */
    pptxWLCN_AppCb_t                AppCb;                      /**< Application callback used for notification of asynchronous events.*/
    void                            *Ctx;                       /**< Context provided by Application to be used on \ref AppCb.*/
    uint16_t                        PollerTargetVoltage;        /**< Target voltage of the DC/DC converter supplying the PTX130W. */
    uint8_t                         PollerPowerLevelMin;        /**< Minimum allowed power level in percent (e.g. 40%). */
    uint8_t                         PollerPowerLevelMax;        /**< Maximum allowed power level in percent (e.g. 90%). */
    uint8_t                         PollerPowerLevelInitial;    /**< Power level used for the very first WPT cycle. */
    int8_t                          ProtMinStepIncrease;        /**< Minimum allowed increase of the power level (in percent). */
    int8_t                          ProtMinStepDecrease;        /**< Minimum allowed decrease of the power level (in percent). */
    uint8_t                         ProtVersionMajor;           /**< Major WLC protocol version. */
    uint8_t                         ProtVersionMinor;           /**< Minor WLC protocol version. */
    ptxWLCINF_PowerClass_t          ProtPowerClass;             /**< Supported power class of the poller. */
    uint8_t                         ProtNwccRetryDelay;         /**< Timeout in case the listener is not ready for its control record to be read. */
    uint8_t                         ProtNwccMaxRetries;         /**< Maximum number of retries if the listener is not ready for its control record to be read. */
    uint8_t                         ProtWtMarginDelay;          /**< Additional delay time of the poller added to  the read wait time. */
    char                            *ProtDiManufacturerName;    /**< Manufacturer name string shared with listener in device information record. */
    char                            *ProtDiModelName;           /**< Firmware version string shared with listener in device information record. */
    char                            *ProtDiFirmwareVersion;     /**< Model name string shared with listener in device information record. */
    uint32_t                        PollingCycleIdleTime;       /**< Time after field-reset before retry. */
    uint32_t                        PollingFieldOffInterval;    /**< Timeout after which the poller run the presence check during 'field-off' state.*/
    uint32_t                        PollingLpcdIdleTime;        /**< Delay time between LPCD pulses. */
    uint8_t                         PollingLpcdLoops;           /**< Number of LPCD pulses, followed by standard polling sequence. */
    uint8_t                         PollingLpcdStandbyEnable;   /**< Go to standby between LPCD pulses. */
    uint8_t                         PollingLpcdNtfEnable;       /**< Enable LPCD NTF. */
    uint8_t                         WptBfodEnable;              /**< Enable/Disable support for BFOD during WPT. */
    uint8_t                         WptBfodGuardTime;           /**< Delay time after which BFOD gets enabled. */
    uint8_t                         WptStopReqEnable;           /**< Enable/Disable support for WPT stop request pattern. */
    uint8_t                         WptPostPwrLevel;            /**< Poller power level used after the WPT. */
    uint8_t                         WptPostSettleTime;          /**< Additional delay time after a successful WPT in ms. (Min = 300us) */
    uint8_t                         RffSyncTime;                /**< Duration of the RF-Field reset used for synchronisation. */
    uint8_t                         StartUpDelay;               /**< Initial startup delay before first communication in [ms]. */
    ptxIoTRd_Protection_Params_t    ExternalProtection;         /**< External protection settings for current limiter/sensor (optional). */
} ptxWLCN_InitParams_t;

/**
 * \brief Main WLCN (WLC NFC Forum) component structure.
 */
typedef struct ptxWLCN
{
    ptxStatus_Comps_t               CompId;                         /**< Component Id. */
    ptxIoTRd_t                      IotRd;                          /**< Iot Reader instance. */
    ptxWLCN_InitParams_t            WlcnInitParams;                 /**< Initialization parameters for WLCN instance. */
    ptxWPT_t                        Wpt;                            /**< WPT (Wireless Power Transfer) instance. */
    ptxNSC_WLC_Events_t             NscWlcEvent;                    /**< WLC event of PTX130W. */
    ptxIoTRd_CardParams_t           Card;                           /**< Card parameters of the connected listener device. */
    uint8_t                         CardRegistryId;                 /**< ID of the currently activated listener, within the card registry. */
    uint8_t                         NdefTxBuffer[0x200];            /**< 512B scratchpad buffer for reading NDEF messages. */
    uint8_t                         NdefRxBuffer[0x200];            /**< 512B scratchpad buffer for writing NDEF messages. */
    uint8_t                         NdefWorkBuffer[0x200];          /**< 512B scratchpad buffer for intermediate NDEF messages. */
    uint8_t                         NdefScratchpad[0x200];          /**< 512B Ndef Scratchpad buffer for building NDEF messages */
    ptxNDEF_t                       NdefComp;                       /**< Context of the generic NDEF operations API. */
    ptxNDEF_InitParams_t            NdefInitParams;                 /**< Initialisation parameters for the NDEF operations API. */
    ptxWLCCAP_t                     RecordWlcCap;                   /**< Capability record container provided by the WLC-L. */
    ptxWLCINF_t                     RecordWlcInf;                   /**< Poll information record container. */
    ptxWLCCTL_t                     RecordWlcCtl;                   /**< Control record container provided by the WLC-L. */
    ptxWLCSTAI_t                    RecordWlcStai;                  /**< Holds the information contained within the status and info record. */
    ptxNDEFRecord_DI_t              RecordDevInfo;                  /**< Holds the information provided in the device information record. */
    uint8_t                         RecordWlcStaiSet;               /**< Stores, if the status and info record was specified by the WLC-L. */
    uint8_t                         RecordDevInfoSet;               /**< Stores, if the device information record was specified by the WLC-L. */
    uint8_t                         CntrNwt;                        /**< Stores the number of repetitions of CAP_WT until the WLC-L will  be ready. */
    uint8_t                         CntrCtl;                        /**< Stores the previous counter contained within the control record. */
    uint8_t                         CntrNwcc;                       /**< Stores the number of NDEF read procedure retries. */
    uint8_t                         PowerLevel;                     /**< Holds the information of the current power level (in percent) applied to the operating volume. */
    uint32_t                        PresenceCheckTimestamp;         /**< Stores a timestamp of the last time the presence check was executed. */
    uint8_t                         WptWbLuTable[PTX_WAVEBANK_LEN]; /**< Stores the lookup table used for WPT wavebank generation. */
    uint8_t                         NfcWbLuTable[PTX_WAVEBANK_LEN]; /**< Stores the lookup table used for NFC wavebank generation. */
    uint8_t                         WptWavebank[PTX_WAVEBANK_LEN];  /**< Stores current WPT wavebank (depends on power level and LUT). */
    uint8_t                         NfcWavebank[PTX_WAVEBANK_LEN];  /**< Stores current NFC wavebank. */
} ptxWLCN_t;

/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initializes the WLC (Wireless Charging) component.
 *
 * This function has to be called before any other API functions at this component.
 *
 * \param[in]   wlcn              Pointer to WLC Component to be initialized.
 * \param[in]   initParam         Pointer containing init parameters.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_Init( ptxWLCN_t *wlcn, ptxWLCN_InitParams_t *initParam);

/**
 * \brief Update Chip configuration.
 *
 * \note  This function updates wavebanks for WLC power levels. This function shall be called after \ref ptxWLCN_Init, and
 *        before \ref ptxWLCN_NFCInit
 *
 *        This function is optional, just used used in case of application wants to overwrite some WLC Wavebanks
 *
 * \param[in]   wlcn                Pointer to WLC Component (already initialized).
 * \param[in]   wlcConfigParams     WLC Config Parameters.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_UpdateChipConfig( ptxWLCN_t *wlcn, ptxNSC_WLC_Config_Parameters wlcConfigParams);

/**
 * \brief Shuts down the NFC-Link at any given time.
 *
 * \param[in]   wlcn    Pointer to WLC Component.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_NFCDeInit (ptxWLCN_t *wlcn);

/**
 * \brief De-Initializes the WLC Component.
 *
 * This function has to be called last one of this component.
 *
 * \param[in]   wlcn           Pointer to WLC Component.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_Deinit( ptxWLCN_t *wlcn);

/**
 * \brief The user can pass an array with 32 entries (in range 0 - 255) to this API call, to define a waveshape
 *        used for the WPT cycles.
 *
 * \param[in]   wlcn    Pointer to WLCN component.
 * \param[in]   table   Pointer to the lookup table [32 entries; in range 0 - 255].
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_SetWptWbLuTable(ptxWLCN_t *wlcn, const uint8_t *table);

/**
 * \brief The user can pass an array with 32 entries (in range 0 - 255) to this API call, to define a waveshape
 *        used for the NFC communication between the individual WPT cycles.
 *
 * \param[in]   wlcn    Pointer to WLCN component.
 * \param[in]   table   Pointer to the lookup table [32 entries; in range 0 - 255].
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_SetNfcWbLuTable (ptxWLCN_t *wlcn, const uint8_t *table);

/**
 * \brief Retrieves the WPT lookup table (LUT) currently in use.
 *
 *
 * \param[in]       wlcn    Pointer to WLCN component.
 * \param[in,out]   table   Pointer to store the lookup table [32 entries; in range 0 - 255].
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_GetWptWbLuTable(ptxWLCN_t *wlcn, uint8_t *table);

/**
 * \brief Retrieves the NFC lookup table (LUT) currently in use.
 *
 *
 * \param[in]       wlcn    Pointer to WLCN component.
 * \param[in,out]   table   Pointer to store the lookup table [32 entries; in range 0 - 255].
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_GetNfcWbLuTable(ptxWLCN_t *wlcn, uint8_t *table);

/**
 * \brief Retrieves the automatically scaled WPT wavebank, currently in use.
 *
 *
 * \param[in]       wlcn    Pointer to WLCN component.
 * \param[in,out]   table   Pointer to store the wavebank [32 entries; in range 0 - 31].
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_GetWptWb(ptxWLCN_t *wlcn, uint8_t *wb);

/**
 * Retrieves the wavebank used for the communication phase (between the WPT cycles).  *
 *
 * \param[in]       wlcn    Pointer to WLCN component.
 * \param[in,out]   table   Pointer to store the wavebank [32 entries; in range 0 - 31].
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_GetNfcWb (ptxWLCN_t *wlcn, uint8_t *wb);

/**
 * \brief Checks if the activated tag/listener is a PTX device.
 *
 *
 * \param[in]       wlcn    Pointer to WLCN component.
 * \param[in,out]   isPtx   Pointer to store the information whether listener is a PTX device.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_IsPtxListener (ptxWLCN_t *wlcn, uint8_t *isPtx);

/*
 * ####################################################################################################################
 * FUNCTIONS FOR POLLER STATE MACHINE
 * ####################################################################################################################
 */

/**
 * \brief Wait for LPCD to be trigger.
 *
 * \note This function shall be called when \p chargingState is \ref WaitForLpcd.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_WaitForListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Poll if LPCD has been triggered.
 *
 * \note This function shall be called when \p chargingState is \ref PollLpcd.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_PollListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Activates one of multiple discovered tags.
 *
 * \note This function shall be called when \p chargingState is \ref ActivateListener.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_ActivateListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Get capability record state.
 *
 * \note This function shall be called when \p chargingState is \ref GetCapRecord.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_GetCapRecord (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Poll if the listener is ready for its capability record to be read.
 *
 * \note This function shall be called when \p chargingState is \ref WaitListenerReady.
 *
 * \param[in]        wlcn            Pointer to WLC Component.
 * \param[in,out]    chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_WaitForListenerReady (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Static charging state.
 *
 * \note This function shall be called when \p chargingState is \ref StaticCharging.
 *
 * \param[in]        wlcn            Pointer to WLC Component.
 * \param[in,out]    chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_StaticCharging (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Set info record state.
 *
 * \note This function shall be called when \p chargingState is \ref SetInfoRecord.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_SetInfoRecord (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Get control record state.
 *
 * \note This function shall be called when \p chargingState is \ref GetControlRecord.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_GetControlRecord (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief In this state, the poller deactivates its RF-field for a certain amount of time (specified by the listener)
 *        This for example happens when listener reports 'battery full' in the capability record.
 *
 * \note This function shall be called when \p chargingState is \ref FieldOff
 *
 * \param[in]        wlcn            Pointer to WLC Component.
 * \param[in,out]    chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_FieldOff (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Negotiated charging state.
 *
 * \note This function shall be called when \p chargingState is \ref NegoCharging
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_NegoCharging (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Poll for the end of the negotiated charging state.
 *
 * \note This function shall be called when \p chargingState is \ref PollCharging.
 *
 * \param[in]        wlcn            Pointer to WLC Component.
 * \param[in,out]    chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_PollCharging (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Poll if the tag is ready for its capability record to be read.
 *
 * \note This function shall be called when \p chargingState is \ref PollListenerReady.
 *
 * \param[in]        wlcn            Pointer to WLC Component.
 * \param[in,out]    chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_PollReady (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Poll for the end of the 'field-off' duration.
 *
 * \note This function shall be called when \p chargingState is \ref PollFieldOff.
 *
 * \param[in]        wlcn            Pointer to WLC Component.
 * \param[in,out]    chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_PollFieldOff (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Deactivate state.
 *
 * \note This function shall be called when \p chargingState is \ref DeactivateListener.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   chargingState   Pointer to charging state.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_State_DeactivateListener (ptxWLCN_t *wlcn, ptxWLCN_ChargeStates_t *chargingState);

/**
 * \brief Validates if the listener is still in the vicinity of the poller. It turns the RF-Field on,
 *        runs the RF-discovery and turns the field off again.
 *
 * \note This function shall only be called when \p chargingState is \ref PollFieldOff.
 *
 * \param[in]       wlcn            Pointer to WLC Component.
 * \param[in,out]   isAvailable     Pointer to uint8_t, indicating if listener is still within proximity of the poller.
 *
 * \return Status, indicating whether the operation was successful.See \ref ptxStatus_t.
 */
ptxStatus_t ptxWLCN_PresenceCheck (ptxWLCN_t *wlcn, uint8_t *isAvailable);

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */
