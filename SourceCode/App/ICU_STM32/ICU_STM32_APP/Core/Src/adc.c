/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  *
  *  版权所有（C）2022-2025，上海脑虎科技有限公司，保留所有权利。
  *  作者 : 崔俊杰 junjie.cui@neuroxess.com
  *  描述 : ADC
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

#define ADC_TIMEOUT 10000  // 10,000 cycles timeout

/* ADC init function */
void MX_ADC_Init(void)
{
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct = {0};
    LL_ADC_InitTypeDef ADC_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
    /**ADC GPIO Configuration
    PA4   ------> ADC_IN4
    PA2   ------> ADC_IN2
    PA3   ------> ADC_IN3
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /** Configure Regular Channel */
    LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_2);

    /** Configure Regular Channel */
    LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_3);

    /** Configure Regular Channel */
    LL_ADC_REG_SetSequencerChAdd(ADC1, LL_ADC_CHANNEL_4);

    /** Common config */
    ADC_REG_InitStruct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_1RANK;
    ADC_REG_InitStruct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
    ADC_REG_InitStruct.Overrun = LL_ADC_REG_OVR_DATA_PRESERVED;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);
    LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_1CYCLE_5);
    LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_DISABLE);
    LL_ADC_REG_SetSequencerScanDirection(ADC1, LL_ADC_REG_SEQ_SCAN_DIR_FORWARD);
    LL_ADC_SetCommonFrequencyMode(__LL_ADC_COMMON_INSTANCE(ADC1), LL_ADC_CLOCK_FREQ_MODE_HIGH);
    LL_ADC_DisableIT_EOC(ADC1);
    LL_ADC_DisableIT_EOS(ADC1);
    ADC_InitStruct.Clock = LL_ADC_CLOCK_SYNC_PCLK_DIV1;
    ADC_InitStruct.Resolution = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    ADC_InitStruct.LowPowerMode = LL_ADC_LP_MODE_NONE;
    LL_ADC_Init(ADC1, &ADC_InitStruct);

    LL_ADC_StartCalibration(ADC1);
    while (LL_ADC_IsCalibrationOnGoing(ADC1));
    LL_ADC_Enable(ADC1);
}

void MX_ADC_DeInit(void)
{
    if (LL_ADC_IsEnabled(ADC1) && LL_ADC_REG_IsConversionOngoing(ADC1))
    {
        LL_ADC_REG_StopConversion(ADC1);
    }

    LL_ADC_Disable(ADC1);
    LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_ADC1);
}

void ADC_Read_MultiChannel(uint16_t* results)
{
    uint32_t timeout = 0;

    for (int i = 0; i < 3; i++)
    {
        LL_ADC_REG_StartConversion(ADC1);
        while(!LL_ADC_IsActiveFlag_EOC(ADC1))
        {
            if (++timeout > ADC_TIMEOUT)
            {
                LL_ADC_ClearFlag_EOC(ADC1);
                results[i] = 0xffff;
                timeout = 0;
            }
        }
        if (results[i] != 0xffff)
        {
            results[i] = LL_ADC_REG_ReadConversionData12(ADC1);
        }
    }
}
