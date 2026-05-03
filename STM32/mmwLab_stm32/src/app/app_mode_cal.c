/**
  ******************************************************************************
  * @file           : app_mode_cal.c
  * @brief          : Calibrate mode control
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "drv_store.h"
#include "mot_ctrl.h"
#include "mot_axis.h"

/* ============================================================================
 * Calibration Mode - Manual Range Setting
 * ============================================================================ */

typedef enum {
    CAL_WAIT_START,
    CAL_WAIT_END,
    CAL_COMPLETE
} CalState_t;

static CalState_t cal_state = CAL_WAIT_START;
static int32_t cal_start_pos1 = 0;
static int32_t cal_start_pos2 = 0;
static int32_t cal_end_pos1 = 0;
static int32_t cal_end_pos2 = 0;

void app_mode_cal_update(void)
{
    Command_t cmd = drv_cmd_get_last_command();

    switch (cal_state)
    {
        case CAL_WAIT_START:
            if (cmd == CMD_MARK_POINT)
            {
                cal_start_pos1 = mot_axis1_get_position();
                cal_start_pos2 = mot_axis2_get_position();
                drv_cmd_send_status("CAL: START point marked");
                cal_state = CAL_WAIT_END;
            }
            break;

        case CAL_WAIT_END:
            if (cmd == CMD_MARK_POINT)
            {
                cal_end_pos1 = mot_axis1_get_position();
                cal_end_pos2 = mot_axis2_get_position();
                drv_cmd_send_status("CAL: END point marked");
                
                drv_store_set_axis1_bounds(cal_start_pos1, cal_end_pos1);
                drv_store_set_axis2_bounds(cal_start_pos2, cal_end_pos2);
                drv_store_save();
                
                drv_cmd_send_status("CAL: Saved to flash");
                cal_state = CAL_COMPLETE;
            }
            break;

        case CAL_COMPLETE:
            break;

        default:
            break;
    }
}

void app_mode_cal_reset(void)
{
    cal_state = CAL_WAIT_START;
}


