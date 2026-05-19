/**
  ******************************************************************************
  * @file           : app_mode_cal.c
  * @brief          : Calibrate mode (jog + mark two endpoints)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "drv_store.h"
#include "mot_ctrl.h"
#include "mot_axis.h"
#include "bsp_uart.h"

#define CAL_DEBUG       1

#define CAL_JOG_SPEED       4000
#define CAL_JOG_TIMEOUT_MS  30

typedef enum {
    CAL_WAIT_START,
    CAL_WAIT_END,
    CAL_COMPLETE
} CalState_t;

typedef enum {
    CJOG_DIR_NONE,
    CJOG_DIR_AX1_FWD,
    CJOG_DIR_AX1_REV,
    CJOG_DIR_AX2_FWD,
    CJOG_DIR_AX2_REV
} CalJogDir_t;

static CalState_t cal_state = CAL_WAIT_START;
static CalJogDir_t cal_jog_dir = CJOG_DIR_NONE;
static uint32_t cal_last_key_tick = 0;
static uint8_t cal_announced = 0;

static int32_t cal_start_pos1 = 0;
static int32_t cal_start_pos2 = 0;
static int32_t cal_end_pos1 = 0;
static int32_t cal_end_pos2 = 0;

static void cal_jog_start(CalJogDir_t dir)
{
    if (dir == cal_jog_dir) return;

    mot_ctrl_stop_all();

    switch (dir) {
        case CJOG_DIR_AX1_FWD:
            if (mot_axis1_limit_triggered()) {
                drv_cmd_send_status("CAL: axis 1 fwd limit");
                return;
            }
            mot_ctrl_jog_axis1(1, CAL_JOG_SPEED);
            break;
        case CJOG_DIR_AX1_REV:
            mot_ctrl_jog_axis1(0, CAL_JOG_SPEED);
            break;
        case CJOG_DIR_AX2_FWD:
            if (mot_axis2_limit_triggered()) {
                drv_cmd_send_status("CAL: axis 2 fwd limit");
                return;
            }
            mot_ctrl_jog_axis2(1, CAL_JOG_SPEED);
            break;
        case CJOG_DIR_AX2_REV:
            mot_ctrl_jog_axis2(0, CAL_JOG_SPEED);
            break;
        default:
            break;
    }
    cal_jog_dir = dir;
}

static void cal_jog_stop(void)
{
    if (cal_jog_dir == CJOG_DIR_NONE) return;
    mot_ctrl_stop_all();
    cal_jog_dir = CJOG_DIR_NONE;
}

void app_mode_cal_update(void)
{
    Command_t cmd = drv_cmd_get_last_command();
    uint32_t now = HAL_GetTick();

    if (!cal_announced) {
        drv_cmd_send_status("CAL: jog with WASD, press m to mark START");
        cal_announced = 1;
    }

    // jog handling, same as jog mode
    switch (cmd) {
        case CMD_JOG_UP:    cal_last_key_tick = now; cal_jog_start(CJOG_DIR_AX2_FWD); break;
        case CMD_JOG_DOWN:  cal_last_key_tick = now; cal_jog_start(CJOG_DIR_AX2_REV); break;
        case CMD_JOG_LEFT:  cal_last_key_tick = now; cal_jog_start(CJOG_DIR_AX1_REV); break;
        case CMD_JOG_RIGHT: cal_last_key_tick = now; cal_jog_start(CJOG_DIR_AX1_FWD); break;
        default: break;
    }

    if (cal_jog_dir != CJOG_DIR_NONE && (now - cal_last_key_tick) > CAL_JOG_TIMEOUT_MS) {
        cal_jog_stop();
    }

    // mark point handling
    switch (cal_state) {
        case CAL_WAIT_START:
            if (cmd == CMD_MARK_POINT) {
                cal_jog_stop();
                cal_start_pos1 = mot_axis1_get_position();
                cal_start_pos2 = mot_axis2_get_position();

                drv_cmd_send_status("CAL: START marked");
                drv_cmd_send_position("  ax1", cal_start_pos1);
                drv_cmd_send_position("  ax2", cal_start_pos2);
                drv_cmd_send_status("CAL: jog to END, press m to mark");

                cal_state = CAL_WAIT_END;
            }
            break;

        case CAL_WAIT_END:
            if (cmd == CMD_MARK_POINT) {
                cal_jog_stop();
                cal_end_pos1 = mot_axis1_get_position();
                cal_end_pos2 = mot_axis2_get_position();

                drv_cmd_send_status("CAL: END marked");
                drv_cmd_send_position("  ax1", cal_end_pos1);
                drv_cmd_send_position("  ax2", cal_end_pos2);

                drv_store_set_axis1_bounds(cal_start_pos1, cal_end_pos1);
                drv_store_set_axis2_bounds(cal_start_pos2, cal_end_pos2);

                uint8_t rc = drv_store_save();
                if (rc == 0) {
                    drv_cmd_send_status("CAL: saved to flash");
                } else {
                    drv_cmd_send_status("CAL: flash save FAILED");
#if CAL_DEBUG
                    bsp_uart_send_string("DBG cal: drv_store_save rc=");
                    char b[12]; int n = 0;
                    if (rc == 0) b[n++] = '0';
                    else { char t[12]; int j=0; uint32_t v=rc; while(v){t[j++]='0'+v%10;v/=10;} while(j)b[n++]=t[--j]; }
                    b[n]=0;
                    bsp_uart_send_string(b);
                    bsp_uart_send_string("\r\n");
#endif
                }

                cal_state = CAL_COMPLETE;
            }
            break;

        case CAL_COMPLETE:
            // sit here until cancel returns us to idle
            break;

        default:
            break;
    }
}

void app_mode_cal_reset(void)
{
    cal_jog_stop();
    cal_state = CAL_WAIT_START;
    cal_jog_dir = CJOG_DIR_NONE;
    cal_last_key_tick = 0;
    cal_announced = 0;
    cal_start_pos1 = 0;
    cal_start_pos2 = 0;
    cal_end_pos1 = 0;
    cal_end_pos2 = 0;
}