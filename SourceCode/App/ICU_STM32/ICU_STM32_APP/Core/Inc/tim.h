/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
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
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

void MX_TIM2_Init(void);
void MX_TIM2_DeInit(void);

void MX_TIM21_Init(void);
void MX_TIM21_DeInit(void);

void TIM21_DeInit(void);
void TIM21_Stop_IT(void);
void TIM21_Start_IT(void);

void TIM21_RegisterCallback(void (*callback)(void));

extern void (*TIM21_PeriodElapsedCallback)(void);

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */
