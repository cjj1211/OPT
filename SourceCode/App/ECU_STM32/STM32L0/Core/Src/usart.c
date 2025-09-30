/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
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
#include "usart.h"
#include <stdio.h>
#include "protocol.h"

/* USER CODE BEGIN 0 */
extern uint8_t rx_buf[1];

__asm(".global __use_no_semihosting\n\t");
void _sys_exit(int x)
{
    x = x;
}

void _ttywrch(int ch)
{
    ch = ch;
}

void _sys_command_string(char* cmd, int len)
{
    len = len;
}

FILE __stdout;
int fputc(int ch, FILE * f)
{
    while((USART2->ISR & 0X40) == 0);
    USART2->TDR = (uint8_t) ch;
    return ch;
}
/* USER CODE END 0 */

UART_HandleTypeDef huart2;

/* USART2 init function */

void MX_USART2_UART_Init(void)
{

    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == USART2)
    {
    /* USER CODE BEGIN USART2_MspInit 0 */

    /* USER CODE END USART2_MspInit 0 */
        /* USART2 clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PA9     ------> USART2_TX
        PA10     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    /* USER CODE BEGIN USART2_MspInit 1 */

    /* USER CODE END USART2_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
    if (uartHandle->Instance == USART2)
    {
    /* USER CODE BEGIN USART2_MspDeInit 0 */

    /* USER CODE END USART2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /**USART2 GPIO Configuration
        PA9     ------> USART2_TX
        PA10     ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

        /* USART2 interrupt Deinit */
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    /* USER CODE BEGIN USART2_MspDeInit 1 */

    /* USER CODE END USART2_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (sdu_analyze(&g_rx_buf, rx_buf[0]))
        {
            g_rx_recv_flag = true;
        }
        HAL_UART_Receive_IT(&huart2, rx_buf, sizeof(rx_buf));
    }
}

void uart_send(uint8_t *data, uint8_t len)
{
    HAL_UART_Transmit(&huart2, data, len, 1000);
}

void all_clock_freq_print (void)
{
    printf(" > SYSCLK_Frequency = %d\r\n", HAL_RCC_GetSysClockFreq());
    printf(" > HCLK_Frequency = %d\r\n", HAL_RCC_GetHCLKFreq());
    printf(" > PCLK1_Frequency = %d\r\n", HAL_RCC_GetPCLK1Freq());
    printf(" > PCLK2_Frequency = %d\r\n", HAL_RCC_GetPCLK2Freq());
}
/* USER CODE END 1 */
