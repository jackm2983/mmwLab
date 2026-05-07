
/**
  ******************************************************************************
  * @file           : bsp_uart.c
  * @brief          : Board support for UART communication
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "stm32l4xx_hal.h"
#include "bsp_uart.h"

/* Forward declaration */
static void bsp_uart_tx_flush(void);

/* ============================================================================
 * UART Handle & Buffers
 * ============================================================================ */

extern UART_HandleTypeDef hlpuart1;

/* Circular buffers for RX and TX */
static uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
static uint8_t uart_tx_buffer[UART_TX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;

/* ============================================================================
 * UART Initialization
 * ============================================================================ */

void bsp_uart_init(void)
{
	HAL_UART_Receive_IT(&hlpuart1, (uint8_t *)&uart_rx_buffer[rx_head], 1);
}

/* ============================================================================
 * TX Functions
 * ============================================================================ */

void bsp_uart_send_char(uint8_t c)
{
    uint16_t next_head = (tx_head + 1) % UART_TX_BUFFER_SIZE;

    /* Wait if buffer is full */
    while (next_head == tx_tail)
    {
        /* Buffer full, wait or overflow */
    }

    uart_tx_buffer[tx_head] = c;
    tx_head = next_head;

    /* Start transmission if not already running */
    if (hlpuart1.gState == HAL_UART_STATE_READY)
    {
        bsp_uart_tx_flush();
    }
}

void bsp_uart_send_string(const char *str)
{
    while (*str)
    {
        bsp_uart_send_char((uint8_t)*str++);
    }
}

void bsp_uart_send_buffer(const uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        bsp_uart_send_char(data[i]);
    }
}

static void bsp_uart_tx_flush(void)
{
    if (tx_head != tx_tail)
    {
        HAL_UART_Transmit_IT(&hlpuart1, &uart_tx_buffer[tx_tail], 1);
    }
}

/* ============================================================================
 * RX Functions
 * ============================================================================ */

uint8_t bsp_uart_data_available(void)
{
    return (rx_head != rx_tail);
}

uint8_t bsp_uart_read_char(void)
{
    uint8_t c = 0;

    if (rx_head != rx_tail)
    {
        c = uart_rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % UART_RX_BUFFER_SIZE;
    }

    return c;
}

uint16_t bsp_uart_read_buffer(uint8_t *data, uint16_t max_length)
{
    uint16_t count = 0;

    while ((count < max_length) && (rx_head != rx_tail))
    {
        data[count++] = uart_rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % UART_RX_BUFFER_SIZE;
    }

    return count;
}

/* ============================================================================
 * Interrupt Handlers (called by HAL)
 * ============================================================================ */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart_)
{
    if (huart_->Instance == LPUART1)
    {
        rx_head = (rx_head + 1) % UART_RX_BUFFER_SIZE;

        /* Re-enable RX interrupt for next character */
        HAL_UART_Receive_IT(huart_, (uint8_t *)&uart_rx_buffer[rx_head], 1);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart_)
{
    if (huart_->Instance == LPUART1)
    {
        tx_tail = (tx_tail + 1) % UART_TX_BUFFER_SIZE;

        /* Continue transmission if more data available */
        if (tx_tail != tx_head)
        {
            HAL_UART_Transmit_IT(huart_, &uart_tx_buffer[tx_tail], 1);
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart_)
{
    if (huart_->Instance == LPUART1)
    {
        /* Clear error and restart RX */
        __HAL_UART_CLEAR_FLAG(huart_, UART_CLEAR_OREF);
        __HAL_UART_CLEAR_FLAG(huart_, UART_CLEAR_NEF);
        __HAL_UART_CLEAR_FLAG(huart_, UART_CLEAR_FEF);
        __HAL_UART_CLEAR_FLAG(huart_, UART_CLEAR_PEF);

        HAL_UART_Receive_IT(huart_, (uint8_t *)&uart_rx_buffer[rx_head], 1);
    }
}


