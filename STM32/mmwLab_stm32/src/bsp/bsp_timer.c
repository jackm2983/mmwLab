/**
  ******************************************************************************
  * @file           : bsp_timer.c
  * @brief          : Board support for timers (step pulse generation)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "cfg_pins.h"
#include "bsp_gpio.h"
#include "stm32l4xx_hal.h"

/* ============================================================================
 * Timer Handles for Stepper Control (from main.c via CubeMX)
 * ============================================================================ */

extern TIM_HandleTypeDef htim2;  /* Motor 1 stepper timer */
extern TIM_HandleTypeDef htim3;  /* Motor 2 stepper timer */

static uint32_t mot1_step_count = 0;
static uint32_t mot2_step_count = 0;
static uint32_t mot1_target_steps = 0;
static uint32_t mot2_target_steps = 0;

/* ============================================================================
 * Motor 1 Step Control (timers initialized by MX_TIM2_Init/MX_TIM3_Init in main.c)
 * ============================================================================ */

void bsp_timer_mot1_set_frequency(uint32_t freq_hz)
{
    /* Prevent division by zero and cap to reasonable limits */
    if (freq_hz == 0)
        freq_hz = 1;
    if (freq_hz > MOTOR_MAX_SPEED)
        freq_hz = MOTOR_MAX_SPEED;

    /* Period in microseconds = 1,000,000 / freq_hz / 2 (for toggle mode) */
    uint32_t period = (1000000 / freq_hz / 2) - 1;

    htim2.Instance->ARR = period;
    htim2.Instance->PSC = 79;  /* 1MHz clock */
}

void bsp_timer_mot1_start(uint32_t num_steps)
{
    mot1_target_steps = num_steps;
    mot1_step_count = 0;
    HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
}

void bsp_timer_mot1_stop(void)
{
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    mot1_step_count = 0;
    mot1_target_steps = 0;
}

uint32_t bsp_timer_mot1_steps_done(void)
{
    return mot1_step_count;
}

uint8_t bsp_timer_mot1_is_moving(void)
{
    return (mot1_step_count < mot1_target_steps);
}

/* ============================================================================
 * Motor 2 Step Control
 * ============================================================================ */

void bsp_timer_mot2_set_frequency(uint32_t freq_hz)
{
    if (freq_hz == 0)
        freq_hz = 1;
    if (freq_hz > MOTOR_MAX_SPEED)
        freq_hz = MOTOR_MAX_SPEED;

    uint32_t period = (1000000 / freq_hz / 2) - 1;

    htim3.Instance->ARR = period;
    htim3.Instance->PSC = 79;
}

void bsp_timer_mot2_start(uint32_t num_steps)
{
    mot2_target_steps = num_steps;
    mot2_step_count = 0;
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);
}

void bsp_timer_mot2_stop(void)
{
    HAL_TIM_OC_Stop_IT(&htim3, TIM_CHANNEL_1);
    mot2_step_count = 0;
    mot2_target_steps = 0;
}

uint32_t bsp_timer_mot2_steps_done(void)
{
    return mot2_step_count;
}

uint8_t bsp_timer_mot2_is_moving(void)
{
    return (mot2_step_count < mot2_target_steps);
}

/* ============================================================================
 * Timer Interrupt Handlers
 * ============================================================================ */

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        if (mot1_step_count < mot1_target_steps)
        {
            bsp_gpio_motor1_pulse();
            mot1_step_count++;
        }
        else
        {
            bsp_timer_mot1_stop();
        }
    }

    if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        if (mot2_step_count < mot2_target_steps)
        {
            bsp_gpio_motor2_pulse();
            mot2_step_count++;
        }
        else
        {
            bsp_timer_mot2_stop();
        }
    }
}

/* ============================================================================
 * Timer Interrupt Vectors
 * ============================================================================ */

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim3);
}

