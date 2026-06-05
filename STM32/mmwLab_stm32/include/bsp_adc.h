/**
  ******************************************************************************
  * @file           : bsp_adc.h
  * @brief          : Board support ADC header
  ******************************************************************************
*/

#ifndef BSP_ADC_H
#define BSP_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Initialization */
void bsp_adc_init(void);

/* Continuous conversion (with DMA) */
void bsp_adc_start_continuous(void);
void bsp_adc_stop(void);

/* Single conversion (blocking) */
void bsp_adc_read_single(uint16_t *i_value, uint16_t *q_value);
void bsp_adc_read_peak_1khz(uint16_t *i_peak, uint16_t *q_peak);

/* Sampling state */
uint8_t bsp_adc_sampling_complete(void);
void bsp_adc_sampling_reset(void);

/* Get sampled data */
void bsp_adc_get_samples(uint16_t **i_samples, uint16_t **q_samples, uint16_t *count);

/* 40 kHz Bandpass filtering */
void bsp_adc_filter_samples(void);
void bsp_adc_get_filtered_samples(float **i_samples, float **q_samples, uint16_t *count);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ADC_H */
