/**
  ******************************************************************************
  * @file           : drv_store.h
  * @brief          : Driver flash storage header
  ******************************************************************************
*/

#ifndef DRV_STORE_H
#define DRV_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

void drv_store_init(void);
uint8_t drv_store_load(void);
uint8_t drv_store_save(void);

void drv_store_set_axis1_bounds(int32_t min, int32_t max);
void drv_store_get_axis1_bounds(int32_t *min, int32_t *max);

void drv_store_set_axis2_bounds(int32_t min, int32_t max);
void drv_store_get_axis2_bounds(int32_t *min, int32_t *max);

void drv_store_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* DRV_STORE_H */
