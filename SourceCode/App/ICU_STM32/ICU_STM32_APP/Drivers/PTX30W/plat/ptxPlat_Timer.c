#include "ptxPlat.h"
#include "tim.h"

#define TIMER_TICKS_PER_MS (1U)

ptxStatus_t ptxPlat_Timer_Init(void)
{
    ptxStatus_t status = ptxStatus_Success;
    MX_TIM21_Init();

    return status;
}

ptxStatus_t ptxPlat_Timer_Deinit(void)
{
    ptxStatus_t status = ptxStatus_Success;
    TIM21_Stop_IT();
    TIM21_DeInit();

    return status;
}

ptxStatus_t ptxPlat_Timer_Start(uint32_t time)
{
    ptxStatus_t status = ptxStatus_Success;
    TIM21->ARR = (time * TIMER_TICKS_PER_MS);
    TIM21_Start_IT();

    return status;
}

ptxStatus_t ptxPlat_Timer_Stop(void)
{
    ptxStatus_t status = ptxStatus_Success;
    TIM21_Stop_IT();

    return status;
}

ptxStatus_t ptxPlat_Timer_SetCb(void *callback)
{
    ptxStatus_t status = ptxStatus_Success;
    TIM21_RegisterCallback(callback);

    return status;
}

ptxStatus_t ptxPlat_Timer_StartSafeguardTimer(void *callback, uint32_t timeout)
{
    ptxStatus_t status = ptxStatus_Success;
    status = ptxPlat_Timer_Init();

    if (ptxStatus_Success == status)
    {
        status = ptxPlat_Timer_SetCb(callback);
    }

    if (ptxStatus_Success == status)
    {
        status = ptxPlat_Timer_Start(timeout);
    }

    return status;
}

ptxStatus_t ptxPlat_Timer_StopSafeguardTimer()
{
    ptxStatus_t status = ptxStatus_Success;
    (void)ptxPlat_Timer_Stop();
    (void)ptxPlat_Timer_SetCb(NULL);
    status = ptxPlat_Timer_Deinit();

    return status;
}

void ptxPlat_Timer_Delay(uint32_t delayMs)
{
    delay_ms(delayMs);
}
