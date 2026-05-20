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
#include "bsp_timer.h"
#include "mot_axis.h"
#include "mot_stepper.h"

#define MOT_CTRL_DEBUG  0

#if MOT_CTRL_DEBUG
static void dbg_u32(const char *label, uint32_t v)
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
}

static void dbg_i32(const char *label, int32_t v)
{
    bsp_uart_send_string(label);
    if (v < 0) { bsp_uart_send_char('-'); v = -v; }
    dbg_u32("", (uint32_t)v);
}
#endif

uint8_t mot_ctrl_move_axis1_rel(int32_t steps, uint32_t freq_hz)
{
    if (steps == 0) return 1;
    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED) return 2;

    uint8_t direction = (steps > 0) ? 1 : 0;
    uint32_t abs_steps = (steps < 0) ? -steps : steps;

#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG ctrl: axis1 rel ");
    dbg_i32("steps=", steps);
    dbg_u32(" freq=", freq_hz);
    dbg_i32(" from=", mot_axis1_get_position());
    bsp_uart_send_string("\r\n");
#endif

    mot_axis1_set_moving(1);
    mot_stepper1_move(abs_steps, direction, freq_hz);
    return 0;
}

uint8_t mot_ctrl_move_axis1_abs(int32_t target_pos, uint32_t freq_hz)
{
    int32_t current_pos = mot_axis1_get_position();
    int32_t steps_needed = target_pos - current_pos;

#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG ctrl: axis1 abs ");
    dbg_i32("target=", target_pos);
    dbg_i32(" cur=", current_pos);
    dbg_i32(" delta=", steps_needed);
    bsp_uart_send_string("\r\n");
#endif

    if (steps_needed == 0) return 0;
    return mot_ctrl_move_axis1_rel(steps_needed, freq_hz);
}

uint8_t mot_ctrl_move_axis2_rel(int32_t steps, uint32_t freq_hz)
{
    if (steps == 0) return 1;
    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED) return 2;

    uint8_t direction = (steps > 0) ? 1 : 0;
    uint32_t abs_steps = (steps < 0) ? -steps : steps;

#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG ctrl: axis2 rel ");
    dbg_i32("steps=", steps);
    dbg_u32(" freq=", freq_hz);
    dbg_i32(" from=", mot_axis2_get_position());
    bsp_uart_send_string("\r\n");
#endif

    mot_axis2_set_moving(1);
    mot_stepper2_move(abs_steps, direction, freq_hz);
    return 0;
}

uint8_t mot_ctrl_move_axis2_abs(int32_t target_pos, uint32_t freq_hz)
{
    int32_t current_pos = mot_axis2_get_position();
    int32_t steps_needed = target_pos - current_pos;

#if MOT_CTRL_DEBUG
    bsp_uart_send_string("DBG ctrl: axis2 abs ");
    dbg_i32("target=", target_pos);
    dbg_i32(" cur=", current_pos);
    dbg_i32(" delta=", steps_needed);
    bsp_uart_send_string("\r\n");
#endif

    if (steps_needed == 0) return 0;
    return mot_ctrl_move_axis2_rel(steps_needed, freq_hz);
}

uint8_t mot_ctrl_jog_axis1(uint8_t direction, uint32_t freq_hz)
{
    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED) return 2;

    mot_axis1_set_moving(1);
    mot_stepper1_move(0x7FFFFFFF, direction, freq_hz);
    return 0;
}

uint8_t mot_ctrl_jog_axis2(uint8_t direction, uint32_t freq_hz)
{
    if (freq_hz == 0 || freq_hz > MOTOR_MAX_SPEED) return 2;

    mot_axis2_set_moving(1);
    mot_stepper2_move(0x7FFFFFFF, direction, freq_hz);
    return 0;
}

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

void mot_ctrl_update(void)
{
    mot_axis1_update_limits();

    int32_t d1 = bsp_timer_mot1_consume_delta();
    if (d1 != 0) {
        mot_axis1_update_position(d1);
    }

    if (!mot_stepper1_is_moving() && mot_axis1_is_moving()) {
        mot_axis1_set_moving(0);
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG ctrl: axis1 motion ended ");
        dbg_i32("pos=", mot_axis1_get_position());
        bsp_uart_send_string("\r\n");
#endif
    }

    mot_axis2_update_limits();

    int32_t d2 = bsp_timer_mot2_consume_delta();
    if (d2 != 0) {
        mot_axis2_update_position(d2);
    }

    if (!mot_stepper2_is_moving() && mot_axis2_is_moving()) {
        mot_axis2_set_moving(0);
#if MOT_CTRL_DEBUG
        bsp_uart_send_string("DBG ctrl: axis2 motion ended ");
        dbg_i32("pos=", mot_axis2_get_position());
        bsp_uart_send_string("\r\n");
#endif
    }
}
