/**
  ******************************************************************************
  * @file           : app_mode_jog.c\n  * @brief          : Jog mode (manual motor control via arrow keys)
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

*/


/*
void mode_jog_update(void)
{
    if (cmd_ready) {
        process_runtime_cmd();
    }

    switch (jog_state) {
        case JOG_IDLE:
            if (stop_requested) {
                mode_done = 1;
            }
            else if (jog_cmd_pending) {
                jog_cmd_pending = 0;
                jog_target_axis = jog_cmd.axis;
                jog_target_dir  = jog_cmd.dir;
                jog_target_mode = jog_cmd.mode;   // nudge or continuous
                jog_target_steps = jog_cmd.steps; // used for nudge
                jog_state = JOG_START;
            }
            break;

        case JOG_START:
            if (limit_blocking(jog_target_axis, jog_target_dir)) {
                uart_queue_status("JOG LIMIT\r\n");
                jog_state = JOG_IDLE;
            } else {
                if (jog_target_mode == JOG_MODE_NUDGE) {
                    motion_start_steps(jog_target_axis, jog_target_dir, jog_target_steps);
                    jog_state = JOG_WAIT_DONE;
                } else {
                    motion_start_continuous(jog_target_axis, jog_target_dir);
                    jog_state = JOG_RUN_CONT;
                }
            }
            break;

        case JOG_RUN_CONT:
            if (stop_requested) {
                motion_stop_all();
                mode_done = 1;
            }
            else if (jog_stop_pending) {
                jog_stop_pending = 0;
                motion_stop_axis(jog_target_axis);
                jog_state = JOG_IDLE;
            }
            else if (limit_hit(jog_target_axis, jog_target_dir)) {
                motion_stop_axis(jog_target_axis);
                uart_queue_status("JOG LIMIT\r\n");
                jog_state = JOG_IDLE;
            }
            break;

        case JOG_WAIT_DONE:
            if (stop_requested) {
                motion_stop_all();
                mode_done = 1;
            }
            else if (motion_done_axis(jog_target_axis)) {
                uart_queue_status("JOG DONE\r\n");
                jog_state = JOG_IDLE;
            }
            else if (limit_hit(jog_target_axis, jog_target_dir)) {
                motion_stop_axis(jog_target_axis);
                uart_queue_status("JOG LIMIT\r\n");
                jog_state = JOG_IDLE;
            }
            break;
    }
}
    
*/


