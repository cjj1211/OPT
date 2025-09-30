/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX130
    Module      : PERIPHERALS
    File        : ptxPERIPH_I2C.c

    Description : Basic I2C implementation on RA4M2 platform.

 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "hal_data.h"
#include "ptxPERIPH_I2C.h"

static const i2c_master_instance_t *i2c = &g_i2c0;

static volatile ptxPERIPH_I2CTRxStatus_t    sCallbackEvent;
#define I2C_TRANSACTION_BUSY_DELAY (100u)

ptxStatus_t ptxPERIPH_I2C_Init(void)
{
    ptxStatus_t status = ptxStatus_Success;

    const fsp_err_t result = R_SCI_I2C_Open(i2c->p_ctrl, i2c->p_cfg);

    if ( (FSP_SUCCESS != result) && (FSP_ERR_ALREADY_OPEN != result) )
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InternalError);
    }

    return status;
}

ptxStatus_t ptxPERIPH_I2C_Deinit(void)
{
    ptxStatus_t status = ptxStatus_Success;

    if (FSP_SUCCESS != R_SCI_I2C_Close(i2c->p_ctrl))
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InternalError);
    }

    return status;
}

ptxStatus_t ptxPERIPH_I2C_Tx(const uint8_t *txBuf, const size_t txLen, uint8_t restart)
{
    ptxStatus_t status = ptxStatus_Success;
    /** Let's see if there is something to write. */
    if ((NULL != txBuf) && (0 < txLen))
    {
        uint32_t timeout_ms = I2C_TRANSACTION_BUSY_DELAY;

        sCallbackEvent = TRX_TX_INPROGRESS;
        /** Tx part of the overall transaction. */
        if (FSP_SUCCESS == R_SCI_I2C_Write(i2c->p_ctrl, (uint8_t *)txBuf, txLen, restart))
        {
            while ((TRX_TX_INPROGRESS == sCallbackEvent) && (timeout_ms))
            {
                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
                --timeout_ms;
            }

            if (TRX_TX_DONE != sCallbackEvent)
            {
                status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InterfaceError);
            }
        }

        if (ptxStatus_Success != status)
        {
            (void) ptxPERIPH_I2C_Reset();
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxPERIPH_I2C_Rx(uint8_t *txBuf, const size_t rxLen, uint8_t restart)
{
    ptxStatus_t status = ptxStatus_Success;
    /** Let's see if there is something to write. */
    if ((NULL != txBuf) && (0 < rxLen))
    {
        uint32_t timeout_ms = I2C_TRANSACTION_BUSY_DELAY;

        sCallbackEvent = TRX_RX_INPROGRESS;
        /** Tx part of the overall transaction. */
        if (FSP_SUCCESS == R_SCI_I2C_Read(i2c->p_ctrl, (uint8_t *)txBuf, rxLen, restart))
        {
            while ((TRX_RX_INPROGRESS == sCallbackEvent) && (timeout_ms))
            {
                R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_MILLISECONDS);
                --timeout_ms;
            }

            if (TRX_RX_DONE != sCallbackEvent)
            {
                status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InterfaceError);
            }
        }

        if (ptxStatus_Success != status)
        {
            (void) ptxPERIPH_I2C_Reset();
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InvalidParameter);
    }

    return status;
}


ptxStatus_t ptxPERIPH_I2C_TRx(const uint8_t *txBuf, const size_t txLen, uint8_t restart, uint8_t *rxBuf, size_t rxLen)
{
    ptxStatus_t status = ptxStatus_Success;
    /** Let's see if there is something to write. */

    if ((NULL != txBuf) && (0 < txLen))
    {
        /**
         * Tx part of the overall transaction.
         */
        status = ptxPERIPH_I2C_Tx(txBuf, txLen, false);
    }

    if (ptxStatus_Success == status)
    {
        /**
         * Rx part of the overall transaction.
         */
        if ((NULL != rxBuf) && (0 < rxLen))
        {
            status = ptxPERIPH_I2C_Rx(rxBuf, rxLen, restart);
        }
    }

    return status;
}

ptxStatus_t ptxPERIPH_I2C_SetSlaveAddress(uint16_t slaveAddress)
{
    ptxStatus_t status = ptxStatus_Success;

    if (FSP_SUCCESS != R_SCI_I2C_SlaveAddressSet(i2c->p_ctrl, slaveAddress, I2C_MASTER_ADDR_MODE_7BIT))
    {
        status = PTX_STATUS(ptxStatus_Comp_WLCN_PERIPH, ptxStatus_InterfaceError);
    }

    return status;
}

ptxStatus_t ptxPERIPH_I2C_Reset(void)
{
    ptxStatus_t status = ptxStatus_Success;

    status = ptxPERIPH_I2C_Deinit();

    if (ptxStatus_Success == status)
    {
        status = ptxPERIPH_I2C_Init();
    }

    return status;
}

void ptxPERIPH_I2C_TransferCallback(i2c_master_callback_args_t *p_args)
{
    if (NULL != p_args)
    {
        switch (p_args->event)
        {
            case I2C_MASTER_EVENT_TX_COMPLETE:
                sCallbackEvent = TRX_TX_DONE;
                break;
            case I2C_MASTER_EVENT_RX_COMPLETE:
                sCallbackEvent = TRX_RX_DONE;
                break;
            default:
                /** Transfer error. */
                sCallbackEvent = TRX_ERROR;
                break;
        }
    }
}
