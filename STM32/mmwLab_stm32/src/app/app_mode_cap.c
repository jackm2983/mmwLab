#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "drv_store.h"
#include "mot_ctrl.h"
#include "mot_axis.h"
#include "bsp_adc.h"

typedef enum {
    CAP_INIT,
    CAP_ROTATE_AXIS1,
    CAP_COLLECT,
    CAP_SEND,
    CAP_ROTATE_AXIS2,
    CAP_DONE
} CapState_t;

static CapState_t cap_state = CAP_INIT;

static uint32_t cap_axis1_step_count = 0;
static uint32_t cap_axis2_step_count = 0;

static int32_t cap_axis1_position = 0;
static int32_t cap_axis2_position = 0;

#define CAP_ROTATION_SPEED      100
#define CAP_STEP_DEG            5

#define CAP_AXIS1_MAX_DEG       360
#define CAP_AXIS2_MAX_DEG       90

#define CAP_AXIS1_STEPS         (CAP_AXIS1_MAX_DEG / CAP_STEP_DEG)
#define CAP_AXIS2_STEPS         (CAP_AXIS2_MAX_DEG / CAP_STEP_DEG)

void app_mode_cap_update(void)
{
    switch (cap_state)
    {
        case CAP_INIT:
            drv_cmd_send_status("CAP: Starting 2-axis sweep");

            cap_axis1_step_count = 0;
            cap_axis2_step_count = 0;

            cap_axis1_position = 0;
            cap_axis2_position = 0;

            mot_ctrl_move_axis2_abs(cap_axis2_position, CAP_ROTATION_SPEED);
            cap_state = CAP_ROTATE_AXIS2;
            break;

        case CAP_ROTATE_AXIS2:
            if (!mot_axis2_is_moving())
            {
                cap_axis1_step_count = 0;
                cap_axis1_position = 0;

                mot_ctrl_move_axis1_abs(cap_axis1_position, CAP_ROTATION_SPEED);
                cap_state = CAP_ROTATE_AXIS1;
            }
            break;

        case CAP_ROTATE_AXIS1:
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
            /*
             * send or store captured data here.
             * include axis1 and axis2 positions if needed.
             */

            cap_axis1_step_count++;

            if (cap_axis1_step_count < CAP_AXIS1_STEPS)
            {
                cap_axis1_position += CAP_STEP_DEG;
                mot_ctrl_move_axis1_abs(cap_axis1_position, CAP_ROTATION_SPEED);
                cap_state = CAP_ROTATE_AXIS1;
            }
            else
            {
                cap_axis2_step_count++;

                if (cap_axis2_step_count <= CAP_AXIS2_STEPS)
                {
                    cap_axis2_position += CAP_STEP_DEG;
                    mot_ctrl_move_axis2_abs(cap_axis2_position, CAP_ROTATION_SPEED);
                    cap_state = CAP_ROTATE_AXIS2;
                }
                else
                {
                    drv_cmd_send_status("CAP: 2-axis sweep complete");
                    cap_state = CAP_DONE;
                }
            }
            break;

        case CAP_DONE:
            break;

        default:
            cap_state = CAP_DONE;
            break;
    }
}

void app_mode_cap_reset(void)
{
    cap_state = CAP_INIT;

    cap_axis1_step_count = 0;
    cap_axis2_step_count = 0;

    cap_axis1_position = 0;
    cap_axis2_position = 0;
}