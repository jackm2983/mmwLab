/**
  ******************************************************************************
  * @file           : bsp_timer.h
  * @brief          : Board support timer header
  ******************************************************************************
*/

#ifndef BSP_TIMER_H
#define BSP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Motor 1 Control (TIM2/TIM3 initialized by main.c) */
void bsp_timer_mot1_set_frequency(uint32_t freq_hz);
void bsp_timer_mot1_start(uint32_t num_steps);
void bsp_timer_mot1_stop(void);
uint32_t bsp_timer_mot1_steps_done(void);
uint8_t bsp_timer_mot1_is_moving(void);

/* Motor 2 Control */
void bsp_timer_mot2_set_frequency(uint32_t freq_hz);
void bsp_timer_mot2_start(uint32_t num_steps);
void bsp_timer_mot2_stop(void);
uint32_t bsp_timer_mot2_steps_done(void);
uint8_t bsp_timer_mot2_is_moving(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_TIMER_H */
