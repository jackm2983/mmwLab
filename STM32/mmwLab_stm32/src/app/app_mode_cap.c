/**
  ******************************************************************************
  * @file           : app_mode_cap.c
  * @brief          : Capture mode control. Characterizing the antenna by received transmission power.
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "drv_store.h"
#include "mot_ctrl.h"
#include "mot_axis.h"
#include "bsp_adc.h"

typedef enum {
    CAP_INIT,
    CAP_ROTATE,
    CAP_COLLECT,
    CAP_SEND,
    CAP_DONE
} CapState_t;

static CapState_t cap_state = CAP_INIT;
static uint32_t cap_step_count = 0;
static int32_t cap_position = 0;
static int32_t cap_max_steps = 72;

#define CAP_ROTATION_SPEED  100

void app_mode_cap_update(void)
{
    switch (cap_state)
    {
        case CAP_INIT:
            drv_cmd_send_status("CAP: Starting sweep");
            cap_step_count = 0;
            cap_position = 0;
            mot_ctrl_move_axis1_abs(cap_position, CAP_ROTATION_SPEED);
            cap_state = CAP_ROTATE;
            break;

        case CAP_ROTATE:
            if (!mot_axis1_is_moving())
            {
                bsp_adc_sampling_reset();
                bsp_adc_start_continuous();
                cap_state = CAP_COLLECT;
            }
            break;

        case CAP_COLLECT:
            if (bsp_adc_sampling_complete())
            {
                bsp_adc_stop();
                cap_state = CAP_SEND;
            }
            break;

        case CAP_SEND:
            cap_step_count++;
            if (cap_step_count < cap_max_steps)
            {
                cap_position += 5;
                mot_ctrl_move_axis1_abs(cap_position, CAP_ROTATION_SPEED);
                cap_state = CAP_ROTATE;
            }
            else
            {
                cap_state = CAP_DONE;
                drv_cmd_send_status("CAP: Sweep complete");
            }
            break;

        case CAP_DONE:
            break;

        default:
            break;
    }
}

void app_mode_cap_reset(void)
{
    cap_state = CAP_INIT;
    cap_step_count = 0;
}


