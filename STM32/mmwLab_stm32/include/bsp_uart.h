/**
  ******************************************************************************
  * @file           : bsp_uart.h
  * @brief          : Board support UART header
  ******************************************************************************
*/

#ifndef BSP_UART_H
#define BSP_UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Startup (enables RX interrupts after UART initialized by main.c) */
void bsp_uart_start(void);

/* TX Functions (LPUART1 initialized by main.c) */
void bsp_uart_send_char(uint8_t c);
void bsp_uart_send_string(const char *str);
void bsp_uart_send_buffer(const uint8_t *data, uint16_t length);

/* RX Functions */
uint8_t bsp_uart_data_available(void);
uint8_t bsp_uart_read_char(void);
uint16_t bsp_uart_read_buffer(uint8_t *data, uint16_t max_length);

#ifdef __cplusplus
}
#endif

#endif /* BSP_UART_H */
