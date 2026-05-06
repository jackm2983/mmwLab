  /**
  ******************************************************************************
  * @file           : bsp_gpio.c
  * @brief          : Board support for GPIO access
  ******************************************************************************
*/

#include "main.h"
#include "cfg_pins.h"
#include "bsp_gpio.h"
#include "stm32l4xx_hal.h"

/* ============================================================================
 * GPIO Initialization (clocks enabled by MX_GPIO_Init, pins configured here)
 * ============================================================================ */

void bsp_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ====== ADC Input Pins (Analog) ====== */
    GPIO_InitStruct.Pin = ADC_I_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC_I_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_Q_PIN;
    HAL_GPIO_Init(ADC_Q_PORT, &GPIO_InitStruct);

    /* ====== Motor 1 Control Pins (Output) ====== */
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = MOT1_PUL_PIN;
    HAL_GPIO_Init(MOT1_PUL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MOT1_DIR_PIN;
    HAL_GPIO_Init(MOT1_DIR_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MOT1_ENA_PIN;
    HAL_GPIO_Init(MOT1_ENA_PORT, &GPIO_InitStruct);

    /* ====== Motor 2 Control Pins (Output) ====== */
    GPIO_InitStruct.Pin = MOT2_PUL_PIN;
    HAL_GPIO_Init(MOT2_PUL_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MOT2_DIR_PIN;
    HAL_GPIO_Init(MOT2_DIR_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MOT2_ENA_PIN;
    HAL_GPIO_Init(MOT2_ENA_PORT, &GPIO_InitStruct);

    /* ====== Limit Switch Pins (Input with Pull-up) ====== */
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = LIMIT1_PIN;
    HAL_GPIO_Init(LIMIT1_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LIMIT2_PIN;
    HAL_GPIO_Init(LIMIT2_PORT, &GPIO_InitStruct);

    /* ====== UART Pins (Alternate Function) ====== */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;

    GPIO_InitStruct.Pin = UART_TX_PIN;
    HAL_GPIO_Init(UART_TX_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = UART_RX_PIN;
    HAL_GPIO_Init(UART_RX_PORT, &GPIO_InitStruct);

    /* Initialize all motor control pins to safe state (disabled) */
    bsp_gpio_motor_disable_all();
}

/* ============================================================================
 * Motor Control Functions
 * ============================================================================ */

void bsp_gpio_motor1_pulse(void)
{
    HAL_GPIO_WritePin(MOT1_PUL_PORT, MOT1_PUL_PIN, GPIO_PIN_SET);
    /* DM566 requires 2.5 us minimum; use a short busy loop instead of HAL_Delay */
    /* At 80 MHz SYSCLK, 20 NOPs ≈ 2.5 us */
    for (volatile int i = 0; i < 20; i++) __NOP();
    HAL_GPIO_WritePin(MOT1_PUL_PORT, MOT1_PUL_PIN, GPIO_PIN_RESET);
}

void bsp_gpio_motor1_direction(GPIO_PinState direction)
{
    HAL_GPIO_WritePin(MOT1_DIR_PORT, MOT1_DIR_PIN, direction);
}

void bsp_gpio_motor1_enable(GPIO_PinState state)
{
    HAL_GPIO_WritePin(MOT1_ENA_PORT, MOT1_ENA_PIN, state);
}

void bsp_gpio_motor2_pulse(void)
{
    HAL_GPIO_WritePin(MOT2_PUL_PORT, MOT2_PUL_PIN, GPIO_PIN_SET);
    /* DM566 requires 2.5 us minimum; use a short busy loop instead of HAL_Delay */
    for (volatile int i = 0; i < 20; i++) __NOP();
    HAL_GPIO_WritePin(MOT2_PUL_PORT, MOT2_PUL_PIN, GPIO_PIN_RESET);
}

void bsp_gpio_motor2_direction(GPIO_PinState direction)
{
    HAL_GPIO_WritePin(MOT2_DIR_PORT, MOT2_DIR_PIN, direction);
}

void bsp_gpio_motor2_enable(GPIO_PinState state)
{
    HAL_GPIO_WritePin(MOT2_ENA_PORT, MOT2_ENA_PIN, state);
}

void bsp_gpio_motor_disable_all(void)
{
    HAL_GPIO_WritePin(MOT1_ENA_PORT, MOT1_ENA_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOT2_ENA_PORT, MOT2_ENA_PIN, GPIO_PIN_RESET);
}

void bsp_gpio_motor_enable_all(void)
{
    HAL_GPIO_WritePin(MOT1_ENA_PORT, MOT1_ENA_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(MOT2_ENA_PORT, MOT2_ENA_PIN, GPIO_PIN_SET);
}

/* ============================================================================
 * Limit Switch Functions
 * ============================================================================ */

uint8_t bsp_gpio_limit1_read(void)
{
    return HAL_GPIO_ReadPin(LIMIT1_PORT, LIMIT1_PIN) == GPIO_PIN_RESET;
}

uint8_t bsp_gpio_limit2_read(void)
{
    return HAL_GPIO_ReadPin(LIMIT2_PORT, LIMIT2_PIN) == GPIO_PIN_RESET;
}

uint8_t bsp_gpio_limit_triggered(void)
{
    return (bsp_gpio_limit1_read() || bsp_gpio_limit2_read());
}
