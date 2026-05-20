/**
  ******************************************************************************
  * @file           : mot_axis.c
  * @brief          : Motor axis state management
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "cfg_pins.h"
#include "bsp_gpio.h"

#define AXIS_DEFAULT_MAX    1000000
#define AXIS_DEFAULT_MIN    (-1000000)

typedef struct {
    int32_t position;
    int32_t home_position;
    uint8_t is_homed;
    uint8_t is_moving;
    uint8_t limit_triggered;
    int32_t max_position;
    int32_t min_position;
} AxisState_t;

static AxisState_t axis1_state = {0};
static AxisState_t axis2_state = {0};

void mot_axis_init(void)
{
    axis1_state.position = 0;
    axis1_state.home_position = 0;
    axis1_state.is_homed = 0;
    axis1_state.is_moving = 0;
    axis1_state.limit_triggered = 0;
    axis1_state.max_position = AXIS_DEFAULT_MAX;
    axis1_state.min_position = AXIS_DEFAULT_MIN;

    axis2_state.position = 0;
    axis2_state.home_position = 0;
    axis2_state.is_homed = 0;
    axis2_state.is_moving = 0;
    axis2_state.limit_triggered = 0;
    axis2_state.max_position = AXIS_DEFAULT_MAX;
    axis2_state.min_position = AXIS_DEFAULT_MIN;
}

int32_t mot_axis1_get_position(void) { return axis1_state.position; }
void mot_axis1_set_position(int32_t pos) { axis1_state.position = pos; }

void mot_axis1_update_position(int32_t delta)
{
    axis1_state.position += delta;
    if (axis1_state.position < axis1_state.min_position)
        axis1_state.position = axis1_state.min_position;
    if (axis1_state.position > axis1_state.max_position)
        axis1_state.position = axis1_state.max_position;
}

void mot_axis1_set_homed(void)
{
    axis1_state.is_homed = 1;
    axis1_state.home_position = axis1_state.position;
}

uint8_t mot_axis1_is_homed(void) { return axis1_state.is_homed; }
void mot_axis1_set_moving(uint8_t moving) { axis1_state.is_moving = moving; }
uint8_t mot_axis1_is_moving(void) { return axis1_state.is_moving; }
void mot_axis1_update_limits(void) { axis1_state.limit_triggered = bsp_gpio_limit1_read(); }

uint8_t mot_axis1_limit_triggered(void)
{
    axis1_state.limit_triggered = bsp_gpio_limit1_read();
    return axis1_state.limit_triggered;
}

void mot_axis1_set_bounds(int32_t min, int32_t max)
{
    axis1_state.min_position = min;
    axis1_state.max_position = max;
}

int32_t mot_axis2_get_position(void) { return axis2_state.position; }
void mot_axis2_set_position(int32_t pos) { axis2_state.position = pos; }

void mot_axis2_update_position(int32_t delta)
{
    axis2_state.position += delta;
    if (axis2_state.position < axis2_state.min_position)
        axis2_state.position = axis2_state.min_position;
    if (axis2_state.position > axis2_state.max_position)
        axis2_state.position = axis2_state.max_position;
}

void mot_axis2_set_homed(void)
{
    axis2_state.is_homed = 1;
    axis2_state.home_position = axis2_state.position;
}

uint8_t mot_axis2_is_homed(void) { return axis2_state.is_homed; }
void mot_axis2_set_moving(uint8_t moving) { axis2_state.is_moving = moving; }
uint8_t mot_axis2_is_moving(void) { return axis2_state.is_moving; }
void mot_axis2_update_limits(void) { axis2_state.limit_triggered = bsp_gpio_limit2_read(); }

uint8_t mot_axis2_limit_triggered(void)
{
    axis2_state.limit_triggered = bsp_gpio_limit2_read();
    return axis2_state.limit_triggered;
}

void mot_axis2_set_bounds(int32_t min, int32_t max)
{
    axis2_state.min_position = min;
    axis2_state.max_position = max;
}
