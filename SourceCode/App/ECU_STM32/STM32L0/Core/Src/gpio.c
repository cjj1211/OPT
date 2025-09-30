/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include <stdio.h>

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
volatile bool g_pwr_key_pressed = false;
volatile bool g_mod_sel_pressed = false;
/* USER CODE END 1 */

/** Configure pins as
    * Analog
    * Input
    * Output
    * EVENT_OUT
    * EXTI
*/
void MX_GPIO_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, en_485_adc_Pin|STM_KEY_Pin|Buzzer_CTR_Pin|NTC_OVER_EN_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, LED_Y_Pin|LED_G_Pin|LED_R_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : PowerKey_Pin MODE_SEL_Pin */
    GPIO_InitStruct.Pin = PowerKey_Pin|MODE_SEL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : en_485_adc_Pin STM_KEY_Pin Buzzer_CTR_Pin NTC_OVER_EN_Pin */
    GPIO_InitStruct.Pin = en_485_adc_Pin|STM_KEY_Pin|Buzzer_CTR_Pin|NTC_OVER_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : OC_Pin TEST_IN_Pin */
    GPIO_InitStruct.Pin = OC_Pin|TEST_IN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : LED_Y_Pin LED_G_Pin LED_R_Pin */
    GPIO_InitStruct.Pin = LED_Y_Pin|LED_G_Pin|LED_R_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

}

/* USER CODE BEGIN 2 */
/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected to the EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
//  HAL_GPIO_TogglePin(GPIOA, Buzzer_CTR_Pin);
    switch (GPIO_Pin)
    {
        case PowerKey_Pin:
//      printf("PowerKey pressed\n");
            g_pwr_key_pressed = true;
            break;
        case MODE_SEL_Pin:
//      printf("ModeSelKey pressed\n");
            g_mod_sel_pressed = true;
            break;
        default:
            break;
    }
}
/* USER CODE END 2 */
