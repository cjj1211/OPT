/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  *
  *  版权所有（C）2022-2025，上海脑虎科技有限公司，保留所有权利。
  *  作者 : 崔俊杰 junjie.cui@neuroxess.com
  *  描述 : 主函数
  *  修改记录:
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "flash.h"
#include "iwdg.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
#include "crc16.h"
#include "protocol.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
#define WDT_TIMEOUT (1000)

/* Private define ------------------------------------------------------------*/
volatile uint8_t g_bUserReset __attribute__((section(".bss.ARM.__at_0x20001C00")));
volatile uint8_t g_run_mode_entry __attribute__((section(".bss.ARM.__at_0x20001C04")));
volatile uint8_t g_stop_mode_enter __attribute__((section(".bss.ARM.__at_0x20001C08")));

volatile uint16_t g_wdt_timeout = WDT_TIMEOUT;

fw_info_t btr_info = {
    .x = 1,
    .y = 1,
    .z = 0,
    .b = 1,
};

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
uint32_t LL_GetTick(void);
void SystemClock_Config(void);
void LPTIM_Config(void);
void LPTIM_Delay(uint32_t ms);

/* Private user code ---------------------------------------------------------*/
typedef void (*PTR_APP_FUN)(void);

#define CPU_VTO_MASK  0x3FFFFE00
void CPUVtoSet(uint32_t newVtoVal)
{
    if ((newVtoVal & CPU_VTO_MASK) == newVtoVal)
    {
        SCB->VTOR = newVtoVal;
    }
}

void CPU_SwitchToApp(void)
{
    PTR_APP_FUN jumpToAppFun;
    uint32_t jumpAddress;

    if (((*(__IO uint32_t*)APP_LOWER) & 0x2FFE0000) == 0x20000000)
    {
        __disable_irq();
        CPUVtoSet(APP_LOWER);
        jumpAddress  = *(volatile uint32_t *)(APP_LOWER + 4);
        jumpToAppFun = (PTR_APP_FUN)jumpAddress;

        /* Initialize user application's Stack Pointer */
        __set_MSP(*(__IO uint32_t*) APP_LOWER);
        jumpToAppFun();
    }

    //!< this line would never be run
    return;
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    //!< Jump to App
    uint32_t tickupdate = 0;
    uint32_t tickcurrent = 0;
    g_run_mode_entry = RUN_MODE_BTR;

    if (g_bUserReset != true)
    {
        if (crc16((void*)APP_LOWER, APP_SIZE) == *((uint16_t*)BTR_DATA))
        {
            CPU_SwitchToApp();
        }
    }

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* SysTick_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, 3);

    /* Configure the system clock */
    SystemClock_Config();
    LL_SYSTICK_EnableIT();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    LL_mDelay(1000);
//    printf("hello, i'm LL BTR, [%d.%d.%d]\n", btr_info.x, btr_info.y, btr_info.z);
    MX_IWDG_Init();

    /* Infinite loop */
    while (1)
    {
        if (g_wdt_timeout == 0)
        {
            LL_IWDG_Refresh();
            g_wdt_timeout = WDT_TIMEOUT;
        }

        if (g_rx_recv_flag)
        {
            protocol_data_parse(&g_rx_buf);
            g_rx_recv_flag = false;
            g_frame_timeout = 0;
            
            tickupdate = LL_GetTick();
        }

        tickcurrent = LL_GetTick();
        if (tickcurrent - tickupdate > 30 * 1000)
        {
            if (crc16((void*)APP_LOWER, APP_SIZE) == *((uint16_t*)BTR_DATA))
            {
                CPU_SwitchToApp();
            }
            tickupdate = LL_GetTick();
        }
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
    while (LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
    {
    }
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    while (LL_PWR_IsActiveFlag_VOS() != 0)
    {
    }
    LL_RCC_HSI_Enable();

    /* Wait till HSI is ready */
    while (LL_RCC_HSI_IsReady() != 1)
    {
    }
    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_LSI_Enable();

    /* Wait till LSI is ready */
    while (LL_RCC_LSI_IsReady() != 1)
    {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
    {
    }

    LL_Init1msTick(16000000);

    LL_SetSystemCoreClock(16000000);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
}

void LPTIM_Config(void) 
{
    LL_RCC_LSI_Enable();
    while (LL_RCC_LSI_IsReady() == 0);

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPTIM1);
    LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM1_CLKSOURCE_LSI);

    LL_LPTIM_SetPrescaler(LPTIM1, LL_LPTIM_PRESCALER_DIV1);
    LL_LPTIM_SetCounterMode(LPTIM1, LL_LPTIM_COUNTER_MODE_INTERNAL);
    LL_LPTIM_Enable(LPTIM1);
}

void LPTIM_Delay(uint32_t ms)
{
    LL_LPTIM_SetAutoReload(LPTIM1, ms * 33);  // LSI=32.768kHz ~ 32.768 ticks/ms
    LL_LPTIM_StartCounter(LPTIM1, LL_LPTIM_OPERATING_MODE_ONESHOT);
    while (LL_LPTIM_IsActiveFlag_ARRM(LPTIM1) == 0);
    LL_LPTIM_ClearFLAG_ARRM(LPTIM1);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
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
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
