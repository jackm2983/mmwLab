/**
  ******************************************************************************
  * @file           : drv_store.c
  * @brief          : Driver for flash data storage (calibration data)
  ******************************************************************************
*/

#include "main.h"
#include "bsp_flash.h"
#include "bsp_uart.h"

#define DRV_STORE_DEBUG 0

static uint8_t drv_store_load(void);

/* ============================================================================
 * Calibration Data Structure (24 bytes, multiple of doubleword)
 * ============================================================================ */

typedef struct {
    uint32_t magic;
    int32_t  axis1_min;
    int32_t  axis1_max;
    int32_t  axis2_min;
    int32_t  axis2_max;
    uint32_t padding;
} CalibrationData_t;

#define CALIB_MAGIC 0xCAFEBABE
static CalibrationData_t calib_data = {0};

#if DRV_STORE_DEBUG
static void dbg_i32(const char *label, int32_t v)
{
    bsp_uart_send_string(label);
    if (v < 0) { bsp_uart_send_char('-'); v = -v; }
    char buf[12]; int i = 0;
    uint32_t u = (uint32_t)v;
    if (u == 0) buf[i++] = '0';
    else {
        char tmp[12]; int j = 0;
        while (u > 0) { tmp[j++] = '0' + (u % 10); u /= 10; }
        while (j > 0) buf[i++] = tmp[--j];
    }
    buf[i] = 0;
    bsp_uart_send_string(buf);
}
#endif

/* ============================================================================
 * Initialization
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

    bsp_flash_read(flash_addr, (uint32_t *)&calib_data, sizeof(CalibrationData_t) / 4);

    if (calib_data.magic != CALIB_MAGIC) {
#if DRV_STORE_DEBUG
        bsp_uart_send_string("DBG store: no valid calib, using defaults\r\n");
#endif
        calib_data.magic = CALIB_MAGIC;
        calib_data.axis1_min = 0;
        calib_data.axis1_max = 10000;
        calib_data.axis2_min = 0;
        calib_data.axis2_max = 10000;
        calib_data.padding = 0;
        return 1;
    }

#if DRV_STORE_DEBUG
    bsp_uart_send_string("DBG store: loaded calib ");
    dbg_i32("a1=[", calib_data.axis1_min);
    dbg_i32(",", calib_data.axis1_max);
    dbg_i32("] a2=[", calib_data.axis2_min);
    dbg_i32(",", calib_data.axis2_max);
    bsp_uart_send_string("]\r\n");
#endif

    return 0;
}

/* ============================================================================
 * Save Calibration to Flash
 * ============================================================================ */

uint8_t drv_store_save(void)
{
    uint32_t flash_addr = bsp_flash_get_user_area_start();
    calib_data.magic = CALIB_MAGIC;
    calib_data.padding = 0;

#if DRV_STORE_DEBUG
    bsp_uart_send_string("DBG store: saving ");
    dbg_i32("a1=[", calib_data.axis1_min);
    dbg_i32(",", calib_data.axis1_max);
    dbg_i32("] a2=[", calib_data.axis2_min);
    dbg_i32(",", calib_data.axis2_max);
    bsp_uart_send_string("] to 0x");
    // address printed as hex
    char hex[9]; const char *h = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) { hex[7-i] = h[(flash_addr >> (i*4)) & 0xF]; }
    hex[8] = 0;
    bsp_uart_send_string(hex);
    bsp_uart_send_string("\r\n");
#endif

    uint32_t status = bsp_flash_write(flash_addr, (uint32_t *)&calib_data,
                                      sizeof(CalibrationData_t) / 4);

#if DRV_STORE_DEBUG
    bsp_uart_send_string("DBG store: write rc=");
    dbg_i32("", (int32_t)status);
    bsp_uart_send_string("\r\n");
#endif

    return status;
}

/* ============================================================================
 * Calibration Data Access
 * ============================================================================ */

void drv_store_set_axis1_bounds(int32_t min, int32_t max)
{
    // ensure min <= max regardless of input order
    if (min > max) { int32_t t = min; min = max; max = t; }
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
    if (min > max) { int32_t t = min; min = max; max = t; }
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
    calib_data.magic = 0;
    calib_data.axis1_min = 0;
    calib_data.axis1_max = 10000;
    calib_data.axis2_min = 0;
    calib_data.axis2_max = 10000;
    calib_data.padding = 0;
}