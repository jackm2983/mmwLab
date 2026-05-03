/**
  ******************************************************************************
  * @file           : app_mode_jog.c
  * @brief          : Jog mode (manual motor control via arrow keys)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "mot_ctrl.h"
#include "mot_axis.h"

/* ============================================================================
 * Jog Mode - Direct User Control
 * ============================================================================ */

#define JOG_SPEED 100  /* Hz (steps per second) */

static uint8_t axis1_jogging = 0;
static uint8_t axis2_jogging = 0;

void app_mode_jog_update(void)
{
    Command_t cmd = drv_cmd_get_last_command();

    switch (cmd)
    {
        case CMD_JOG_UP:
            /* Axis 2 forward (if not at limit) */
            if (!mot_axis2_limit_triggered())
            {
                if (!axis2_jogging)
                {
                    mot_ctrl_move_axis2_rel(1, JOG_SPEED);
                    axis2_jogging = 1;
                }
            }
            break;

        case CMD_JOG_DOWN:
            /* Axis 2 backward (if not at limit) */
            if (!axis2_jogging)
            {
                mot_ctrl_move_axis2_rel(-1, JOG_SPEED);
                axis2_jogging = 1;
            }
            break;

        case CMD_JOG_LEFT:
            /* Axis 1 backward (if not at limit) */
            if (!axis1_jogging)
            {
                mot_ctrl_move_axis1_rel(-1, JOG_SPEED);
                axis1_jogging = 1;
            }
            break;

        case CMD_JOG_RIGHT:
            /* Axis 1 forward (if not at limit) */
            if (!mot_axis1_limit_triggered())
            {
                if (!axis1_jogging)
                {
                    mot_ctrl_move_axis1_rel(1, JOG_SPEED);
                    axis1_jogging = 1;
                }
            }
            break;

        default:
            break;
    }

    /* Update motor state */
    if (!mot_axis1_is_moving())
        axis1_jogging = 0;
    if (!mot_axis2_is_moving())
        axis2_jogging = 0;
}
