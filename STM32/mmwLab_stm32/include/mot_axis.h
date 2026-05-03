/**
  ******************************************************************************
  * @file           : mot_axis.h
  * @brief          : Motor axis state header
  ******************************************************************************
*/

#ifndef MOT_AXIS_H
#define MOT_AXIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

/* Initialization */
void mot_axis_init(void);

/* Axis 1 Control */
int32_t mot_axis1_get_position(void);
void mot_axis1_set_position(int32_t pos);
void mot_axis1_update_position(int32_t delta);
void mot_axis1_set_homed(void);
uint8_t mot_axis1_is_homed(void);
void mot_axis1_set_moving(uint8_t moving);
uint8_t mot_axis1_is_moving(void);
void mot_axis1_update_limits(void);
uint8_t mot_axis1_limit_triggered(void);
void mot_axis1_set_bounds(int32_t min, int32_t max);

/* Axis 2 Control */
int32_t mot_axis2_get_position(void);
void mot_axis2_set_position(int32_t pos);
void mot_axis2_update_position(int32_t delta);
void mot_axis2_set_homed(void);
uint8_t mot_axis2_is_homed(void);
void mot_axis2_set_moving(uint8_t moving);
uint8_t mot_axis2_is_moving(void);
void mot_axis2_update_limits(void);
uint8_t mot_axis2_limit_triggered(void);
void mot_axis2_set_bounds(int32_t min, int32_t max);

#ifdef __cplusplus
}
#endif

#endif /* MOT_AXIS_H */
