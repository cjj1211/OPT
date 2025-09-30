#include "ptxPlat.h"
#include "ptx30w_drv.h"
#include "i2c.h"
#include <string.h>

#define I2C_COM_RETRY (2U)

static void safeguardCallback(void *ctx);

static volatile ptxPlat_I2CTRxStatus_t sCallbackEvent;

ptxStatus_t ptxPlat_I2C_Init(void)
{
    ptxStatus_t status = ptxStatus_Success;
    return status;
}

ptxStatus_t ptxPlat_I2C_Deinit(void)
{
    ptxStatus_t status = ptxStatus_Success;
    return status;
}

ptxStatus_t ptxPlat_I2C_TRx(
    const uint8_t *txBuf, size_t txLen, uint8_t restart, uint8_t *rxBuf, size_t rxLen)
{
    ptxStatus_t status = ptxStatus_Success;
    I2C_Status statusI2C = I2C_OK;

    /** Let's see if there is something to write. */
    if ((NULL != txBuf) && (0 < txLen))
    {
        /** Start a Guard timer to avoid being locked in waiting for a interrupt if there is a HW
         * issue. */
        // status = ptxPlat_Timer_StartSafeguardTimer(safeguardCallback, PTX_PLAT_I2C_TIMEOUT);
        if (ptxStatus_Success == status) {
            statusI2C = I2C_Write(0x4B, (uint8_t *)txBuf, txLen, restart);
            if (I2C_OK != statusI2C) {
                printf("I2C Write failed[%d]\n", statusI2C);
                status = ptxStatus_InterfaceError;
            }

            // sCallbackEvent = TRX_TX_INPROGRESS;
            /** Tx part of the overall transaction. */
            // if (HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, (0x4B << 1), (uint8_t *)txBuf, txLen, I2C_FIRST_FRAME) == HAL_OK)
            // {
            //     while (TRX_TX_INPROGRESS == sCallbackEvent)
            //     {
            //         HAL_Delay(1);
            //     }
            //     if (TRX_TX_DONE != sCallbackEvent)
            //     {
            //         status = ptxStatus_InterfaceError;
            //     }
            // } else {
            //     printf("I2C Tx failed\n");
            // }
        } else {
            status = ptxStatus_InterfaceError;
        }
        // (void)ptxPlat_Timer_StopSafeguardTimer();

        if (ptxStatus_Success == status) {
            /** Let's see if there is something to read. Only 1 buffer allowed for reception. */
            if ((NULL != rxBuf) && (0 < rxLen)) {
                /** Start a Guard timer to avoid being locked in waiting for a interrupt if there is
                 * a HW issue. */
                // status = ptxPlat_Timer_StartSafeguardTimer(safeguardCallback, PTX_PLAT_I2C_TIMEOUT);
                if (ptxStatus_Success == status) {
                    statusI2C = I2C_Read(0x4B, rxBuf, rxLen);
                    if (I2C_OK != statusI2C) {
                        printf("I2C Read failed[%d]\n", statusI2C);
                        status = ptxStatus_InterfaceError;
                    }

                    // sCallbackEvent = TRX_RX_INPROGRESS;
                    /** Rx part of the overall transaction. */
                    // if (HAL_I2C_Master_Seq_Receive_IT(&hi2c1, (0x4B << 1), rxBuf, rxLen, I2C_LAST_FRAME) == HAL_OK)
                    // {
                    //     while (TRX_RX_INPROGRESS == sCallbackEvent)
                    //     {
                    //         HAL_Delay(1);
                    //     }
                    //     if (TRX_RX_DONE != sCallbackEvent)
                    //     {
                    //         status = ptxStatus_InterfaceError;
                    //     }
                    // }
                    // else
                    // {
                    //     printf("I2C Rx failed\n");
                    //     status = ptxStatus_InterfaceError;
                    // }
                }
                // (void)ptxPlat_Timer_StopSafeguardTimer();
            }
        }

        if (ptxStatus_Success != status)
        {
            /**
             * If I2C low level operation failed, peripheral must be re-started due to low-level
             * reset. However, forward error status to the caller in order to start re-try
             * procedure.
             */
            (void)ptxPlat_I2C_Reset();
        }
    }
    else
    {
        status = ptxStatus_InvalidParameter;
    }

    return status;
}


ptxStatus_t ptxPlat_I2C_TRx_Retry(
    const uint8_t *txBuf, size_t txLen, uint8_t restart, uint8_t *rxBuf, size_t rxLen)
{
    ptxStatus_t status = ptxStatus_Success;
    uint8_t retry = I2C_COM_RETRY;
    do
    {
        status = ptxPlat_I2C_TRx(txBuf, txLen, restart, rxBuf, rxLen);
        /** If there is a failure, we must wait a bit until the PTX30W boots up and retry. */
        if (ptxStatus_Success != status)
        {
            delay_ms(1);
        }
        retry--;
    } while ((ptxStatus_Success != status) && (0 != retry));

    return status;
}

ptxStatus_t ptxPlat_I2C_SetSlaveAddress(uint16_t slaveAddress)
{
    ptxStatus_t status = ptxStatus_Success;
    /** Sets the address for the I2C slave device. */

    return status;
}

ptxStatus_t ptxPlat_I2C_Reset(void)
{
    ptxStatus_t status = ptxStatus_Success;

    // sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) &g_i2c_driver_ctrl;
    uint32_t address = PTX30W_I2C_ADDRESS;

    /** Deinitialize and reinitialize the I2C HW module. */
    status = ptxPlat_I2C_Deinit();

    if (ptxStatus_Success == status)
    {
        status = ptxPlat_I2C_Init();
    }

    if (ptxStatus_Success == status)
    {
        status = ptxPlat_I2C_SetSlaveAddress((uint16_t) address);
    }

    return status;
}


/** Callback of safeguard timer. */
static void safeguardCallback(void *ctx)
{
    sCallbackEvent = TRX_TIMEOUT;
}
