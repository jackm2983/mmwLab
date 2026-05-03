/**
  ******************************************************************************
  * @file           : mot_ctrl.c
  * @brief          : Motor move commands (high-level control)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "bsp_gpio.h"
#include "mot_axis.h"
#include "mot_stepper.h"

/* ============================================================================
 * Movement Command Functions
 * ============================================================================ */

uint8_t mot_ctrl_move_axis1_rel(int32_t steps, uint32_t freq_hz)
{
    if (steps == 0)
        return 1;

    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED)
        return 2;

    int32_t target_pos = mot_axis1_get_position() + steps;

    if (target_pos < 0 || target_pos > (360 / 5) * STEPS_PER_DEGREE)
        return 3;

    uint8_t direction = (steps > 0) ? 1 : 0;
    uint32_t abs_steps = (steps < 0) ? -steps : steps;

    mot_axis1_set_moving(1);
    mot_stepper1_move(abs_steps, direction, freq_hz);

    return 0;
}

uint8_t mot_ctrl_move_axis1_abs(int32_t target_pos, uint32_t freq_hz)
{
    int32_t current_pos = mot_axis1_get_position();
    int32_t steps_needed = target_pos - current_pos;
    return mot_ctrl_move_axis1_rel(steps_needed, freq_hz);
}

uint8_t mot_ctrl_move_axis2_rel(int32_t steps, uint32_t freq_hz)
{
    if (steps == 0)
        return 1;

    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED)
        return 2;

    int32_t target_pos = mot_axis2_get_position() + steps;

    if (target_pos < 0 || target_pos > (360 / 5) * STEPS_PER_DEGREE)
        return 3;

    uint8_t direction = (steps > 0) ? 1 : 0;
    uint32_t abs_steps = (steps < 0) ? -steps : steps;

    mot_axis2_set_moving(1);
    mot_stepper2_move(abs_steps, direction, freq_hz);

    return 0;
}

uint8_t mot_ctrl_move_axis2_abs(int32_t target_pos, uint32_t freq_hz)
{
    int32_t current_pos = mot_axis2_get_position();
    int32_t steps_needed = target_pos - current_pos;
    return mot_ctrl_move_axis2_rel(steps_needed, freq_hz);
}

/* ============================================================================
 * Homing Sequences
 * ============================================================================ */

void mot_ctrl_home_axis1(void)
{
    mot_axis1_set_moving(1);
    mot_stepper1_move(0xFFFFFFFF, 0, 50);
}

void mot_ctrl_home_axis2(void)
{
    mot_axis2_set_moving(1);
    mot_stepper2_move(0xFFFFFFFF, 0, 50);
}

/* ============================================================================
 * Stop Commands
 * ============================================================================ */

void mot_ctrl_stop_axis1(void)
{
    mot_stepper1_stop();
    mot_axis1_set_moving(0);
}

void mot_ctrl_stop_axis2(void)
{
    mot_stepper2_stop();
    mot_axis2_set_moving(0);
}

void mot_ctrl_stop_all(void)
{
    mot_ctrl_stop_axis1();
    mot_ctrl_stop_axis2();
}

/* ============================================================================
 * Status Update (call periodically)
 * ============================================================================ */

void mot_ctrl_update(void)
{
    mot_axis1_update_limits();
    if (mot_stepper1_is_moving())
    {
        mot_axis1_update_position(1);
    }
    else if (mot_axis1_is_moving())
    {
        mot_axis1_set_moving(0);
    }

    if (mot_axis1_limit_triggered() && mot_axis1_is_moving())
    {
        mot_ctrl_stop_axis1();
        mot_axis1_set_homed();
    }

    mot_axis2_update_limits();
    if (mot_stepper2_is_moving())
    {
        mot_axis2_update_position(1);
    }
    else if (mot_axis2_is_moving())
    {
        mot_axis2_set_moving(0);
    }

    if (mot_axis2_limit_triggered() && mot_axis2_is_moving())
    {
        mot_ctrl_stop_axis2();
        mot_axis2_set_homed();
    }
}

