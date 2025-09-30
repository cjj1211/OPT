/**
 * @file hmi.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-01-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "main.h"
#include "hmi.h"
#include "gpio.h"
#include "protocol.h"
#include <stdio.h>


volatile uint16_t g_keep_alive = 30;
extern volatile bool is_stop_enter;
/**
  * @brief low-power modes config
  * 
  */
static void low_power_mode_enter(void)
{
    /* Enable Power Clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Enter STOP mode */
//    EXTI->PR = 0;
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    /* Restet */
    __disable_irq();
    HAL_NVIC_SystemReset();
}

/**
  * @brief power down device
  *        Not actual power off, in fact, into low-power mode
  * 
  */
void power_down(void)
{
    printf("power_down\n");
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // peripheral config
    GPIO_InitStruct.Pin = STM_KEY_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_RESET);

    //!< actual operation: enter low-power mode
    low_power_mode_enter();
}

/**
  * @brief check SYS_WKUP(PA0) key
  * @details
  */
void wakeup_key_listen (void)
{
    uint16_t press_cnt = 0;

    while (g_pwr_key_pressed)
    {
        HAL_Delay(10);
        if (HAL_GPIO_ReadPin(PowerKey_GPIO_Port, PowerKey_Pin) == GPIO_PIN_RESET)
        {
            press_cnt++;
            if (press_cnt >= (1 * 1000 / 10 + 1))
            {
                led_off(LED_Y);
            }
        }
        else
        {
#if defined(LOG_DBG)
            printf("key up, press_time = %d\r\n", press_cnt * 10);
#endif
            g_pwr_key_pressed = false;

            /// Enter standby mode
            if (press_cnt >= (1 * 1000 / 10 + 1))
            {
#if defined(LOG_DBG)
                printf("enter stop mode\n");
#endif
                key_event_notify(CLOSEECU);

                if (!is_stop_enter) {
                    vout_close();
                }
                HAL_GPIO_WritePin(GPIOA, en_485_adc_Pin, GPIO_PIN_RESET);
//                HAL_Delay(600);
                power_down();
                is_stop_enter = true;
                g_keep_alive = 500;
            }
            else
            {
                //!< do nothing
//                HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_SET);
//                HAL_Delay(100);
//                HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_RESET);
//                printf("pwr_key press\n");
            }
        }
    }

    while (g_mod_sel_pressed)
    {
        HAL_Delay(10);
        if (HAL_GPIO_ReadPin(MODE_SEL_GPIO_Port, MODE_SEL_Pin) == GPIO_PIN_RESET)
        {
            press_cnt++;
        }
        else
        {
#if defined(LOG_DBG)
            printf("key up, press_time = %d\r\n", press_cnt * 10);
#endif
            g_mod_sel_pressed = false;

            /// Enter standby mode
            if (press_cnt >= (1 * 1000 / 10 + 1))
            {
                key_event_notify(CLOSEICU);
//              printf("enter stop mode\n");
//              power_down();
            }
            else
            {
                g_keep_alive = 5000;
                key_event_notify(OPENICU);
                //!< do nothing
//              HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_SET);
//              HAL_Delay(100);
//              HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_RESET);
            }
        }
    }
}

void led_on(LED led_entity)
{
    switch (led_entity)
    {
        case LED_G:
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
            break;
        case LED_Y:
            HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_SET);
            break;
        case LED_R:
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
            break;
        case LED_A:
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}

void led_off(LED led_entity)
{
    switch (led_entity)
    {
        case LED_G:
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            break;
        case LED_Y:
            HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_RESET);
            break;
        case LED_R:
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
            break;
        case LED_A:
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_Y_GPIO_Port, LED_Y_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

void beep_on(void)
{
    HAL_GPIO_WritePin(Buzzer_CTR_GPIO_Port, Buzzer_CTR_Pin, GPIO_PIN_SET);
}

void beep_off(void)
{
    HAL_GPIO_WritePin(Buzzer_CTR_GPIO_Port, Buzzer_CTR_Pin, GPIO_PIN_RESET);
}

void vout_close(void)
{
    HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_RESET);
}