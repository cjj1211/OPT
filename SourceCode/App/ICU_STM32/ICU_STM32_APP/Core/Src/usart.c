/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  *
  *  版权所有（C）2022-2025，上海脑虎科技有限公司，保留所有权利。
  *  作者 : 崔俊杰 junjie.cui@neuroxess.com
  *  描述 : UART
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "protocol.h"
#include "usart.h"

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

/* USART2 init function */
void MX_USART2_UART_Init(void)
{
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

    /**USART2 GPIO Configuration
      PA9   ------> USART2_TX
      PA10   ------> USART2_RX
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn);

    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART2, &USART_InitStruct);
    LL_USART_ConfigAsyncMode(USART2);
    LL_USART_Enable(USART2);
    LL_USART_EnableIT_RXNE(USART2);
}

void MX_USART2_UART_DeInit(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    NVIC_DisableIRQ(USART2_IRQn);
}

void USART2_SendData(uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        while (!LL_USART_IsActiveFlag_TXE(USART2));

        LL_USART_TransmitData8(USART2, data[i]);
    }

    while (!LL_USART_IsActiveFlag_TC(USART2));
}

void LL_USART2_RxCpltCallback(uint8_t data)
{
    if (sdu_analyze(&g_rx_buf, data))
    {
        g_rx_recv_flag = true;
    }
}
