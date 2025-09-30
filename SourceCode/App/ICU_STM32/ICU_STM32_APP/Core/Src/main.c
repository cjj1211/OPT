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
#include "adc.h"
#include "i2c.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "flash.h"
#include "info.h"

/* Private includes ----------------------------------------------------------*/
#include "protocol.h"
#include "ptx30w_drv.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
volatile uint8_t g_bUserReset __attribute__((section(".bss.ARM.__at_0x20001C00")));
volatile uint8_t g_run_mode_entry __attribute__((section(".bss.ARM.__at_0x20001C04")));
volatile uint8_t g_start_mode __attribute__((section(".bss.ARM.__at_0x20001C08")));
volatile uint8_t g_wakeup_source __attribute__((section(".bss.ARM.__at_0x20001C0C")));
volatile uint8_t g_chrg_triggered __attribute__((section(".bss.ARM.__at_0x20001C10")));

/* Private macro -------------------------------------------------------------*/
#define ADC_TIMEOUT (1000)
#define PTX_TIMEOUT (1000)
#define WDT_TIMEOUT (100)

#define DETUNE_DELAY (5)
#define UPLOAD_DELAY (30)
#define EMER_TRY_MAX (3)

/* Private variables ---------------------------------------------------------*/
volatile uint32_t g_stop_timeout = STOP_TIMEOUT;
volatile uint16_t g_wdt_timeout = WDT_TIMEOUT;
volatile uint16_t g_adc_timeout = ADC_TIMEOUT;
volatile uint16_t g_ptx_timeout = PTX_TIMEOUT;
volatile uint16_t g_chg_timeout = 0;
volatile uint16_t g_exit_timeout = 0;
volatile uint8_t g_chrg_flag = 0;
volatile uint8_t g_overtemp_triggered = 0;
volatile uint8_t IRQ_flag = 0;

uint16_t voltage_mV;
uint16_t ptx_vddc_mV;
int16_t current_ma;
uint8_t ble_status = 0;
float temperature;      // temperature of the Titanium shell

static uint8_t low_volt_count = 0;
static uint8_t chrg_count = 0;
static uint8_t hot_count = 0;
static uint8_t lot_count = 0;
static uint8_t exit_count = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void LPTIM_Config(void);
void LPTIM_Delay(uint32_t ms);

/* Private user code ---------------------------------------------------------*/
#if defined(LOG_DEBUG)
void printf_hex(uint8_t *ptr, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        printf("%.2x ", ptr[i]);
    }
    printf("\n");
}
#endif

void Pre_Enter_Stop_Mode(void)
{
    g_start_mode = START_PRE_STOP;
    NVIC_SystemReset();
}

void Enter_STOP_Mode(void)
{
//    printf("Enter STOP Mode\n");
    MX_TIM2_DeInit();
    MX_TIM21_DeInit();
    MX_USART2_UART_DeInit();
    MX_I2C1_DeInit();
    MX_ADC_DeInit();
    MX_GPIO_Stop();

    // Close SysTick
    SysTick->CTRL = 0;
    SysTick->VAL = 0;

    // Config Low-Power Mode
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP);
    LL_PWR_SetRegulModeLP(LL_PWR_REGU_LPMODES_LOW_POWER);

    // Enter STOP Mode
    LL_LPM_EnableDeepSleep();
    __WFI();

    // Wakeup
    LPTIM_Config();
    LPTIM_Delay(50);
    if (g_overtemp_triggered)
    {
        g_start_mode = START_POWERUP;
        NVIC_SystemReset();
    }
    else if (LL_GPIO_IsInputPinSet(PTX_GPIO1_GPIO_Port, PTX_GPIO1_Pin) == 0)
    {
        bool reset_flag = false;
        for (int move_time = 0; move_time < 100 * 4; move_time++)
        {
            if (LL_GPIO_IsInputPinSet(PTX_GPIO1_GPIO_Port, PTX_GPIO1_Pin) == 1)
            {
                reset_flag = true;
                break;
            }
            LPTIM_Delay(10);
        }
        if (reset_flag)
        {
            g_wakeup_source = CHG_TRIG_SRC;
            g_start_mode = START_WKUP;
            g_chrg_triggered = 0;
            NVIC_SystemReset();
        }
        else
        {
            g_chrg_triggered = 1;
            Pre_Enter_Stop_Mode();
        }
    }
    else
    {
        Pre_Enter_Stop_Mode();
    }
}

void Safe_Boot(void)
{
    struct emer_event evt1;
    struct emer_event evt2;

    STM32_EEPROM_Read(EMER_EVT_1ST_ADDR, (uint8_t*)&evt1, sizeof(struct emer_event));
    STM32_EEPROM_Read(EMER_EVT_2ND_ADDR, (uint8_t*)&evt2, sizeof(struct emer_event));

    if ((evt1.exist == EVENT_MAGIC) && (evt2.exist == EVENT_MAGIC)) {
#if defined(RECOVERY_ENABLED)
        uint16_t magic_num = *((__IO uint16_t *)RECOVERY_MAGIC_ADDR);
        if(++magic_num >= 5) {
            g_emer_event.exist = 0;
            printf("Clear EMER, %d\n", magic_num);
            STM32_EEPROM_Write(EMER_EVT_1ST_ADDR, (uint8_t*)&g_emer_event, sizeof(g_emer_event));
            STM32_EEPROM_Write(EMER_EVT_2ND_ADDR, (uint8_t*)&g_emer_event, sizeof(g_emer_event));
            magic_num = 0;
        }
        STM32_EEPROM_Write(RECOVERY_MAGIC_ADDR, (uint8_t*)&magic_num, sizeof(magic_num));
#endif
        delay_ms(2000);
//        printf("EMER EXIST, Boot failed (0x%02x)\n", evt1.type);
        delay_ms(1000);
        Enter_STOP_Mode();
    }
}

static void notify_chrg_status(uint8_t status)
{
    uint8_t data[2] = {0};
    data[0] = EVT_CHRGSTAT;
    data[1] = status;
    sdu_send(FRAME_EVT_NOTICE, data, sizeof(data));
//    printf("Chrg Status: %d\n", status);
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    __enable_irq();
    g_run_mode_entry = RUN_MODE_APP;
    g_bUserReset = false;

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* SysTick_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, 3);

    /* Configure the system clock */
    SystemClock_Config();
    NX_SysTick_Config(SystemCoreClock / 1000);

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_ADC_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    MX_TIM2_Init();

    if (g_start_mode == START_WKUP)
    {
        Safe_Boot();
        MX_IWDG_Init();
        system_limit_init();
        GPIO_Config(1);
    }
    else if (g_start_mode == START_PRE_STOP)
    {
        Enter_STOP_Mode();
    }
    else
    {
        g_start_mode = START_POWERUP;
        Safe_Boot();
        MX_IWDG_Init();
        system_limit_init();
        GPIO_Config(0);
    }

    /* temperature sensor config */
    TMP119_Config();

#if 0
    int ret = ptx30w_driver_init();
//    printf("ptx30w_driver init = %d\n", ret);
#endif

//    printf("TRIG_SRC: 0x%02x, %d\n", g_wakeup_source, g_chrg_triggered);
    if (g_wakeup_source != HOT_TRIG_SRC && g_wakeup_source != LOT_TRIG_SRC)
    {
        TMP119_SetHighLimit(sys_limits.temp_soft_limit);
        TMP119_SetLowLimit(LOW_LIMIT_RESET);
        g_wakeup_source = HOT_TRIG_SRC;
//        ptx30w_set_rt(ParamType_DetuneEnable, 0);
    }

    delay_ms(500);
//    printf("hello, i'm LL APP, [%d.%d.%d] %d\n", app_info.x, app_info.y, app_info.z, g_stop_mode_enter);

//    printf("Sys limits: %d, %d, %d, %d\n",
//                      sys_limits.discharging_curr_limit,
//                      sys_limits.temp_soft_limit,
//                      sys_limits.temp_hard_limit,
//                      sys_limits.undervoltage_limit);


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
            g_rx_recv_flag = false;
            g_frame_timeout = 0;
            protocol_data_parse(&g_rx_buf);
        }

        /** If the pin remains in a low-level state for 8 seconds continuously, it is determined that 
         *  the device has entered the charging state. After 5 mins, it will automatically enter the 
         *  sleep mode.
         */
        if (g_chrg_flag && (g_chg_timeout == 0))
        {
            g_chg_timeout = 100;
            if (LL_GPIO_IsInputPinSet(PTX_GPIO1_GPIO_Port, PTX_GPIO1_Pin) == 0)
            {
                chrg_count++;
            }
            else
            {
                g_chrg_flag = 0;
                chrg_count = 0;
                g_chrg_triggered = 0;
            }
//            printf("chrg: %d\n", chrg_count);
            
            if (chrg_count >= 10 * 8)
            {
//                printf("Mock enter\n");
                g_chrg_flag = 0;
                chrg_count = 0;
                g_chrg_triggered = 1;
                g_stop_timeout = STOP_TIMEOUT - 1;

                notify_chrg_status(g_chrg_triggered);
            }
        }

        if (g_stop_timeout == 0)
        {
//            printf("Pre_stop\n");
            Pre_Enter_Stop_Mode();
        }

        //!< Determine whether to exit the charging state
        if (g_chrg_triggered && (g_exit_timeout == 0))
        {
            g_exit_timeout = 1000;
            if (LL_GPIO_IsInputPinSet(PTX_GPIO1_GPIO_Port, PTX_GPIO1_Pin) == 1)
            {
                exit_count++;
            }
            else
            {
                exit_count = 0;
            }

            if (exit_count >= 8) {
                exit_count = 0;
                g_chrg_triggered = 0;
                g_stop_timeout = STOP_TIMEOUT;

                notify_chrg_status(g_chrg_triggered);
            }
        }

#if defined(NFC_ENABLED)
        if (g_ptx_timeout == 0)
        {
            ptx30w_update_status();
            g_ptx_timeout = PTX_TIMEOUT;
        }

        if (IRQ_flag)
        {
            IRQ_flag = 0;
            ptx30w_TDC_transfer();
        }
#endif

        if (g_adc_timeout == 0)
        {
            uint16_t adc_values[3] = {0};
            ADC_Read_MultiChannel(adc_values);
            voltage_mV = (adc_values[0] * 3300 * 5) / 4096;
            ptx_vddc_mV = (adc_values[1] * 3300 * 5) / 4096;
            /**
              * INA186A3
              * V_out = V_ref + (I_shunt * R_shunt * Gain), where V_ref = 1.2V, Gain = 100
              * Measured reference voltage: 1195mV
              * The current consumed by STM32 itself: 3.5mA
              */
            current_ma = ((adc_values[2] * 3300) / 4096 - 1195) / 5.0 - 3.5;
            TMP119_ReadTemperature(&temperature);
            g_adc_timeout = ADC_TIMEOUT;

            if (current_ma < 0 && (-current_ma >= sys_limits.discharging_curr_limit))
            {
                int try = 0;
                for (int i = 0; i < EMER_TRY_MAX; i++)
                {
                    delay_ms(100);
                    ADC_Read_MultiChannel(adc_values);
                    current_ma = ((adc_values[2] * 3300) / 4096 - 1195) / 5.0 - 3.5;
                    if (current_ma < 0 && (-current_ma >= sys_limits.discharging_curr_limit)) {
                        try++;
                    }
                    else
                    {
                        break;
                    }
                }

                if (try >= EMER_TRY_MAX) {
                    g_emer_event.exist = EVENT_MAGIC;
                    g_emer_event.type = EMER_EVT_OVERCURR;
                    g_emer_event.value = current_ma;
//                    printf("EMER_EVT_OVERCURR, %d\n", g_emer_event.value);
                    STM32_EEPROM_Write(EMER_EVT_1ST_ADDR, (uint8_t*)&g_emer_event, sizeof(g_emer_event));
                    STM32_EEPROM_Write(EMER_EVT_2ND_ADDR, (uint8_t*)&g_emer_event, sizeof(g_emer_event));
#if defined(RECOVERY_ENABLED)
                    uint16_t value = 0;
                    STM32_EEPROM_Write(RECOVERY_MAGIC_ADDR, (uint8_t*)&value, sizeof(value));
#endif
                    delay_ms(10);
                    Enter_STOP_Mode();
                }
            }

            if ((int16_t)(temperature * 10) >= sys_limits.temp_hard_limit)
            {
                int try = 0;
                for (int i = 0; i < EMER_TRY_MAX; i++)
                {
                    delay_ms(100);
                    TMP119_ReadTemperature(&temperature);
                    if ((int16_t)(temperature * 10) < sys_limits.temp_hard_limit) {
                        break;
                    }
                    try++;
                }

                if (try >= EMER_TRY_MAX)
                {
                    g_emer_event.exist = EVENT_MAGIC;
                    g_emer_event.type = EMER_EVT_OVERTEMP;
                    g_emer_event.value = (int16_t)(temperature * 10);
//                    printf("EMER_EVT_OVERTEMP, %d\n", g_emer_event.value);
                    STM32_EEPROM_Write(EMER_EVT_1ST_ADDR, (uint8_t*)&g_emer_event, sizeof(g_emer_event));
                    STM32_EEPROM_Write(EMER_EVT_2ND_ADDR, (uint8_t*)&g_emer_event, sizeof(g_emer_event));
#if defined(RECOVERY_ENABLED)
                    uint16_t value = 0;
                    STM32_EEPROM_Write(RECOVERY_MAGIC_ADDR, (uint8_t*)&value, sizeof(value));
#endif
                    delay_ms(10);
                    Enter_STOP_Mode();
                }
            }

            if (ble_status && voltage_mV <= sys_limits.undervoltage_limit)
            {
                low_volt_count++;
            }
            else
            {
                low_volt_count = 0;
            }
            if (low_volt_count >= 10)
            {
                Pre_Enter_Stop_Mode();
            }

//            printf("voltage_mV = %d, ptx_vddc_mV = %d, current_ma = %d, temperature = %f, chrg_triggered: %d, overtemp: %d\n ",
//                       voltage_mV, ptx_vddc_mV, current_ma, temperature, g_chrg_triggered, g_overtemp_triggered);
            if (g_overtemp_triggered == 1)
            {
                uint16_t cfg_val = 0;

                //!< High/Low Alert flag cleared on read of configuration register
                TMP119_ReadReg(TMP119_REG_CFGR, &cfg_val);
//                printf("CFGR_= 0x%04x\n", cfg_val);
                if (cfg_val & 0x8000)
                {
                    hot_count++;
                    lot_count = 0;
                    if (hot_count >= DETUNE_DELAY && g_chrg_triggered) {
//                        ptx30w_set_rt(ParamType_DetuneEnable, 1);
                        TMP119_SetHighLimit(HIGH_LIMIT_RESET);
                        TMP119_SetLowLimit(sys_limits.temp_soft_limit - 10);
//                        printf("chrg off, High limit remains 5s, Enter Stop\n");
                        g_wakeup_source = LOT_TRIG_SRC;
                        Pre_Enter_Stop_Mode();
                    }
                    if (hot_count >= UPLOAD_DELAY) {
                        hot_count = 0;
                        uint8_t data[3] = {0};
                        data[0] = EVT_OVERTEMP;
                        data[1] = ((int16_t)(temperature * 10));
                        data[2] = ((int16_t)(temperature * 10)) >> 8;
                        sdu_send(FRAME_EVT_NOTICE, data, sizeof(data));
//                        printf("High limit remains 30s\n");
                    }
                }
                else if (cfg_val & 0x4000)
                {
                    lot_count++;
                    hot_count = 0;
                    if (lot_count >= DETUNE_DELAY && g_chrg_triggered) {
//                        ptx30w_set_rt(ParamType_DetuneEnable, 0);
                        TMP119_SetHighLimit(sys_limits.temp_soft_limit);
                        TMP119_SetLowLimit(LOW_LIMIT_RESET);
//                        printf("chrg on, Low limit remains 5s, Enter Stop\n");
                        g_wakeup_source = HOT_TRIG_SRC;
                        Pre_Enter_Stop_Mode();
                    }
                }
            }
            else
            {
                hot_count = 0;
                lot_count = 0;
            }
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
