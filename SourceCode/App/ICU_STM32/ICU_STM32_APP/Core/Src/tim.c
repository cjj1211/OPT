/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* TIM2 init function */
void MX_TIM2_Init(void)
{
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    /* TIM2 interrupt Init */
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);

    TIM_InitStruct.Prescaler = 15;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 999;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM2, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(TIM2);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);

    LL_TIM_EnableIT_UPDATE(TIM2);
    LL_TIM_EnableCounter(TIM2);
}

/* TIM21 init function */
void MX_TIM21_Init(void)
{
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM21);

    /* TIM21 interrupt Init */
    NVIC_SetPriority(TIM21_IRQn, 0);
    NVIC_EnableIRQ(TIM21_IRQn);

    TIM_InitStruct.Prescaler = 15999;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 499;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM21, &TIM_InitStruct);
    LL_TIM_EnableARRPreload(TIM21);
    LL_TIM_SetClockSource(TIM21, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM21, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM21);
}

void MX_TIM2_DeInit(void)
{
    if (LL_TIM_IsEnabledCounter(TIM2))
    {
        LL_TIM_DisableCounter(TIM2);
    }

    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_TIM2);
    NVIC_DisableIRQ(TIM2_IRQn);
}

void MX_TIM21_DeInit(void)
{
    if (LL_TIM_IsEnabledCounter(TIM21)) {
        LL_TIM_DisableCounter(TIM21);
    }

    LL_APB1_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM21);
    NVIC_DisableIRQ(TIM21_IRQn);
}

void TIM21_Start_IT(void)
{
    LL_TIM_EnableCounter(TIM21);
    LL_TIM_EnableIT_UPDATE(TIM21);
}

void TIM21_Stop_IT(void)
{
    LL_TIM_DisableIT_UPDATE(TIM21);
    LL_TIM_DisableCounter(TIM21);
}

void TIM21_DeInit(void)
{
    LL_APB1_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM21);
}

void (*TIM21_PeriodElapsedCallback)(void) = NULL;

void TIM21_RegisterCallback(void (*callback)(void))
{
    TIM21_PeriodElapsedCallback = callback;
}
