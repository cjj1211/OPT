/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include "hmi.h"
#include "protocol.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_buf[1] = {0};

uint32_t adc_values[3];

volatile bool is_stop_enter = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_ADC_Init();
    /* USER CODE BEGIN 2 */
    HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOA, en_485_adc_Pin, GPIO_PIN_SET);
    HAL_Delay(200);
    HAL_UART_Receive_IT(&huart2, rx_buf, sizeof(rx_buf));
    handlers_init();

    HAL_Delay(100);
#if defined(LOG_DBG)
    printf("ECU bank init...\n");
    all_clock_freq_print();
    reset_status_get();
#else
    key_event_notify(OPENECU);
#endif
//  HAL_GPIO_WritePin(GPIOB, LED_Y_Pin, GPIO_PIN_SET);
    led_on(LED_Y);
    g_keep_alive = 2000;
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        if (g_rx_recv_flag)
        {
            g_rx_recv_flag = false;
            g_frame_timeout= 0;
#if defined(LOG_DBG)
            printf("recv frame, cmd = 0x%x\n", g_rx_buf.sdu.pdu.type);
#endif
            protocol_data_parse(&g_rx_buf);
        }

        wakeup_key_listen();

        if (g_adc_tim == 0)
        {
            g_adc_tim = 1000;
            HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);
            for (int i = 0; i < 3; i++)
            {
                HAL_ADC_Start(&hadc);
                if (HAL_ADC_PollForConversion(&hadc, 10) == HAL_OK)
                {
                    adc_values[i] = HAL_ADC_GetValue(&hadc);
                }
            }
#if defined(LOG_DBG)
            printf("%d mV, %d mV, %d mV\n", adc_values[0] * 3300 * 5 / 4096, adc_values[1] * 3300 / 4096, adc_values[2] *3300 * 2 / 4096);
#endif
        }

        if (g_keep_alive == 0)
        {
            g_keep_alive = 5000;
            HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_SET);
            HAL_Delay(200);
            HAL_GPIO_WritePin(GPIOA, STM_KEY_Pin, GPIO_PIN_RESET);
#if defined(LOG_DBG)
            printf("reset\n");
#endif
        }
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                                            |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
/**
 * @brief  Get reset status
 * @retval reset_status_t
 */
reset_status_t reset_status_get (void)
{
    uint32_t reg = READ_REG(RCC->CSR);
    reset_status_t rst_st = RST_UNK;

    if (READ_BIT(RCC->CSR, RCC_CSR_SFTRSTF) != RESET)
    {
#if defined(LOG_DBG)
        printf("Software reset\r\n");
#endif
        rst_st = RST_SFT;
    }
    else if (READ_BIT(RCC->CSR, RCC_CSR_PORRSTF) != RESET)
    {
#if defined(LOG_DBG)
        printf("POR/PDR reset\r\n");
#endif
        rst_st = RST_POR;
    }
    else if (READ_BIT(reg, RCC_CSR_PINRSTF) != RESET)
    {
#if defined(LOG_DBG)
        printf("PIN reset\r\n");
#endif
        rst_st = RST_PIN;
    }
    else
    {
        Error_Handler();
    }
    //!< Clear reset flags
    __HAL_RCC_CLEAR_RESET_FLAGS();

    return rst_st;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
         ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
