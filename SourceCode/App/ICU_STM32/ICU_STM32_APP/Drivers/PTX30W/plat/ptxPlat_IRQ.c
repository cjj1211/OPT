#include "ptxPlat.h"

#define WAIT_FOR_IRQ_TIMEOUT (20U) /** Timeout for waiting for an IRQ. */

static void safeguardCallback(void *ctx);

static enum {
    IrqStatus_Wait, 
    IrqStatus_Triggered, 
    IrqStatus_Timeout 
} sIrqStatus;

ptxStatus_t ptxPlat_IRQ_Init(void)
{
    ptxStatus_t status = ptxStatus_Success;

    return status;
}

ptxStatus_t ptxPlat_IRQ_Deinit(void)
{
    ptxStatus_t status = ptxStatus_Success;

    return status;
}

ptxStatus_t ptxPlat_IRQ_WaitForIrq(uint32_t timeoutMs)
{
    int pinLevel;
    ptxStatus_t status = ptxStatus_Success;

    sIrqStatus = IrqStatus_Wait;

    delay_ms(1);
    /** Read the pin level first. */
    pinLevel = LL_GPIO_IsInputPinSet(PTX_IRQ_GPIO_Port, PTX_IRQ_Pin);
    // printf("IRQ = %d\n", pinLevel);
    
    /** If the IRQ is not yet triggered, wait for timeout. */
    if (0 == pinLevel)
    {
        if(0 != timeoutMs)
        {
            status = ptxPlat_Timer_StartSafeguardTimer(safeguardCallback, timeoutMs);
            if (ptxStatus_Success == status)
            {
                while (IrqStatus_Wait == sIrqStatus)
                {
                    __DSB();
                    __WFI();
                    __ISB();
                }

                if (IrqStatus_Timeout == sIrqStatus)
                {
                    status = ptxStatus_TimeOut;
                }
            }
            (void)ptxPlat_Timer_StopSafeguardTimer();
        }
        else
        {
            status = ptxStatus_TimeOut;
        }
    }

    return status;
}

void PtxIrq_Callback(void *p_args)
{
    sIrqStatus = IrqStatus_Triggered;
}

static void safeguardCallback(void *ctx)
{
    sIrqStatus = IrqStatus_Timeout;
}
