/** \file
   ---------------------------------------------------------------
   Copyright (C) 2023. Panthronics AG - All Rights Reserved.

   This material may not be reproduced, displayed, modified or
   distributed without the express prior written permission of the
   Panthronics AG.

   PLEASE REFER TO OUR DISCLAIMER NOTICE IN THE PROJECT ROOT.
   ---------------------------------------------------------------

    Project     : PTX1K
    Module      : PLAT
    File        : ptxPLAT_SPI.c

    Description : basic SPI implementation on Renesas RA4M2 platform.
         1) Pins: SPI0. SPI_CK = P4_12, MOSI = P4_11, MISO = P4_10, CS = P4_13
         2) Spi instance is automatically generated during build. Spi Open assigns this instance to the pointer.
         3) Callback function has to be defined - it is used to report the actual status of the transfer.

 */

/*
 * ####################################################################################################################
 * INCLUDES
 * ####################################################################################################################
 */
#include "ptxPLAT_SPI.h"
#include "ptxPLAT_EXT.h"
#include <string.h>

/*
 * ####################################################################################################################
 * DEFINES / TYPES
 * ####################################################################################################################
 */

/**
 * SPI instance type wrapper.
 */
typedef spi_instance_t ptxPLAT_SpiInstance_t;

/*
 * ####################################################################################################################
 * DEFINES
 * ####################################################################################################################
 */

#define PTX_PLAT_SPI_CS_LOW             BSP_IO_LEVEL_LOW
#define PTX_PLAT_SPI_CS_HIGH            BSP_IO_LEVEL_HIGH



#ifdef POLLER_BOARD
    #define PTX_PLAT_SPI_CHANNEL                (0u)
#else
    #define PTX_PLAT_SPI_CHANNEL                (0u)
#endif
/*
 * ####################################################################################################################
 * STATIC VARIABLES
 * ####################################################################################################################
 */

/**
 * Instance of available (predefined) spi port driver.
 */
const ptxPLAT_SpiPort_t available_spi_port =
{

# ifdef POLLER_BOARD
        .SpiInstance = (ptxPLAT_SpiInstance_t *)&g_spi0,
        .Nss = {&g_ioport, IOPORT_PORT_01_PIN_03}
# else
        .SpiInstance = (ptxPLAT_SpiInstance_t *)&g_pmod2_spi,
        .Nss = {&g_ioport, IOPORT_PORT_04_PIN_13}
# endif

};

/**
 * Instance of SPI context.
 */
ptxPLAT_Spi_t spi_ctx;


/*
 * ####################################################################################################################
 * INTERNAL FUNCTIONS
 * ####################################################################################################################
 */

static ptxStatus_t ptxPLAT_SPI_PortOpen(ptxPLAT_SpiPort_t **spiPort, ptxPLAT_SpiConfigPars_t *spiPars);
static ptxStatus_t ptxPLAT_SPI_PortClose(ptxPLAT_SpiPort_t *spiPort);
static ptxStatus_t ptxPLAT_SPI_TxAux(ptxPLAT_Spi_t *spi, uint8_t *txBuff, size_t *len);
static ptxStatus_t ptxPLAT_SPI_RxAux(ptxPLAT_Spi_t *spi, uint8_t *rxBuff, size_t *len);
static ptxStatus_t ptxPLAT_SPI_TRx_Helper(ptxPLAT_Spi_t *spi, uint8_t *txBuf[], size_t txLen[], size_t numTxBuffers, uint8_t *rxBuf[], size_t *rxLen[], size_t numRxBuffers);
static ptxStatus_t ptxPLAT_SPI_SetTRxState(ptxPLAT_Spi_t *spi, uint8_t state);
static ptxStatus_t ptxPLAT_SPI_SetChipSelect(ptxPLAT_SpiPort_t *spiPort, uint8_t newState);


/*
 * ####################################################################################################################
 * API FUNCTIONS
 * ####################################################################################################################
 */

ptxStatus_t ptxPLAT_SPI_GetInitialized(ptxPLAT_Spi_t **spi, ptxPLAT_SpiConfigPars_t *spiPars)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((NULL != spi) && (NULL != spiPars))
    {
        /**
         * Initialization of the SPI Context
         */

        ptxPLAT_SpiPort_t *spi_port = NULL;
        memset(&spi_ctx, 0, sizeof(ptxPLAT_Spi_t));

        status = ptxPLAT_SPI_PortOpen(&spi_port, spiPars);

        if (ptxStatus_Success == status)
        {
            ptxPlatGpio_Config_t gpio_pars;
            gpio_pars.Type = GPIO_Type_PTXExtIRQPin;

            status = ptxPLAT_GPIO_GetInitialized(&spi_ctx.Gpio, &gpio_pars);

            if((ptxStatus_Success == status) && (NULL != spi_port))
            {
                spi_ctx.SpiPortUsed = spi_port;
                *spi = &spi_ctx;
            } else
            {
                /**
                 *  Clean up here: close SPI port instance since it has been successfully opened.
                 */
                ptxPLAT_SPI_PortClose(spi_port);

                status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InternalError);
            }
        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InternalError);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxPLAT_SPI_Deinit(ptxPLAT_Spi_t *spi)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != spi)
    {
        /**
         * De Init tasks shall be performed here:
         *  close spi port - power down the peripheral
         *  de-init external irq gpio module
         */
        ptxPLAT_SPI_PortClose(spi->SpiPortUsed);

        ptxPLAT_GPIO_Deinit(spi->Gpio);

        /** Clean spi context structure. */
        memset(&spi_ctx, 0, sizeof(ptxPLAT_Spi_t));
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }
    return status;
}

ptxStatus_t ptxPLAT_SPI_TRx(ptxPLAT_Spi_t *spi, uint8_t *txBuf[], size_t txLen[], size_t numTxBuffers, uint8_t *rxBuf[], size_t *rxLen[], size_t numRxBuffers, uint8_t flags)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((NULL != spi) && (NULL != txBuf) && (NULL != txLen))
    {
        /**
         * For any transmission operation over any interface the interrupt from GPIO-IRQ shall be disabled
         * to avoid potential mutual use of SPI driver.
         */

        uint32_t int_enable_status = 0xFFFFFFFFU;

        /** If function call is successful, returned value will be 0 or 1. */
        ptxPLAT_GPIO_GetIntEnableStatus (spi->Gpio, &int_enable_status);

        ptxPLAT_GPIO_DisableInterrupt (spi->Gpio);
        status = ptxPLAT_SPI_TRx_Helper(spi, txBuf, txLen, numTxBuffers, rxBuf, rxLen, numRxBuffers);

        if (1u == int_enable_status)
        {
            /** Restore interrupt state. */
            ptxPLAT_GPIO_EnableInterrupt (spi->Gpio);
        }

    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    (void)flags;

    return status;
}

ptxStatus_t ptxPLAT_SPI_StartWaitForRx(ptxPLAT_Spi_t *spi, pptxPlat_RxCallBack_t irqCb, void *ctxIrqCb)
{
    ptxStatus_t status = ptxStatus_Success;

    if ((NULL != spi) && (NULL != irqCb) && (NULL != ctxIrqCb))
    {
        /** This function shall ensure that IRQ is not high before starting the wait for the event. */

        /** First register the callback function and the context, for future use */
        spi->RxCb = irqCb;
        spi->CtxRxCb = ctxIrqCb;

        /** Read IRQ first to check if it is already HIGH.*/
        uint8_t value_IRQ = 0;
        status = ptxPLAT_GPIO_ReadLevel(spi->Gpio, &value_IRQ);

        if(1u == value_IRQ)
        {
            /** IRQ is already high, so let's trigger the asynchronous event now to prevent than race condition has happened. */
            spi->RxCb(spi->CtxRxCb);
        }

        /** Enable waiting for the trigger on IRQ (Not blocking). ISR on rising edge. */
        status = ptxPLAT_GPIO_StartWaitForTrigger(spi->Gpio, (pptxPlat_IRQCallBack_t)irqCb, ctxIrqCb);

    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxPLAT_SPI_StopWaitForRx(ptxPLAT_Spi_t *spi)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != spi)
    {
        /** Stop the wait for the trigger on IRQ (Not blocking) */
        status = ptxPLAT_GPIO_StopWaitForTrigger(spi->Gpio);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }
    return status;
}

void ptxPLAT_SPI_TransferCallback(spi_callback_args_t *p_args)
{
    if((NULL != p_args) && (PTX_PLAT_SPI_CHANNEL == p_args->channel))
    {
        switch(p_args->event)
        {
            case SPI_EVENT_TRANSFER_COMPLETE:
                ptxPLAT_SPI_SetTRxState(&spi_ctx, 1u);
                break;

            default:
                /** Transfer error. */
                ptxPLAT_SPI_SetTRxState(&spi_ctx, 0xFFu);
                break;
        }
    } else
    {
        /** Transfer error. */
        ptxPLAT_SPI_SetTRxState(&spi_ctx, 0xFFu);
    }
}


/*
 * ####################################################################################################################
 * LOCAL FUNCTIONS
 * ####################################################################################################################
 */
static ptxStatus_t ptxPLAT_SPI_SetTRxState(ptxPLAT_Spi_t *spi, uint8_t state)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != spi)
    {
        spi->TransferState = state;
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxPLAT_SPI_PortClose(ptxPLAT_SpiPort_t *spiPort)
{
    ptxStatus_t status = ptxStatus_Success;

    if(NULL != spiPort)
    {
        /**
         * Close spi port: power down the peripheral.
         */
        ptxPLAT_SpiInstance_t *spi_instance = (ptxPLAT_SpiInstance_t *)spiPort->SpiInstance;
        fsp_err_t r_status = R_SCI_SPI_Close(spi_instance->p_ctrl);
        if(FSP_SUCCESS != r_status)
        {
            status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InternalError);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxPLAT_SPI_PortOpen(ptxPLAT_SpiPort_t **spiPort, ptxPLAT_SpiConfigPars_t *spiPars)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != spiPars) && (NULL != spiPort))
    {
        /**
         * It is assumed that the driver (interface API + data structures) for SPI port has already been generated.
         * Here we only set the speed.
         * Default configuration structure has to be wrapped and modified to change speed.
         */
        if (PTX_PLAT_HOST_SPEED_SPI_MAX >= spiPars->IntfSpeed)
        {
            ptxPLAT_SpiInstance_t *spi_instance = (ptxPLAT_SpiInstance_t *)available_spi_port.SpiInstance;

            sci_spi_div_setting_t br_setting;
            fsp_err_t r_status = R_SCI_SPI_CalculateBitrate(spiPars->IntfSpeed, &br_setting, false);

            if(FSP_SUCCESS == r_status)
            {
                r_status = R_SCI_SPI_Open(spi_instance->p_ctrl, spi_instance->p_cfg);
            }

            if (FSP_SUCCESS == r_status)
            {
                /**
                 * Set the SFRs responsible for generating the desired clock frequency.
                 */
                sci_spi_instance_ctrl_t *sci_reg_intfc = ((sci_spi_instance_ctrl_t *) spi_instance->p_ctrl);
                sci_reg_intfc->p_reg->SMR = (uint8_t) (R_SCI0_SMR_CM_Msk | (uint8_t) (br_setting.cks << R_SCI0_SMR_CKS_Pos));
                sci_reg_intfc->p_reg->BRR = br_setting.brr;
                sci_reg_intfc->p_reg->MDDR = br_setting.mddr;

                *spiPort = (ptxPLAT_SpiPort_t *)&available_spi_port;
            }
            else
            {
                status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InternalError);
            }

        } else
        {
            status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxPLAT_SPI_TxAux(ptxPLAT_Spi_t *spi, uint8_t *txBuff, size_t *len)
{
    /**
     * All input arguments shall be checked by the caller. Here, it is expected all are valid i.e. not NULL and
     * allocated enough memory for Rx operation.
     *
     * What about setting a safeguard timer before the wait-for-transfer-end loop?
     */
    ptxStatus_t status = ptxStatus_Success;

    ptxPLAT_SpiInstance_t *spi_instance = (ptxPLAT_SpiInstance_t *)spi->SpiPortUsed->SpiInstance;
    uint32_t length = (uint32_t)*len;

    ptxPLAT_SPI_SetTRxState(spi, 0);

    fsp_err_t _st = R_SCI_SPI_Write(spi_instance->p_ctrl, txBuff, length, SPI_BIT_WIDTH_8_BITS);
    if(FSP_SUCCESS != _st)
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InterfaceError);
    }

    if(ptxStatus_Success == status)
    {
        /** Wait until transfer notification is received via callback. */
        while (0 == spi->TransferState);

        if(1u != spi->TransferState)
        {
            status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InterfaceError);
        }
    }

    return status;
}

static ptxStatus_t ptxPLAT_SPI_RxAux(ptxPLAT_Spi_t *spi, uint8_t *rxBuff, size_t *len)
{
    /**
     * All input arguments shall be checked by the caller. Here, it is expected all are valid i.e. not NULL and
     * allocated enough memory for Rx operation.
     *
     * What about setting a safeguard timer before the wait-for-transfer-end loop?
     */
    ptxStatus_t status = ptxStatus_Success;

    ptxPLAT_SpiInstance_t *spi_instance = (ptxPLAT_SpiInstance_t *)spi->SpiPortUsed->SpiInstance;
    uint32_t length = (uint32_t)*len;

    ptxPLAT_SPI_SetTRxState(spi, 0);

    fsp_err_t r_status = R_SCI_SPI_Read(spi_instance->p_ctrl, rxBuff, length, SPI_BIT_WIDTH_8_BITS);
    if(FSP_SUCCESS != r_status)
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InterfaceError);
    }

    if(ptxStatus_Success == status)
    {
        /** Wait until transfer notification is received via callback. */
        while (0 == spi->TransferState);

        if (1u != spi->TransferState)
        {
            status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InterfaceError);
        }
    }

    return status;
}

static ptxStatus_t ptxPLAT_SPI_SetChipSelect(ptxPLAT_SpiPort_t *spiPort, uint8_t newState)
{
    ptxStatus_t status = ptxStatus_Success;

    if((NULL != spiPort) && ((PTX_PLAT_SPI_CS_LOW == newState) || (PTX_PLAT_SPI_CS_HIGH == newState)))
    {
        fsp_err_t r_status = R_IOPORT_PinWrite(spiPort->Nss.PortInstance->p_ctrl, spiPort->Nss.PinNumber, newState);
        if(FSP_SUCCESS != r_status)
        {
            status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InterfaceError);
        }
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

static ptxStatus_t ptxPLAT_SPI_TRx_Helper(ptxPLAT_Spi_t *spi, uint8_t *txBuf[], size_t txLen[], size_t numTxBuffers, uint8_t *rxBuf[], size_t *rxLen[], size_t numRxBuffers)
{
    ptxStatus_t status = ptxStatus_Success;

    /**
     * Tx operation is required always: to send and to receive anything on SPI. So, tx buffers have to be provided always.
     */

    if ((NULL != spi) && (NULL != spi->SpiPortUsed) && (NULL != txBuf) && (NULL != txLen))
    {
        uint8_t i =0;

        /** At this point the SPI transfer operation is triggered */
        ptxPLAT_SPI_SetChipSelect(spi->SpiPortUsed, PTX_PLAT_SPI_CS_LOW);

        /** Tx part of the overall transaction. */
        i = 0;
        while ((ptxStatus_Success == status) && (i < numTxBuffers))
        {
            if ((txBuf[i] != NULL) && (txLen[i]>0))
            {
                size_t trx_len = txLen[i];
                status = ptxPLAT_SPI_TxAux (spi, txBuf[i], &trx_len);
            } else
            {
                status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
            }

            i++;
        }

        if (ptxStatus_Success == status)
        {
            /** Let's see if there is something to read. */
            if ((NULL != rxBuf) && (NULL != rxLen))
            {
                i = 0;
                while((ptxStatus_Success == status) && (i < numRxBuffers))
                {
                    if ((rxBuf[i] != NULL) && (rxLen[i] != NULL) && (*rxLen[i] > 0))
                    {
                        status = ptxPLAT_SPI_RxAux(spi, rxBuf[i], rxLen[i]);
                    } else
                    {
                        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
                    }
                    i++;
                }
            }
        }

        /** In any case, at this point the SPI transfer operation is finished */
        ptxPLAT_SPI_SetChipSelect(spi->SpiPortUsed, PTX_PLAT_SPI_CS_HIGH);
    } else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

