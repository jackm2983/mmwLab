/**
  ******************************************************************************
  * @file           : cfg_pins.h
  * @brief          : Pin assignments for Nucleo L4A6ZG
  ******************************************************************************
*/

#ifndef CFG_PINS_H
#define CFG_PINS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ADC Pins - I and Q readings
 * ============================================================================ */
#define ADC_I_PIN           GPIO_PIN_3      // PA3
#define ADC_I_PORT          GPIOA
#define ADC_Q_PIN           GPIO_PIN_0      // PC0
#define ADC_Q_PORT          GPIOC

/* ============================================================================
 * Stepper Motor Controller 1 (DM556) - D70, D71, D72
 * ============================================================================ */
#define MOT1_PUL_PIN        GPIO_PIN_2      // PF2 (D70)
#define MOT1_PUL_PORT       GPIOF
#define MOT1_DIR_PIN        GPIO_PIN_6      // PB6 (D71)
#define MOT1_DIR_PORT       GPIOB
#define MOT1_ENA_PIN        GPIO_PIN_2      // PB2 (D72)
#define MOT1_ENA_PORT       GPIOB

/* ============================================================================
 * Stepper Motor Controller 2 (DM556) - D65, D66, D67
 * ============================================================================ */
#define MOT2_PUL_PIN        GPIO_PIN_0      // PG0 (D65)
#define MOT2_PUL_PORT       GPIOG
#define MOT2_DIR_PIN        GPIO_PIN_1      // PD1 (D66)
#define MOT2_DIR_PORT       GPIOD
#define MOT2_ENA_PIN        GPIO_PIN_0      // PD0 (D67)
#define MOT2_ENA_PORT       GPIOD

/* ============================================================================
 * Limit Sensors
 * ============================================================================ */
#define LIMIT1_PIN          GPIO_PIN_3      // PF3 (D49)
#define LIMIT1_PORT         GPIOF
#define LIMIT2_PIN          GPIO_PIN_5      // PF5 (D50)
#define LIMIT2_PORT         GPIOF

/* ============================================================================
 * UART Communication (USB via LPUART1)
 * ============================================================================ */
#define UART_TX_PIN         GPIO_PIN_7      // PG7 (D4)
#define UART_TX_PORT        GPIOG
#define UART_RX_PIN         GPIO_PIN_8      // PG8 (D5)
#define UART_RX_PORT        GPIOG

#ifdef __cplusplus
}
#endif

#endif
