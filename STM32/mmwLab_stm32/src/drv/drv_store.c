/**
  ******************************************************************************
  * @file           : drv_store.c
  * @brief          : Driver for flash data storage (calibration data)
  ******************************************************************************
*/

#include "main.h"
#include "bsp_flash.h"

/* ============================================================================
 * Calibration Data Structure
 * ============================================================================ */

typedef struct {
    uint32_t magic;         /* Validation magic */
    int32_t axis1_min;      /* Axis 1 minimum position */
    int32_t axis1_max;      /* Axis 1 maximum position */
    int32_t axis2_min;      /* Axis 2 minimum position */
    int32_t axis2_max;      /* Axis 2 maximum position */
} CalibrationData_t;

#define CALIB_MAGIC 0xCAFEBABE
static CalibrationData_t calib_data = {0};

/* ============================================================================
 * Storage Initialization
 * ============================================================================ */

void drv_store_init(void)
{
    bsp_flash_init();
    drv_store_load();
}

/* ============================================================================
 * Load Calibration from Flash
 * ============================================================================ */

uint8_t drv_store_load(void)
{
    uint32_t flash_addr = bsp_flash_get_user_area_start();
    
    /* Read calibration data */
    bsp_flash_read(flash_addr, (uint32_t *)&calib_data, sizeof(CalibrationData_t) / 4);

    /* Validate */
    if (calib_data.magic != CALIB_MAGIC)
    {
        /* Invalid or no data - use defaults */
        calib_data.magic = CALIB_MAGIC;
        calib_data.axis1_min = 0;
        calib_data.axis1_max = 10000;
        calib_data.axis2_min = 0;
        calib_data.axis2_max = 10000;
        return 1;  /* No valid calibration found */
    }

    return 0;  /* Successfully loaded */
}

/* ============================================================================
 * Save Calibration to Flash
 * ============================================================================ */

uint8_t drv_store_save(void)
{
    uint32_t flash_addr = bsp_flash_get_user_area_start();

    /* Ensure magic is set */
    calib_data.magic = CALIB_MAGIC;

    /* Write to flash */
    uint32_t status = bsp_flash_write(flash_addr, (uint32_t *)&calib_data, 
                                      sizeof(CalibrationData_t) / 4);

    return status;
}

/* ============================================================================
 * Calibration Data Access
 * ============================================================================ */

void drv_store_set_axis1_bounds(int32_t min, int32_t max)
{
    calib_data.axis1_min = min;
    calib_data.axis1_max = max;
}

void drv_store_get_axis1_bounds(int32_t *min, int32_t *max)
{
    *min = calib_data.axis1_min;
    *max = calib_data.axis1_max;
}

void drv_store_set_axis2_bounds(int32_t min, int32_t max)
{
    calib_data.axis2_min = min;
    calib_data.axis2_max = max;
}

void drv_store_get_axis2_bounds(int32_t *min, int32_t *max)
{
    *min = calib_data.axis2_min;
    *max = calib_data.axis2_max;
}

/* ============================================================================
 * Clear and Reset
 * ============================================================================ */

void drv_store_clear(void)
{
    calib_data.magic = 0;  /* Invalidate */
    calib_data.axis1_min = 0;
    calib_data.axis1_max = 10000;
    calib_data.axis2_min = 0;
    calib_data.axis2_max = 10000;
}
