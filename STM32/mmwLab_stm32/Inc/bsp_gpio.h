/**
  ******************************************************************************
  * @file           : bsp_gpio.h
  * @brief          : Board support GPIO header
  ******************************************************************************
*/

#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Initialization */
void bsp_gpio_init(void);

/* Motor 1 Control */
void bsp_gpio_motor1_pulse(void);
void bsp_gpio_motor1_direction(GPIO_PinState direction);
void bsp_gpio_motor1_enable(GPIO_PinState state);

/* Motor 2 Control */
void bsp_gpio_motor2_pulse(void);
void bsp_gpio_motor2_direction(GPIO_PinState direction);
void bsp_gpio_motor2_enable(GPIO_PinState state);

/* Bulk Motor Control */
void bsp_gpio_motor_disable_all(void);
void bsp_gpio_motor_enable_all(void);

/* Limit Switches */
uint8_t bsp_gpio_limit1_read(void);
uint8_t bsp_gpio_limit2_read(void);
uint8_t bsp_gpio_limit_triggered(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_GPIO_H */
