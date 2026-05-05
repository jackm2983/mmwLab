/**
  ******************************************************************************
  * @file           : bsp_flash.h
  * @brief          : Board support flash memory header
  ******************************************************************************
*/

#ifndef BSP_FLASH_H
#define BSP_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Initialization */
void bsp_flash_init(void);

/* Read Operations */
void bsp_flash_read(uint32_t start_addr, uint32_t *buffer, uint16_t word_count);
uint32_t bsp_flash_read_word(uint32_t addr);
void bsp_flash_read_bytes(uint32_t start_addr, uint8_t *buffer, uint16_t byte_count);

/* Write Operations */
uint32_t bsp_flash_write(uint32_t start_addr, uint32_t *data, uint16_t word_count);
uint32_t bsp_flash_write_bytes(uint32_t start_addr, uint8_t *data, uint16_t byte_count);

/* Erase Operations */
uint32_t bsp_flash_erase_page(uint32_t page_addr);
uint32_t bsp_flash_erase_pages(uint32_t start_addr, uint16_t page_count);

/* Storage Area Info */
uint32_t bsp_flash_get_user_area_start(void);
uint32_t bsp_flash_get_user_area_size(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_FLASH_H */
