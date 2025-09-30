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
    File        : ptxPERIPH_APPTIMER.c

    Description :

 */
#include "hal_data.h"
#include "ptxPERIPH_APPTIMER.h"

const timer_instance_t *timer = &g_timer1;

ptxStatus_t ptxPERIPH_APPTIMER_Start(uint32_t ms)
{
    ptxStatus_t status = ptxStatus_Success;

    fsp_err_t r_status = R_GPT_Open(timer->p_ctrl, timer->p_cfg);

    if(FSP_SUCCESS == r_status)
    {
        uint32_t timer_freq_hz = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_PCLKD) >>  timer->p_cfg->source_div;
        uint32_t period_counts = (uint32_t) (((uint64_t) timer_freq_hz * ms) / 1000);
        r_status = R_GPT_PeriodSet(timer->p_ctrl, period_counts);

        if (FSP_SUCCESS == r_status)
        {
            r_status = R_GPT_Start(timer->p_ctrl);
        }
    }

    if(FSP_SUCCESS != r_status)
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InternalError);
    }

    return status;
}

ptxStatus_t ptxPERIPH_APPTIMER_Status(TimerStatus_t *timerStatus)
{
    ptxStatus_t status = ptxStatus_Success;

    if (NULL != timerStatus)
    {
        timer_status_t timer_status;

        fsp_err_t r_status = R_GPT_StatusGet(timer->p_ctrl, &timer_status);
        if (FSP_SUCCESS == r_status)
        {
            if(timer_status.state == TIMER_STATE_STOPPED)
            {
                timerStatus->IsElapsed = 1u;
                timerStatus->ElapsedTime = 0u;
            }
            else
            {
                timerStatus->IsElapsed = 0u;
                uint32_t timer_freq_hz = R_FSP_SystemClockHzGet(FSP_PRIV_CLOCK_PCLKD) >> timer->p_cfg->source_div;
                timerStatus->ElapsedTime = (uint32_t) (((uint64_t) timer_status.counter * 1000u) / timer_freq_hz);
            }
        }
        else
        {
            status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InternalError);
        }
    }
    else
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InvalidParameter);
    }

    return status;
}

ptxStatus_t ptxPERIPH_APPTIMER_Stop()
{
    ptxStatus_t status = ptxStatus_Success;

    fsp_err_t r_status = R_GPT_Stop(timer->p_ctrl);
    if(FSP_SUCCESS == r_status)
    {
        r_status = R_GPT_Close(timer->p_ctrl);
    }

    if(FSP_SUCCESS != r_status)
    {
        status = PTX_STATUS(ptxStatus_Comp_PLAT, ptxStatus_InternalError);
    }

    return status;
}

void ptxPERIPH_APPTIMER_IsrCallback(timer_callback_args_t *p_args)
{
    R_GPT_Stop(timer->p_ctrl);
    (void)p_args;
}

