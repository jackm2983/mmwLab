/**
  ******************************************************************************
  * @file           : mot_ctrl.h
  * @brief          : Motor control header
  ******************************************************************************
*/

#ifndef MOT_CTRL_H
#define MOT_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Relative Movement */
uint8_t mot_ctrl_move_axis1_rel(int32_t steps, uint32_t freq_hz);
uint8_t mot_ctrl_move_axis2_rel(int32_t steps, uint32_t freq_hz);

/* Absolute Movement */
uint8_t mot_ctrl_move_axis1_abs(int32_t target_pos, uint32_t freq_hz);
uint8_t mot_ctrl_move_axis2_abs(int32_t target_pos, uint32_t freq_hz);

/* Homing */
void mot_ctrl_home_axis1(void);
void mot_ctrl_home_axis2(void);

/* Stop */
void mot_ctrl_stop_axis1(void);
void mot_ctrl_stop_axis2(void);
void mot_ctrl_stop_all(void);

/* Update (call from main loop) */
void mot_ctrl_update(void);

uint8_t mot_ctrl_jog_axis1(uint8_t direction, uint32_t freq_hz);
uint8_t mot_ctrl_jog_axis2(uint8_t direction, uint32_t freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* MOT_CTRL_H */
