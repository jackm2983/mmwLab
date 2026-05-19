/**
  ******************************************************************************
  * @file           : mot_ctrl.c
  * @brief          : Motor move commands (high-level control)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "bsp_gpio.h"
#include "bsp_uart.h"
#include "mot_axis.h"
#include "mot_stepper.h"

// set to 1 to enable verbose debug output from this module
#define MOT_CTRL_DEBUG  1

#if MOT_CTRL_DEBUG
static void dbg_print_u32(const char *label, uint32_t v)
{
    bsp_uart_send_string(label);
    char buf[12];
    int i = 0;
    if (v == 0) { buf[i++] = '0'; }
    else {
        char tmp[12];
        int j = 0;
        while (v > 0) { tmp[j++] = '0' + (v % 10); v /= 10; }
        while (j > 0) { buf[i++] = tmp[--j]; }
    }
    buf[i] = 0;
    bsp_uart_send_string(buf);
    bsp_uart_send_string("\r\n");
}

static void dbg_print_i32(const char *label, int32_t v)
{
    bsp_uart_send_string(label);
    if (v < 0) { bsp_uart_send_char('-'); v = -v; }
    dbg_print_u32("", (uint32_t)v);
}
#endif

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
    {
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG mot_ctrl: axis1 rel rejected, bounds\r\n");
        dbg_print_i32("  cur=", mot_axis1_get_position());
        dbg_print_i32("  steps=", steps);
        dbg_print_i32("  target=", target_pos);
        dbg_print_i32("  max=", (360 / 5) * STEPS_PER_DEGREE);
#endif
        return 3;
    }

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
    {
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG mot_ctrl: axis2 rel rejected, bounds\r\n");
#endif
        return 3;
    }

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
 * Jog (continuous) Movement
 *   no position bounds check, limit switch is the safety
 * ============================================================================ */

uint8_t mot_ctrl_jog_axis1(uint8_t direction, uint32_t freq_hz)
{
#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG mot_ctrl: jog_axis1 ");
    bsp_uart_send_string(direction ? "FWD " : "REV ");
    dbg_print_u32("freq=", freq_hz);
#endif

    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED)
    {
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG   rejected: bad freq\r\n");
        dbg_print_u32("  MOTOR_MAX_SPEED=", MOTOR_MAX_SPEED);
#endif
        return 2;
    }

    if (direction && mot_axis1_limit_triggered())
    {
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG   rejected: limit triggered\r\n");
#endif
        return 3;
    }

    mot_axis1_set_moving(1);
    mot_stepper1_move(0x7FFFFFFF, direction, freq_hz);

#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG   stepper started\r\n");
#endif
    return 0;
}

uint8_t mot_ctrl_jog_axis2(uint8_t direction, uint32_t freq_hz)
{
#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG mot_ctrl: jog_axis2 ");
    bsp_uart_send_string(direction ? "FWD " : "REV ");
    dbg_print_u32("freq=", freq_hz);
#endif

    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED)
    {
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG   rejected: bad freq\r\n");
#endif
        return 2;
    }

    if (direction && mot_axis2_limit_triggered())
    {
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG   rejected: limit triggered\r\n");
#endif
        return 3;
    }

    mot_axis2_set_moving(1);
    mot_stepper2_move(0x7FFFFFFF, direction, freq_hz);

#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG   stepper started\r\n");
#endif
    return 0;
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