/**
  ******************************************************************************
  * @file           : bsp_adc.c
  * @brief          : Board support for ADC (I/Q signal acquisition)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "cfg_pins.h"
#include "stm32l4xx_hal.h"

/* ============================================================================
 * ADC Handle & Buffers
 * ============================================================================ */

static ADC_HandleTypeDef hadc1;

/* DMA buffers for continuous ADC conversion */
static uint32_t adc_dma_buffer[2];  /* [0] = I channel (PA3), [1] = Q channel (PC0) */
static uint16_t adc_samples_i[SAMPLE_SIZE];
static uint16_t adc_samples_q[SAMPLE_SIZE];
static volatile uint16_t adc_sample_count = 0;
static volatile uint8_t adc_conversion_complete = 0;

/* ============================================================================
 * ADC Initialization
 * ============================================================================ */

void bsp_adc_init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* ADC1 configuration */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = 2;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.OversamplingMode = DISABLE;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure ADC channel for I signal (PA3 = ADC1_IN8) */
    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure ADC channel for Q signal (PC0 = ADC1_IN1) */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Calibrate ADC */
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }
}

/* ============================================================================
 * ADC Conversion Functions
 * ============================================================================ */

void bsp_adc_start_continuous(void)
{
    adc_sample_count = 0;
    adc_conversion_complete = 0;

    /* Start ADC with DMA */
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buffer, 2) != HAL_OK)
    {
        Error_Handler();
    }
}

void bsp_adc_stop(void)
{
    HAL_ADC_Stop_DMA(&hadc1);
}

/* Single conversion read (blocking) */
void bsp_adc_read_single(uint16_t *i_value, uint16_t *q_value)
{
    if (HAL_ADC_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT) != HAL_OK)
    {
        Error_Handler();
    }

    *i_value = HAL_ADC_GetValue(&hadc1);

    if (HAL_ADC_PollForConversion(&hadc1, ADC_TIMEOUT) != HAL_OK)
    {
        Error_Handler();
    }

    *q_value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);
}

/* Store sample in buffers */
static void bsp_adc_store_sample(void)
{
    if (adc_sample_count < SAMPLE_SIZE)
    {
        adc_samples_i[adc_sample_count] = (uint16_t)(adc_dma_buffer[0] & 0xFFF);
        adc_samples_q[adc_sample_count] = (uint16_t)(adc_dma_buffer[1] & 0xFFF);
        adc_sample_count++;

        if (adc_sample_count >= SAMPLE_SIZE)
        {
            adc_conversion_complete = 1;
        }
    }
}

/* Check if sampling is complete */
uint8_t bsp_adc_sampling_complete(void)
{
    return adc_conversion_complete;
}

/* Reset sampling state */
void bsp_adc_sampling_reset(void)
{
    adc_sample_count = 0;
    adc_conversion_complete = 0;
}

/* Get sampled data */
void bsp_adc_get_samples(uint16_t **i_samples, uint16_t **q_samples, uint16_t *count)
{
    *i_samples = adc_samples_i;
    *q_samples = adc_samples_q;
    *count = adc_sample_count;
}

/* ============================================================================
 * DMA & Interrupt Callbacks
 * ============================================================================ */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        bsp_adc_store_sample();
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        /* Error occurred - can restart or handle gracefully */
        bsp_adc_sampling_reset();
    }
}

