/** \file
   ---------------------------------------------------------------
   Copyright (C) 2024. Renesas Electronics Corp. - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Renesas Electronics Corp..

   PLEASE CHECK FURTHER DISCLAIMER AT THE END OF FILE
   ---------------------------------------------------------------

    Project     : PTX130W-N/PTX30W-N
    Module      : ...
    File        : ...

    Description : ...
*/
#ifndef STACK_COMPS_WLC_POLLER_WLCN_PTXWLCN_POLLERDEFINES_H_
#define STACK_COMPS_WLC_POLLER_WLCN_PTXWLCN_POLLERDEFINES_H_


/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#ifdef __cplusplus
extern "C" {
#endif


/*
 * ####################################################################################################################
 * POLLER DEVICE DEFINES
 * ####################################################################################################################
 */
/**
 * \brief Major protocol version number.
 */
#define     POLLER_WLC_PROTOCOL_VERSION_MAJOR   (2u)

/**
 * \brief Minor protocol version number.
 */
#define     POLLER_WLC_PROTOCOL_VERSION_MINOR   (0u)

//#define     SUPPORT_DEVICE_INFORMATION_RECORD

/**
 * \brief Manufacturer name that is sent along the device information record.
 */
#define     POLLER_DEVINF_MANUFACTURER_NAME     ("Panthronics AG")

/**
 * \brief Model name that is sent along the device information record.
 */
#define     POLLER_DEVINF_MODEL_NAME            ("PTX130W NFC WLC Poller EB v1.1")

/**
 * \brief Firmware Version that is sent along the device information record.
 */
#define     POLLER_DEVINF_FIRMWARE_VERSION      ("PTX WLCN 2.0.0")

/**
 * \brief Power class that is sent along the WLC Poll Information Record.
 */
#define     POLLER_POWER_CLASS_USED             (PowerClass3)

/**
 * \brief Lowest charging power level (will also be used for static mode). Stay above 30.
 */
#define     POLLER_POWER_LEVEL_MIN              (40u)

/**
 * \brief Highest charging power level. Maximum of 100 allowed!
 */
#define     POLLER_POWER_LEVEL_MAX              (80u)

/**
 * \brief Initial power level used for negotiated charging.
 */
#define     POLLER_POWER_LEVEL_INITIAL          (80u)

/**
 * \brief Minimum allowed increase of the power level (in percent).
 */
#define     POLLER_POWER_STEPS_MIN_INC          (1)

/**
 * \brief Minimum allowed decrease of the power level (in percent).
 */
#define     POLLER_POWER_STEPS_MIN_DEC          (-1)

/**
 * \brief Power level, after a finished WPT charging phase in percent. If set to 0, WPT power level will be kept.
 */
#define     POLLER_POST_WPT_POWER_LEVEL         (0u)

/**
 * \brief Time between polling cycles.
 */
#define     POLLER_TIME_BETWEEN_POLLING_CYCLES  (500u)

/**
 * \brief Timeout of waiting for cards.
 */
#define     POLLER_WAIT_FOR_CARDS_TIMEOUT       (300u)

/**
 * \brief VPTX voltage applied by DCDC converter to supply VBAT1/2 and VDPA.
 *        Attention: defines the max. output power of the poller​.
 */
#define     POLLER_TARGET_DCDC_VOLTAGE_MV       (4000u)

/**
 * \brief Polling interval, to check if listener is still present during Field-Off-Period (e.g. when battery full).
 *        This parameter has to be a power of 2!
 */
#define     POLLER_FIELD_OFF_POLL_INTERVAL      (1 << 10)

/**
 * \brief Enable/Disable Background Foreign Object Detection (BFOD).
 */
#define     POLLER_WPT_BFOD_EN                  (0u)

/**
 * \brief Enable/Disable WPT stop request pattern.
 */
#define     POLLER_WPT_STOP_REQ_DET_EN          (0u)

/**
 * \brief Foreign object detection guard time duration.
 *         BFOD is enabled after this delay (in ms) upon WPT start.
 */
#define     POLLER_WPT_BFOD_GUARD_TIME          (10u)

/**
 *  \brief Milliseconds between LPCD pulses
 */
#define     POLLER_LPCD_IDLE_TIME               (500u)

/**
 * \brief Number of LPCD loops before next full poller poll phase.
 */
#define     POLLER_LPCD_LOOPS                   (20u)

/**
 * \brief PTX130W will enter standby between LPCD pulses (reduces power consumption).
 */
#define     POLLER_LPCD_STANDYBY_EN             (1u)

/**
* \brief PTX130W will send a notification to the host MCU once the LPCD is triggered.
*/
#define     POLLER_LPCD_NTF_EN                  (0u)

/**
 * \brief Number of maximum retries for reading the control field of the listener.
 */
#define     POLLER_NWCC_MAX_RETRIES             (3u)

/**
 * \brief Delay for retrying to read the control field.
 */
#define     POLLER_NWCC_RETRY_DELAY             (35u)

/**
 * \brief Additional delay time between writing the Info record and reading the control field.
 *  This delay is additionally added to the request delay by the listener!
 */
#define     POLLER_WT_MARGIN_DELAY              (30u)       /** Min 0, Max 30 ms*/

/**
 * \brief Settle duration (in ms) after power level has been decreased, subsequent to a successful WPT.
 *          Currently, no values below 1ms are supported!
 */
#define     POLLER_POWER_SETTLE_DURATION        (1u)       /** Min 300us, currently 1ms */

/**
 * \brief Duration of the RF-Field reset used for synchronisation in 50us units.
 */
#define     POLLER_RFF_SYNC_DURATION            (1u)       /** Min = 0 --> No RFF reset; Max = 12.75ms */


/**
 * \brief Initial startup delay before first communication in [ms] --> during this delay, Poller applies CW.
 */
#define     POLLER_STARTUP_DELAY                (0u)

#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */

/**
  Copyright (C) 2024. Renesas Electronics Corp. - All Rights Reserved.

                     -DISCLAIMER-

  Your use of this Software is limited to those specific rights granted under the terms of
  a software license agreement between the user who downloaded the software, his/her employer
  (which must be your employer) and Renesas Electronics Corp. (the "License"). You may not use this Software
  unless you agree to abide by the terms of the License. The License limits your use, and you
  acknowledge, that the Software may not be edited, altered, modified, adapted or otherwise
  changed and may solely and exclusively be used in conjunction with a Renesas Electronics Corp. component,
  which is integrated into your product. Other than for the foregoing purpose, you may not use,
  reproduce, copy, prepare derivative works of, modify, distribute, perform, display or sell this
  Software and/or its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS"
  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
  WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
  IN NO EVENT SHALL Renesas Electronics Corp. OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE
  THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL,
  SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
  PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING
  BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Austrian law shall apply without reference to its international conflict of laws provisions.
  Should you have any questions regarding your right to use this Software, contact Renesas Electronics Corp.
  (https://www.renesas.com)
*/
