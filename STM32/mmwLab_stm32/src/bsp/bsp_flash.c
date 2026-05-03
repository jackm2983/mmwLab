  /**
  ******************************************************************************
  * @file           : bsp_flash.c
  * @brief          : Board support for flash memory access (data storage)
  ******************************************************************************
*/

#include "main.h"
#include "stm32l4xx_hal.h"

/* ============================================================================
 * Flash Memory Layout (STM32L476)
 * ============================================================================
 * Page size: 2KB
 * Total: 1MB flash
 * Using last pages for data storage
 */

#define FLASH_USER_START_ADDR   FLASH_BANK1_END - (4 * FLASH_PAGE_SIZE)  /* Last 4 pages */
#define FLASH_PAGE_SIZE_BYTES   2048

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * Get the page number from a flash address
 */
static uint32_t bsp_flash_get_page(uint32_t addr)
{
    return (addr - FLASH_BASE) / FLASH_PAGE_SIZE_BYTES;
}

/**
 * Get the page address from a page number
 */
static uint32_t bsp_flash_get_page_addr(uint32_t page)
{
    return FLASH_BASE + (page * FLASH_PAGE_SIZE_BYTES);
}

/* ============================================================================
 * Flash Initialization
 * ============================================================================ */

void bsp_flash_init(void)
{
    /* Flash is ready to use - no special init needed for read/write */
}

/* ============================================================================
 * Flash Read Operations
 * ============================================================================ */

/**
 * Read words from flash memory
 * @param start_addr: Starting flash address
 * @param buffer: Output buffer for data
 * @param word_count: Number of 32-bit words to read
 */
void bsp_flash_read(uint32_t start_addr, uint32_t *buffer, uint16_t word_count)
{
    if (!buffer || word_count == 0)
        return;

    for (uint16_t i = 0; i < word_count; i++)
    {
        buffer[i] = *(__IO uint32_t *)(start_addr + (i * 4));
    }
}

/**
 * Read a single 32-bit word from flash
 */
uint32_t bsp_flash_read_word(uint32_t addr)
{
    return *(__IO uint32_t *)addr;
}

/**
 * Read bytes from flash (more flexible)
 */
void bsp_flash_read_bytes(uint32_t start_addr, uint8_t *buffer, uint16_t byte_count)
{
    if (!buffer || byte_count == 0)
        return;

    for (uint16_t i = 0; i < byte_count; i++)
    {
        buffer[i] = *(__IO uint8_t *)(start_addr + i);
    }
}

/* ============================================================================
 * Flash Write Operations
 * ============================================================================ */

/**
 * Write words to flash memory
 * @param start_addr: Starting flash address (must be page-aligned for erase)
 * @param data: Data buffer to write
 * @param word_count: Number of 32-bit words to write
 * @return: 0 on success, error code on failure
 */
uint32_t bsp_flash_write(uint32_t start_addr, uint32_t *data, uint16_t word_count)
{
    if (!data || word_count == 0)
        return 1;

    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t page_error = 0;
    uint32_t status;

    /* Unlock flash */
    HAL_FLASH_Unlock();

    /* Calculate page range */
    uint32_t start_page = bsp_flash_get_page(start_addr);
    uint32_t end_addr = start_addr + (word_count * 4);
    uint32_t end_page = bsp_flash_get_page(end_addr);
    uint32_t page_count = (end_page - start_page) + 1;

    /* Erase pages */
    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_config.Page = start_page;
    erase_config.NbPages = page_count;

    status = HAL_FLASHEx_Erase(&erase_config, &page_error);
    if (status != HAL_OK)
    {
        HAL_FLASH_Lock();
        return HAL_FLASH_GetError();
    }

    /* Program words */
    uint32_t write_addr = start_addr;
    for (uint16_t i = 0; i < word_count; i++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, write_addr, (uint64_t)data[i]);
        if (status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return HAL_FLASH_GetError();
        }
        write_addr += 4;
    }

    /* Verify written data */
    for (uint16_t i = 0; i < word_count; i++)
    {
        uint32_t verify_addr = start_addr + (i * 4);
        if (*(__IO uint32_t *)verify_addr != data[i])
        {
            HAL_FLASH_Lock();
            return 2;  /* Verification failed */
        }
    }

    /* Lock flash */
    HAL_FLASH_Lock();

    return 0;  /* Success */
}

/**
 * Write bytes to flash (wrapper for byte-level writes)
 */
uint32_t bsp_flash_write_bytes(uint32_t start_addr, uint8_t *data, uint16_t byte_count)
{
    if (!data || byte_count == 0)
        return 1;

    /* Pad data to word boundary */
    uint32_t word_count = (byte_count + 3) / 4;
    uint32_t *word_buffer = (uint32_t *)data;

    return bsp_flash_write(start_addr, word_buffer, word_count);
}

/* ============================================================================
 * Flash Erase Operations
 * ============================================================================ */

/**
 * Erase a page of flash memory
 * @param page_addr: Address of the page to erase (should be page-aligned)
 * @return: 0 on success, error code on failure
 */
uint32_t bsp_flash_erase_page(uint32_t page_addr)
{
    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_config.Page = bsp_flash_get_page(page_addr);
    erase_config.NbPages = 1;

    uint32_t status = HAL_FLASHEx_Erase(&erase_config, &page_error);

    HAL_FLASH_Lock();

    return (status == HAL_OK) ? 0 : HAL_FLASH_GetError();
}

/**
 * Erase multiple pages
 */
uint32_t bsp_flash_erase_pages(uint32_t start_addr, uint16_t page_count)
{
    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_config.Page = bsp_flash_get_page(start_addr);
    erase_config.NbPages = page_count;

    uint32_t status = HAL_FLASHEx_Erase(&erase_config, &page_error);

    HAL_FLASH_Lock();

    return (status == HAL_OK) ? 0 : HAL_FLASH_GetError();
}

/* ============================================================================
 * Flash Storage API (user data)
 * ============================================================================ */

/**
 * Get the user data storage area start address
 */
uint32_t bsp_flash_get_user_area_start(void)
{
    return FLASH_USER_START_ADDR;
}

/**
 * Get available storage space (in bytes)
 */
uint32_t bsp_flash_get_user_area_size(void)
{
    return 4 * FLASH_PAGE_SIZE_BYTES;
}