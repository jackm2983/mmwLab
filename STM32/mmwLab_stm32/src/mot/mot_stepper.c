/**
  ******************************************************************************
  * @file           : mot_stepper.c
  * @brief          : Motor pulse generation (low-level stepper control)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"

/* ============================================================================
 * Stepper Initialization
 * ============================================================================ */

void mot_stepper_init(void)
{
    bsp_gpio_motor_disable_all();
}

/* ============================================================================
 * Motor 1 Stepper Control
 * ============================================================================ */

void mot_stepper1_move(int32_t num_steps, uint8_t direction, uint32_t freq_hz)
{
    if (num_steps == 0) return;

    bsp_gpio_motor1_direction(direction ? GPIO_PIN_SET : GPIO_PIN_RESET);
    bsp_gpio_motor1_enable(GPIO_PIN_SET);
    bsp_timer_mot1_set_frequency(freq_hz);
    bsp_timer_mot1_start((uint32_t)num_steps, direction ? +1 : -1);
}

void mot_stepper1_stop(void)
{
    bsp_timer_mot1_stop();
    bsp_gpio_motor1_enable(GPIO_PIN_RESET);
}

uint32_t mot_stepper1_steps_remaining(void)
{
    return bsp_timer_mot1_steps_done();
}

uint8_t mot_stepper1_is_moving(void)
{
    return bsp_timer_mot1_is_moving();
}

/* ============================================================================
 * Motor 2 Stepper Control
 * ============================================================================ */

void mot_stepper2_move(int32_t num_steps, uint8_t direction, uint32_t freq_hz)
{
    if (num_steps == 0) return;

    bsp_gpio_motor2_direction(direction ? GPIO_PIN_SET : GPIO_PIN_RESET);
    bsp_gpio_motor2_enable(GPIO_PIN_SET);
    bsp_timer_mot2_set_frequency(freq_hz);
    bsp_timer_mot2_start((uint32_t)num_steps, direction ? +1 : -1);
}

void mot_stepper2_stop(void)
{
    bsp_timer_mot2_stop();
    bsp_gpio_motor2_enable(GPIO_PIN_RESET);
}

uint32_t mot_stepper2_steps_remaining(void)
{
    return bsp_timer_mot2_steps_done();
}

uint8_t mot_stepper2_is_moving(void)
{
    return bsp_timer_mot2_is_moving();
}

/* ============================================================================
 * Acceleration Ramp Helper
 * ============================================================================ */

uint32_t mot_stepper_get_ramp_frequency(uint32_t start_freq, uint32_t target_freq,
                                         uint32_t step_num, uint32_t total_steps)
{
    if (total_steps == 0) return target_freq;
    uint32_t freq_delta = target_freq - start_freq;
    return start_freq + (freq_delta * step_num) / total_steps;
}