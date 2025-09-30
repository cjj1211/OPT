/**
 * @file bsp.c
 * @author DCX (chunxiao.dai@neuroxess.com)
 * @brief general BSP API
 * @version 0.1
 * @date 2025-03-06
 *
 * @copyright Copyright (C) 2025 NeuroXess
 *
 */

#include "bsp.h"
#include "stm32l0xx_ll_cortex.h"

uint32_t NX_SysTick_Config(uint32_t ticks)
{
    if ((ticks - 1) > SysTick_LOAD_RELOAD_Msk)  return (1);      /* Reload value impossible */

    SysTick->LOAD  = ticks - 1;                                  /* set reload register */
    NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Systick Interrupt */
    SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
    return (0);                                                  /* Function successful */
}

/**
 * @brief delay function (us)
 *
 * @param delay us, < 1000
 */
__inline void delay_us(uint16_t delay)
{
    uint32_t tickstart = SysTick->VAL;
    uint32_t tickNum = 0;
    uint32_t tickMax = SysTick->LOAD + 1;
    uint32_t delay_usvalue = (tickMax / 1000) * delay;

    while (1)
    {
        uint32_t cur_tick = SysTick->VAL;
        if (cur_tick > tickstart)
        {
            tickNum = tickstart + (tickMax - cur_tick);
        }
        else
        {
            tickNum = tickstart - cur_tick;
        }

        if (tickNum > delay_usvalue)
        {
            return;
        }
    }
}

/**
 * @brief delay function(ms)
 *
 * @param delay ms
 */
void delay_ms(uint32_t delay)
{
    for (int i = 0; i < 10 * delay; i++)
    {
        delay_us(100);
    }
}
