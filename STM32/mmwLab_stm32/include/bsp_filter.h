/**
  ******************************************************************************
  * @file           : bsp_filter.h
  * @brief          : Digital signal filtering (IIR bandpass filter for 40kHz)
  ******************************************************************************
*/

#ifndef BSP_FILTER_H
#define BSP_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Filter Configuration
 * ============================================================================ */

/* Butterworth bandpass filter centered at 40 kHz with ~20 kHz bandwidth */
typedef struct {
    float z1;           /* First delay state */
    float z2;           /* Second delay state */
    float gain;         /* Filter gain */
} BandpassFilterState;

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/**
 * @brief Initialize a 40 kHz bandpass filter state
 * @param filter Pointer to filter state structure
 */
void bsp_filter_init_40khz(BandpassFilterState *filter);

/**
 * @brief Apply 40 kHz bandpass filter to a single sample
 * @param filter Pointer to filter state
 * @param sample Input sample value
 * @return Filtered output value
 */
float bsp_filter_apply(BandpassFilterState *filter, float sample);

/**
 * @brief Apply filter to an array of samples (in-place)
 * @param filter Pointer to filter state
 * @param samples Array of samples to filter
 * @param count Number of samples
 */
void bsp_filter_apply_batch(BandpassFilterState *filter, float *samples, uint16_t count);

#ifdef __cplusplus
}
#endif

#endif /* BSP_FILTER_H */
