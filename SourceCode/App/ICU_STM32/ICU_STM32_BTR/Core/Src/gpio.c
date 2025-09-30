/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  *
  *  版权所有（C）2022-2024，上海脑虎科技有限公司，保留所有权利。
  *  作者 : 崔俊杰 junjie.cui@neuroxess.com
  *  描述 : GPIO
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/

/** Configure pins as
    * Analog
    * Input
    * Output
    * EVENT_OUT
    * EXTI
*/
void MX_GPIO_Init(void)
{
    LL_EXTI_InitTypeDef EXTI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);

    LL_GPIO_ResetOutputPin(EN_Icharge_GPIO_Port, EN_Icharge_Pin);
    LL_GPIO_ResetOutputPin(PTX30W_SM_GPIO_Port, PTX30W_SM_Pin);
    LL_GPIO_SetOutputPin(NRF_nReset_GPIO_Port, NRF_nReset_Pin);
    LL_GPIO_SetOutputPin(VBLE_SEL_GPIO_Port, VBLE_SEL_Pin);
    LL_GPIO_ResetOutputPin(VSYS_ON_GPIO_Port, VSYS_ON_Pin);

    /**/
    GPIO_InitStruct.Pin = EN_Icharge_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(EN_Icharge_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = PTX30W_SM_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(PTX30W_SM_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = NRF_nReset_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    LL_GPIO_Init(NRF_nReset_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = VBLE_SEL_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(VBLE_SEL_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin = VSYS_ON_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(VSYS_ON_GPIO_Port, &GPIO_InitStruct);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE1);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE5);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE6);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);

    LL_GPIO_SetPinPull(PTX_IRQ_GPIO_Port, PTX_IRQ_Pin, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(T_EXIT_GPIO_Port, T_EXIT_Pin, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(PTX_GPIO1_GPIO_Port, PTX_GPIO1_Pin, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(PTX_GPIO0_GPIO_Port, PTX_GPIO0_Pin, LL_GPIO_PULL_DOWN);

    LL_GPIO_SetPinMode(PTX_IRQ_GPIO_Port, PTX_IRQ_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(T_EXIT_GPIO_Port, T_EXIT_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(PTX_GPIO1_GPIO_Port, PTX_GPIO1_Pin, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinMode(PTX_GPIO0_GPIO_Port, PTX_GPIO0_Pin, LL_GPIO_MODE_INPUT);

    /**/
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_1;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);

    /**/
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_5;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);

    /**/
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_6;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
    LL_EXTI_Init(&EXTI_InitStruct);

    /**/
    EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
    EXTI_InitStruct.LineCommand = ENABLE;
    EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
    EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING;
    LL_EXTI_Init(&EXTI_InitStruct);

    /* EXTI interrupt init*/
    NVIC_SetPriority(EXTI0_1_IRQn, 0);
    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 0);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
}
