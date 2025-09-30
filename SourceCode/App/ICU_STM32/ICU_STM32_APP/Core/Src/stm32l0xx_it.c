/**
  ******************************************************************************
  * @file    stm32l0xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "main.h"
#include "stm32l0xx_it.h"

/* Private includes ----------------------------------------------------------*/
#include "usart.h"
#include "protocol.h"
#include "tim.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t sys_tick = 0;

/* Private function prototypes -----------------------------------------------*/
uint32_t LL_GetTick(void)
{
    return sys_tick;
}

/* Private user code ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/
extern void PtxIrq_Callback(void *p_args);

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable Interrupt.
  */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1)
    {
    }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
    while (1)
    {
    }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/* STM32L0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line 0 and line 1 interrupts.
  */
void EXTI0_1_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);

		if (1 == LL_GPIO_IsInputPinSet(PTX_IRQ_GPIO_Port, PTX_IRQ_Pin)) {
            IRQ_flag = 1;
            PtxIrq_Callback(NULL);
        }
    }

    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
    }
}

/**
  * @brief This function handles EXTI line 4 to 15 interrupts.
  */
void EXTI4_15_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_5) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_5);
        g_overtemp_triggered = 1;
    }

    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_6) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_6);
        g_chrg_flag = 1;
    }
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM2))
    {
        LL_TIM_ClearFlag_UPDATE(TIM2);
        if ((g_frame_timeout > 0) && (g_frame_timeout != MSG_FRAME_RECEIVED))
        {
            g_frame_timeout--;
        }
        if (g_adc_timeout > 0)
        {
            g_adc_timeout--;
        }
        if ((g_stop_timeout > 0) && (g_stop_timeout!= STOP_TIMEOUT))
        {
            g_stop_timeout--;
        }
        if (g_wdt_timeout > 0)
        {
            g_wdt_timeout--;
        }
        if (g_chg_timeout > 0)
        {
            g_chg_timeout--;
        }
        if (g_exit_timeout > 0)
        {
            g_exit_timeout--;
        }

        sys_tick++;
    }
}

/**
  * @brief This function handles TIM21 global interrupt.
  */
void TIM21_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(TIM21)) {
        LL_TIM_ClearFlag_UPDATE(TIM21);
        // printf("TIM21_IRQHandler\n");
        if (NULL != TIM21_PeriodElapsedCallback) {
            TIM21_PeriodElapsedCallback();
            // printf("TIM21 timeout\n");
        }
    }
}

/**
  * @brief This function handles I2C1 event global interrupt / I2C1 wake-up interrupt through EXTI line 23.
  */
void I2C1_IRQHandler(void)
{
}

/**
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
    if (LL_USART_IsActiveFlag_RXNE(USART2))
    {
        uint8_t data = LL_USART_ReceiveData8(USART2);
        LL_USART2_RxCpltCallback(data);
    }
}
