/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PowerKey_Pin GPIO_PIN_0
#define PowerKey_GPIO_Port GPIOA
#define MODE_SEL_Pin GPIO_PIN_1
#define MODE_SEL_GPIO_Port GPIOA
#define STM_ADC_VOUT_Pin GPIO_PIN_2
#define STM_ADC_VOUT_GPIO_Port GPIOA
#define ADC_IOUT_Pin GPIO_PIN_4
#define ADC_IOUT_GPIO_Port GPIOA
#define en_485_adc_Pin GPIO_PIN_5
#define en_485_adc_GPIO_Port GPIOA
#define STM_KEY_Pin GPIO_PIN_6
#define STM_KEY_GPIO_Port GPIOA
#define Buzzer_CTR_Pin GPIO_PIN_7
#define Buzzer_CTR_GPIO_Port GPIOA
#define OC_Pin GPIO_PIN_0
#define OC_GPIO_Port GPIOB
#define LED_Y_Pin GPIO_PIN_1
#define LED_Y_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_2
#define LED_G_GPIO_Port GPIOB
#define LED_R_Pin GPIO_PIN_10
#define LED_R_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_12
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOB
#define NTC_OVER_EN_Pin GPIO_PIN_11
#define NTC_OVER_EN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
