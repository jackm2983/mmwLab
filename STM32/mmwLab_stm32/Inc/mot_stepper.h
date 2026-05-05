/**
  ******************************************************************************
  * @file           : mot_stepper.h
  * @brief          : Motor stepper control header
  ******************************************************************************
*/

#ifndef MOT_STEPPER_H
#define MOT_STEPPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Initialization */
void mot_stepper_init(void);

/* Motor 1 Stepper */
void mot_stepper1_move(int32_t num_steps, uint8_t direction, uint32_t freq_hz);
void mot_stepper1_stop(void);
uint32_t mot_stepper1_steps_remaining(void);
uint8_t mot_stepper1_is_moving(void);

/* Motor 2 Stepper */
void mot_stepper2_move(int32_t num_steps, uint8_t direction, uint32_t freq_hz);
void mot_stepper2_stop(void);
uint32_t mot_stepper2_steps_remaining(void);
uint8_t mot_stepper2_is_moving(void);

/* Acceleration Helper */
uint32_t mot_stepper_get_ramp_frequency(uint32_t start_freq, uint32_t target_freq, 
                                         uint32_t step_num, uint32_t total_steps);

#ifdef __cplusplus
}
#endif

#endif /* MOT_STEPPER_H */
