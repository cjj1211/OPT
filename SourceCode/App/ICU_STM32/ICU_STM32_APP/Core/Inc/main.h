/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32l0xx_ll_adc.h"
#include "stm32l0xx_ll_i2c.h"
#include "stm32l0xx_ll_iwdg.h"
#include "stm32l0xx_ll_crs.h"
#include "stm32l0xx_ll_rcc.h"
#include "stm32l0xx_ll_bus.h"
#include "stm32l0xx_ll_system.h"
#include "stm32l0xx_ll_exti.h"
#include "stm32l0xx_ll_cortex.h"
#include "stm32l0xx_ll_utils.h"
#include "stm32l0xx_ll_pwr.h"
#include "stm32l0xx_ll_dma.h"
#include "stm32l0xx_ll_tim.h"
#include "stm32l0xx_ll_usart.h"
#include "stm32l0xx_ll_gpio.h"
#include "stm32l0xx_ll_lptim.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
#include "bsp.h"
#include <stdio.h>
#include <string.h>

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
extern volatile uint16_t g_adc_timeout;
extern volatile uint32_t g_stop_timeout;
extern volatile uint16_t g_wdt_timeout;
extern volatile uint16_t g_chg_timeout;
extern volatile uint16_t g_exit_timeout;
extern volatile uint16_t g_ptx_timeout;
extern volatile uint8_t g_chrg_flag;
extern volatile uint8_t g_overtemp_triggered;
extern volatile uint8_t IRQ_flag;
extern uint16_t voltage_mV;
extern uint16_t ptx_vddc_mV;
extern int16_t current_ma;
extern uint8_t ble_status;

/* Exported macro ------------------------------------------------------------*/
#define RUN_MODE_BTR    (0u)
#define RUN_MODE_APP    (1u)

#define START_POWERUP  (0xC0)
#define START_WKUP     (0xA5)
#define START_PRE_STOP (0x5A)

#define STOP_TIMEOUT    (1000 * 60 * 5)

//!< Wake-up source
#define CHG_TRIG_SRC   0xC0  // Triggered by GPIO_1 of PTX30W
#define HOT_TRIG_SRC   0xA5  // Triggered by Alert pin of TMP119 (High limit)
#define LOT_TRIG_SRC   0x5A  // Triggered by Alert pin of TMP119 (Low limit)


/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

void printf_hex(uint8_t *ptr, uint32_t len);
void Pre_Enter_Stop_Mode(void);

/* Private defines -----------------------------------------------------------*/
#define EN_Icharge_Pin        LL_GPIO_PIN_3
#define EN_Icharge_GPIO_Port  GPIOB
#define PTX_IRQ_Pin           LL_GPIO_PIN_1
#define PTX_IRQ_GPIO_Port     GPIOA
#define PTX_IRQ_EXTI_IRQn     EXTI0_1_IRQn
#define PTX30W_SM_Pin         LL_GPIO_PIN_8
#define PTX30W_SM_GPIO_Port   GPIOA
#define NRF_nReset_Pin        LL_GPIO_PIN_7
#define NRF_nReset_GPIO_Port  GPIOA
#define VBLE_SEL_Pin          LL_GPIO_PIN_1
#define VBLE_SEL_GPIO_Port    GPIOB
#define T_EXIT_Pin            LL_GPIO_PIN_5
#define T_EXIT_GPIO_Port      GPIOA
#define T_EXIT_EXTI_IRQn      EXTI4_15_IRQn
#define VSYS_ON_Pin           LL_GPIO_PIN_0
#define VSYS_ON_GPIO_Port     GPIOB
#define PTX_GPIO1_Pin         LL_GPIO_PIN_6
#define PTX_GPIO1_GPIO_Port   GPIOA
#define PTX_GPIO0_Pin         LL_GPIO_PIN_0
#define PTX_GPIO0_GPIO_Port   GPIOA
#define PTX_GPIO0_EXTI_IRQn   EXTI0_1_IRQn

#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
