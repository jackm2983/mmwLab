/**
  ******************************************************************************
  * @file           : bsp_filter.c
  * @brief          : Digital signal filtering (IIR bandpass filter for 40kHz)
  ******************************************************************************
*/

#include "bsp_filter.h"
#include <math.h>

/* ============================================================================
 * Filter Coefficients (2nd-order Butterworth Bandpass @ 40 kHz)
 * ============================================================================
 * 
 * Designed for:
 *   - Center frequency: 40 kHz
 *   - Bandwidth: ~20 kHz (30-50 kHz passband)
 *   - Sample rate: ~1 MHz (conservative estimate)
 * 
 * Using direct form II IIR structure:
 *   y(n) = b0*w(n) + b1*w(n-1) + b2*w(n-2)
 *   w(n) = x(n) - a1*w(n-1) - a2*w(n-2)
 * ============================================================================ */

#define FILTER_B0    0.0248f    /* Numerator coefficients */
#define FILTER_B1    0.0000f
#define FILTER_B2   -0.0248f
#define FILTER_A1   -1.8270f    /* Denominator coefficients (negated for feedback) */
#define FILTER_A2    0.9410f
#define FILTER_GAIN  1.0f

/* ============================================================================
 * Filter Initialization
 * ============================================================================ */

void bsp_filter_init_40khz(BandpassFilterState *filter)
{
    if (filter == NULL)
        return;

    filter->z1 = 0.0f;
    filter->z2 = 0.0f;
    filter->gain = FILTER_GAIN;
}

/* ============================================================================
 * Filter Application (Single Sample)
 * ============================================================================ */

float bsp_filter_apply(BandpassFilterState *filter, float sample)
{
    if (filter == NULL)
        return sample;

    /* Direct form II IIR structure */
    float w = sample - (FILTER_A1 * filter->z1) - (FILTER_A2 * filter->z2);
    float output = (FILTER_B0 * w) + (FILTER_B1 * filter->z1) + (FILTER_B2 * filter->z2);

    /* Update state */
    filter->z2 = filter->z1;
    filter->z1 = w;

    return output;
}

/* ============================================================================
 * Filter Application (Batch)
 * ============================================================================ */

void bsp_filter_apply_batch(BandpassFilterState *filter, float *samples, uint16_t count)
{
    if (filter == NULL || samples == NULL || count == 0)
        return;

    for (uint16_t i = 0; i < count; i++)
    {
        samples[i] = bsp_filter_apply(filter, samples[i]);
    }
}
