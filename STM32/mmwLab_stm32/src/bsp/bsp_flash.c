/**
  ******************************************************************************
  * @file           : bsp_flash.c
  * @brief          : Board support for flash memory access
  ******************************************************************************
*/

#include "main.h"
#include "bsp_uart.h"
#include "stm32l4xx_hal.h"

#define BSP_FLASH_DEBUG 1

#define FLASH_PAGE_SIZE_BYTES   2048
#define FLASH_USER_START_ADDR   ((FLASH_BANK1_END + 1) - (4 * FLASH_PAGE_SIZE_BYTES))

#if BSP_FLASH_DEBUG
static void dbg_hex32(const char *label, uint32_t v)
{
    bsp_uart_send_string(label);
    char hex[9]; const char *h = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) { hex[7-i] = h[(v >> (i*4)) & 0xF]; }
    hex[8] = 0;
    bsp_uart_send_string(hex);
}

static void dbg_u32(const char *label, uint32_t v)
{
    bsp_uart_send_string(label);
    char buf[12]; int i = 0;
    if (v == 0) buf[i++] = '0';
    else {
        char tmp[12]; int j = 0;
        while (v > 0) { tmp[j++] = '0' + (v % 10); v /= 10; }
        while (j > 0) buf[i++] = tmp[--j];
    }
    buf[i] = 0;
    bsp_uart_send_string(buf);
}
#endif

/* ============================================================================
 * Helpers
 * ============================================================================ */

static uint32_t bsp_flash_get_page(uint32_t addr)
{
    return (addr - FLASH_BASE) / FLASH_PAGE_SIZE_BYTES;
}

void bsp_flash_init(void)
{
    // nothing required
}

/* ============================================================================
 * Read
 * ============================================================================ */

void bsp_flash_read(uint32_t start_addr, uint32_t *buffer, uint16_t word_count)
{
    if (!buffer || word_count == 0) return;
    for (uint16_t i = 0; i < word_count; i++) {
        buffer[i] = *(__IO uint32_t *)(start_addr + (i * 4));
    }
}

uint32_t bsp_flash_read_word(uint32_t addr)
{
    return *(__IO uint32_t *)addr;
}

void bsp_flash_read_bytes(uint32_t start_addr, uint8_t *buffer, uint16_t byte_count)
{
    if (!buffer || byte_count == 0) return;
    for (uint16_t i = 0; i < byte_count; i++) {
        buffer[i] = *(__IO uint8_t *)(start_addr + i);
    }
}

/* ============================================================================
 * Write (doubleword aligned, stm32l4 requirement)
 * ============================================================================ */

uint32_t bsp_flash_write(uint32_t start_addr, uint32_t *data, uint16_t word_count)
{
    if (!data || word_count == 0) return 1;

    // stm32l4 requires doubleword (8 byte) alignment for programming
    if (start_addr & 0x7) {
#if BSP_FLASH_DEBUG
        bsp_uart_send_string("DBG flash: addr not 8-byte aligned ");
        dbg_hex32("addr=0x", start_addr);
        bsp_uart_send_string("\r\n");
#endif
        return 4;
    }

    uint16_t dword_count = (word_count + 1) / 2;

#if BSP_FLASH_DEBUG
    bsp_uart_send_string("DBG flash: write ");
    dbg_u32("words=", word_count);
    dbg_u32(" dwords=", dword_count);
    dbg_hex32(" addr=0x", start_addr);
    bsp_uart_send_string("\r\n");
#endif

    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t page_error = 0;
    uint32_t status;

    HAL_FLASH_Unlock();

    uint32_t start_page = bsp_flash_get_page(start_addr);
    uint32_t end_addr = start_addr + (dword_count * 8) - 1;
    uint32_t end_page = bsp_flash_get_page(end_addr);
    uint32_t page_count = (end_page - start_page) + 1;

    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_config.Banks = FLASH_BANK_1;
    erase_config.Page = start_page;
    erase_config.NbPages = page_count;

    status = HAL_FLASHEx_Erase(&erase_config, &page_error);
    if (status != HAL_OK) {
#if BSP_FLASH_DEBUG
        bsp_uart_send_string("DBG flash: erase FAILED ");
        dbg_u32("hal_err=", HAL_FLASH_GetError());
        bsp_uart_send_string("\r\n");
#endif
        HAL_FLASH_Lock();
        return HAL_FLASH_GetError();
    }

    // program doublewords
    uint32_t write_addr = start_addr;
    for (uint16_t i = 0; i < dword_count; i++) {
        uint32_t lo = data[i * 2];
        uint32_t hi = (i * 2 + 1 < word_count) ? data[i * 2 + 1] : 0xFFFFFFFF;
        uint64_t dword = ((uint64_t)hi << 32) | (uint64_t)lo;

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, write_addr, dword);
        if (status != HAL_OK) {
#if BSP_FLASH_DEBUG
            bsp_uart_send_string("DBG flash: program FAILED ");
            dbg_u32("dword=", i);
            dbg_u32(" hal_err=", HAL_FLASH_GetError());
            bsp_uart_send_string("\r\n");
#endif
            HAL_FLASH_Lock();
            return HAL_FLASH_GetError();
        }
        write_addr += 8;
    }

    // verify
    for (uint16_t i = 0; i < word_count; i++) {
        uint32_t verify_addr = start_addr + (i * 4);
        uint32_t got = *(__IO uint32_t *)verify_addr;
        if (got != data[i]) {
#if BSP_FLASH_DEBUG
            bsp_uart_send_string("DBG flash: verify FAILED ");
            dbg_u32("word=", i);
            dbg_hex32(" got=0x", got);
            dbg_hex32(" want=0x", data[i]);
            bsp_uart_send_string("\r\n");
#endif
            HAL_FLASH_Lock();
            return 2;
        }
    }

    HAL_FLASH_Lock();

#if BSP_FLASH_DEBUG
    bsp_uart_send_string("DBG flash: write OK\r\n");
#endif

    return 0;
}

uint32_t bsp_flash_write_bytes(uint32_t start_addr, uint8_t *data, uint16_t byte_count)
{
    if (!data || byte_count == 0) return 1;
    uint32_t word_count = (byte_count + 3) / 4;
    uint32_t *word_buffer = (uint32_t *)data;
    return bsp_flash_write(start_addr, word_buffer, word_count);
}

/* ============================================================================
 * Erase
 * ============================================================================ */

uint32_t bsp_flash_erase_page(uint32_t page_addr)
{
    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_config.Banks = FLASH_BANK_1;
    erase_config.Page = bsp_flash_get_page(page_addr);
    erase_config.NbPages = 1;

    uint32_t status = HAL_FLASHEx_Erase(&erase_config, &page_error);

    HAL_FLASH_Lock();
    return (status == HAL_OK) ? 0 : HAL_FLASH_GetError();
}

uint32_t bsp_flash_erase_pages(uint32_t start_addr, uint16_t page_count)
{
    FLASH_EraseInitTypeDef erase_config = {0};
    uint32_t page_error = 0;

    HAL_FLASH_Unlock();

    erase_config.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_config.Banks = FLASH_BANK_1;
    erase_config.Page = bsp_flash_get_page(start_addr);
    erase_config.NbPages = page_count;

    uint32_t status = HAL_FLASHEx_Erase(&erase_config, &page_error);

    HAL_FLASH_Lock();
    return (status == HAL_OK) ? 0 : HAL_FLASH_GetError();
}

/* ============================================================================
 * User Area
 * ============================================================================ */

uint32_t bsp_flash_get_user_area_start(void)
{
    return FLASH_USER_START_ADDR;
}

uint32_t bsp_flash_get_user_area_size(void)
{
    return 4 * FLASH_PAGE_SIZE_BYTES;
}