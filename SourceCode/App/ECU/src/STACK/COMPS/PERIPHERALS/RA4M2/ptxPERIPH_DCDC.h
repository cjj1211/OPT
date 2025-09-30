/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130W
    Module      : PERIPHERALS
    File        : ptxPERIPH_DCDC.h

    Description :
*/

/**
 * \addtogroup grp_ptx_api_peripherals_dcdc PTX Peripherals DCDC API
 *
 * @{
 */

#ifndef STACK_COMPS_PERIPHERALS_PTXPERIPH_DCDC_H_
#define STACK_COMPS_PERIPHERALS_PTXPERIPH_DCDC_H_

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "hal_data.h"
#include "ptxStatus.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */
#define PTX_PERIPH_DCDC_VOLTAGE_MAX_VAL             (5400u) /**< Maximum output value of DCDC in mV. */
#define PTX_PERIPH_DCDC_VOLTAGE_MIN_VAL             (3000u) /**< Minimum output value of DCDC in mV. */

#define PTX_PERIPH_DCDC_VOLTAGE_DAC_RESOLUTION      (12u)   /**< DAC resolution in bits. */
#define PTX_PERIPH_DCDC_VOLTAGE_UV_PER_STEP         (805)   /**< Voltage increase/decrease per DAC step on RA4M2 MCU */

#define PTX_PERIPH_DCDC_VOLTAGE_DAC_STEPS           (1 << PTX_PERIPH_DCDC_VOLTAGE_DAC_RESOLUTION)   /**< DAC steps within FSR */
#define PTX_PERIPH_DCDC_VOLTAGE_DAC_MAX_VAL         (PTX_PERIPH_DCDC_VOLTAGE_DAC_STEPS - 1)         /**< Maximum value of DAC setting. */
#define PTX_PERIPH_DCDC_VOLTAGE_DAC_MIN_VAL         (0)    /**< Minimum value of DAC setting. */
#define PTX_PERIPH_DCDC_VOLTAGE_DAC_VREF            (3300u) /**< Minimum value of DAC setting. */
#define PTX_PERIPH_DCDC_VOLTAGE_THRESHOLD           (100u)  /**< Threshold mV. Valid deviation of DCDC Voltage*/
#define VOLTAGE_REG_STEP                            (1u)    /**< DAC step used to regulate the voltage */

#ifdef POLLER_WLC_PCB_VERSION_V03
    #define PTX_PERIPH_DCDC_VOLTAGE_VFB             (500u)  /**< Feedback Voltage of DC/DC Converter */
    #define PTX_PERIPH_DCDC_RESISTOR_R5             (430u)  /**< Resistor R5 of DC/DC converter voltage divider (kOhm) */
    #define PTX_PERIPH_DCDC_RESISTOR_R7             (47u)   /**< Resistor R7 of DC/DC converter voltage divider (kOhm) */
    #define PTX_PERIPH_DCDC_RESISTOR_R8             (560u)  /**< Resistor R8 of DC/DC converter voltage divider (kOhm) */
#else
    #define PTX_PERIPH_DCDC_VOLTAGE_VFB             (800u)  /**< Feedback Voltage of DC/DC Converter (800 mV) */
    #define PTX_PERIPH_DCDC_RESISTOR_R5             (510u)  /**< Resistor R5 of DC/DC converter voltage divider (kOhm) */
    #define PTX_PERIPH_DCDC_RESISTOR_R7             (100u)  /**< Resistor R7 of DC/DC converter voltage divider (kOhm) */
    #define PTX_PERIPH_DCDC_RESISTOR_R8             (680u)  /**< Resistor R8 of DC/DC converter voltage divider (kOhm) */
#endif

#define PTX_PERIPH_DCDC_RESISTOR_R8_DIVBY_R5    ((1000 * PTX_PERIPH_DCDC_RESISTOR_R8)/PTX_PERIPH_DCDC_RESISTOR_R5)  /**< Constant needed for voltage calculation */
#define PTX_PERIPH_DCDC_RESISTOR_R8_DIVBY_R7    ((1000 * PTX_PERIPH_DCDC_RESISTOR_R8)/PTX_PERIPH_DCDC_RESISTOR_R7)  /**< Constant needed for voltage calculation */
/** Calculation for V_DAC from desired DCDC output voltage on RA4M2 MCU based demo PCB */
#define PTX_PERIPH_DCDC_VOLTAGE_DAC(V_OUT)      (PTX_PERIPH_DCDC_VOLTAGE_VFB * (1000 + (PTX_PERIPH_DCDC_RESISTOR_R8_DIVBY_R5 + PTX_PERIPH_DCDC_RESISTOR_R8_DIVBY_R7)) - (V_OUT * PTX_PERIPH_DCDC_RESISTOR_R8_DIVBY_R5) )

/**
 * Poller-HW-specific. DCDC Input parameters. DCDC main structure.
 */
typedef struct ptxPERIPH_DCDC
{
        ioport_instance_t const *ioport_instance;   /**< ioport instance */
        dac_instance_t const    *dac_instance;      /**< dac instance. */
        bsp_io_port_pin_t       DCDC_pin;           /**< DCDC pin component. (RA4M2) */
} ptxPERIPH_DCDC_t;
/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

/**
 * \brief Initialized DCDC module.
 *
 * \note This function shall be successfully executed before any other call to the functions in this module.
 *
 * \param[in]       dcdc      Pointer to an uninitialized DCDC context.
 * \param[in]       dcdcPars  Input parameters for DCDC.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_DCDC_Init(ptxPERIPH_DCDC_t *dcdc);


/**
 * \brief De initialize the DCDC module.
 *
 * \param[in]       dcdc      Pointer to an initialized DCDC context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_DCDC_Deinit(ptxPERIPH_DCDC_t *dcdc);


/**
 * \brief Enable DCDC at Poller Board.
 *
 * \param[in]       dcdc      Pointer to an initialized DCDC context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_DCDC_EnableDCDC(ptxPERIPH_DCDC_t *dcdc);


/**
 * \brief Disable DCDC at Poller Board.
 *
 * \param[in]       dcdc      Pointer to an initialized DCDC context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_DCDC_DisableDCDC(ptxPERIPH_DCDC_t *dcdc);


/**
 * \brief Init DAC pin at Poller Board.
 *
 * \param[in]       dcdc      Pointer to an initialized DCDC context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_DCDC_InitDAC(ptxPERIPH_DCDC_t *dcdc);


/**
 * \brief Deinit DAC pin at Poller Board.
 *
 * \param[in]       dcdc      Pointer to an initialized DCDC context.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_DCDC_DeinitDAC(ptxPERIPH_DCDC_t *dcdc);


/**
 * \brief Set DAC pin at Poller Board.
 *
 * \param[in]       dcdc             Pointer to an initialized DCDC context.
 * \param[in]       value            Value to be written at DAC.
 *
 * \return Status, indicating whether the operation was successful. See \ref ptxStatus_t.
 */
ptxStatus_t ptxPERIPH_DCDC_SetDAC(ptxPERIPH_DCDC_t *dcdc, uint16_t value);


#ifdef __cplusplus
}
#endif

#endif /* Guard */
/** @} */

