/**
  ******************************************************************************
  * @file           : bsp_adc.c
  * @brief          : Board support for ADC (I/Q signal acquisition)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "cfg_pins.h"
#include "bsp_filter.h"
#include "stm32l4xx_hal.h"


 #define ADC_PEAK_READ_MS    20


/* ============================================================================
 * ADC Handle & Buffers
 * ============================================================================ */

static ADC_HandleTypeDef hadc1;

/* DMA buffers for continuous ADC conversion */
static uint32_t adc_dma_buffer[2];  /* [0] = I channel (PA3), [1] = Q channel (PC0) */
static uint16_t adc_samples_i[SAMPLE_SIZE];
static uint16_t adc_samples_q[SAMPLE_SIZE];
static float adc_samples_i_filtered[SAMPLE_SIZE];  /* Filtered I channel */
static float adc_samples_q_filtered[SAMPLE_SIZE];  /* Filtered Q channel */
static volatile uint16_t adc_sample_count = 0;
static volatile uint8_t adc_conversion_complete = 0;

/* Filter states for 40 kHz bandpass filtering */
static BandpassFilterState filter_i;
static BandpassFilterState filter_q;

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
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;  /* Faster sampling for 40kHz signal */
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
    sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;  /* Faster sampling for 40kHz signal */

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* Calibrate ADC */
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
    {
        Error_Handler();
    }

    /* Initialize 40 kHz bandpass filters */
    bsp_filter_init_40khz(&filter_i);
    bsp_filter_init_40khz(&filter_q);
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




/* Read ADC peaks over a time window (optimized for 40kHz signals) */
void bsp_adc_read_peak_1khz(uint16_t *i_peak, uint16_t *q_peak)
{
    uint16_t i_max = 0;
    uint16_t q_max = 0;

    uint32_t start = HAL_GetTick();

    while ((HAL_GetTick() - start) < ADC_PEAK_READ_MS)
    {
        uint16_t i_value = 0;
        uint16_t q_value = 0;

        bsp_adc_read_single(&i_value, &q_value);

        if (i_value > i_max) {
            i_max = i_value;
        }

        if (q_value > q_max) {
            q_max = q_value;
        }
    }

    *i_peak = i_max;
    *q_peak = q_max;
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

/* Apply 40 kHz bandpass filter to captured samples */
void bsp_adc_filter_samples(void)
{
    /* Convert raw 12-bit ADC values to float and apply bandpass filter */
    for (uint16_t i = 0; i < adc_sample_count; i++)
    {
        /* Normalize ADC samples (12-bit: 0-4095) to 0-1 range */
        float i_normalized = (float)adc_samples_i[i] / 4095.0f;
        float q_normalized = (float)adc_samples_q[i] / 4095.0f;

        /* Apply 40 kHz bandpass filter */
        adc_samples_i_filtered[i] = bsp_filter_apply(&filter_i, i_normalized);
        adc_samples_q_filtered[i] = bsp_filter_apply(&filter_q, q_normalized);
    }
}

/* Get filtered sample data */
void bsp_adc_get_filtered_samples(float **i_samples, float **q_samples, uint16_t *count)
{
    *i_samples = adc_samples_i_filtered;
    *q_samples = adc_samples_q_filtered;
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

