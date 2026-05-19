/**
  ******************************************************************************
  * @file           : app_mode_cap.c
  * @brief          : 2-axis sweep capture (motor1 inner, motor2 outer)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "drv_store.h"
#include "mot_ctrl.h"
#include "mot_axis.h"
#include "bsp_adc.h"
#include "bsp_uart.h"

#define CAP_DEBUG               1

#define CAP_ROTATION_SPEED      4000
#define CAP_AXIS1_SUBDIVISIONS  72    // 360° / 5° = 72 steps over the full sweep
#define CAP_AXIS2_SUBDIVISIONS  18    // 90° / 5° = 18 steps over the full sweep
#define CAP_SETTLE_MS           50

typedef enum {
    CAP_INIT,
    CAP_HOME_AX2,
    CAP_HOME_AX1,
    CAP_SETTLE,
    CAP_MEASURE,
    CAP_NEXT,
    CAP_DONE
} CapState_t;

static CapState_t cap_state = CAP_INIT;
static uint32_t cap_settle_start = 0;

// indices into the sweep grid, 0..SUBDIVISIONS inclusive
static int32_t cap_ax1_idx = 0;
static int32_t cap_ax2_idx = 0;

// step positions for sweep, computed from calibration at init
static int32_t cap_ax1_start = 0;
static int32_t cap_ax1_end = 0;
static int32_t cap_ax2_start = 0;
static int32_t cap_ax2_end = 0;

static uint32_t cap_samples_done = 0;
static uint32_t cap_samples_total = 0;

static void send_int(int32_t v)
{
    if (v < 0) { bsp_uart_send_char('-'); v = -v; }
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

// linear interp from start to end based on idx / total
static int32_t interp_pos(int32_t start, int32_t end, int32_t idx, int32_t total)
{
    if (total == 0) return start;
    return start + ((end - start) * idx) / total;
}

static void cap_emit_sample(int32_t ax1_pos, int32_t ax2_pos, uint16_t i_val, uint16_t q_val)
{
    bsp_uart_send_string("cap,");
    send_int(cap_ax1_idx);
    bsp_uart_send_char(',');
    send_int(cap_ax2_idx);
    bsp_uart_send_char(',');
    send_int(ax1_pos);
    bsp_uart_send_char(',');
    send_int(ax2_pos);
    bsp_uart_send_char(',');
    send_int(i_val);
    bsp_uart_send_char(',');
    send_int(q_val);
    bsp_uart_send_string("\r\n");
}

static void cap_emit_progress(void)
{
    bsp_uart_send_string("CAP: ");
    send_int(cap_samples_done);
    bsp_uart_send_char('/');
    send_int(cap_samples_total);
    bsp_uart_send_string(" ax1_idx=");
    send_int(cap_ax1_idx);
    bsp_uart_send_string(" ax2_idx=");
    send_int(cap_ax2_idx);
    bsp_uart_send_string("\r\n");
}

void app_mode_cap_update(void)
{
    switch (cap_state)
    {
        case CAP_INIT: {
            drv_cmd_send_status("CAP: starting 2-axis sweep");

            // load calibrated bounds. start of sweep = lower bound (closer to 0),
            // end of sweep = far bound. drv_store always returns min<=max.
            int32_t a1min, a1max, a2min, a2max;
            drv_store_get_axis1_bounds(&a1min, &a1max);
            drv_store_get_axis2_bounds(&a2min, &a2max);

            // sweep direction: if cal end position is negative relative to start,
            // we sweep toward the negative direction. infer from which bound is
            // farther from zero.
            // for this rig: cal stored as [end, start] sorted to [min, max] where
            // min is the more negative value. so sweep from 0 (or max) down to min.
            cap_ax1_start = a1max;  // typically the "home" / near-zero end
            cap_ax1_end   = a1min;  // typically the "far" end
            cap_ax2_start = a2max;
            cap_ax2_end   = a2min;

#if CAP_DEBUG
            bsp_uart_send_string("DBG cap: sweep ax1 ");
            send_int(cap_ax1_start);
            bsp_uart_send_string(" -> ");
            send_int(cap_ax1_end);
            bsp_uart_send_string("\r\n");
            bsp_uart_send_string("DBG cap: sweep ax2 ");
            send_int(cap_ax2_start);
            bsp_uart_send_string(" -> ");
            send_int(cap_ax2_end);
            bsp_uart_send_string("\r\n");
#endif

            cap_ax1_idx = 0;
            cap_ax2_idx = 0;
            cap_samples_done = 0;
            cap_samples_total = (CAP_AXIS1_SUBDIVISIONS + 1) * (CAP_AXIS2_SUBDIVISIONS + 1);

            // move both to sweep start
            int32_t ax1_target = interp_pos(cap_ax1_start, cap_ax1_end, 0, CAP_AXIS1_SUBDIVISIONS);
            int32_t ax2_target = interp_pos(cap_ax2_start, cap_ax2_end, 0, CAP_AXIS2_SUBDIVISIONS);

            mot_ctrl_move_axis2_abs(ax2_target, CAP_ROTATION_SPEED);
            cap_state = CAP_HOME_AX2;

            (void)ax1_target;  // will be commanded after ax2 settles
            break;
        }

        case CAP_HOME_AX2:
            if (!mot_axis2_is_moving()) {
                int32_t ax1_target = interp_pos(cap_ax1_start, cap_ax1_end,
                                                cap_ax1_idx, CAP_AXIS1_SUBDIVISIONS);
                mot_ctrl_move_axis1_abs(ax1_target, CAP_ROTATION_SPEED);
                cap_state = CAP_HOME_AX1;
            }
            break;

        case CAP_HOME_AX1:
            if (!mot_axis1_is_moving()) {
                cap_settle_start = HAL_GetTick();
                cap_state = CAP_SETTLE;
            }
            break;

        case CAP_SETTLE:
            if ((HAL_GetTick() - cap_settle_start) >= CAP_SETTLE_MS) {
                cap_state = CAP_MEASURE;
            }
            break;

        case CAP_MEASURE: {
            uint16_t i_val = 0;
            uint16_t q_val = 0;
            bsp_adc_read_single(&i_val, &q_val);

            int32_t ax1_pos = mot_axis1_get_position();
            int32_t ax2_pos = mot_axis2_get_position();
            cap_emit_sample(ax1_pos, ax2_pos, i_val, q_val);
            cap_samples_done++;

#if CAP_DEBUG
            if ((cap_samples_done % 10) == 0) {
                cap_emit_progress();
            }
#endif
            cap_state = CAP_NEXT;
            break;
        }

        case CAP_NEXT:
            // advance axis 1 (inner loop)
            cap_ax1_idx++;

            if (cap_ax1_idx <= CAP_AXIS1_SUBDIVISIONS) {
                int32_t ax1_target = interp_pos(cap_ax1_start, cap_ax1_end,
                                                cap_ax1_idx, CAP_AXIS1_SUBDIVISIONS);
                mot_ctrl_move_axis1_abs(ax1_target, CAP_ROTATION_SPEED);
                cap_state = CAP_HOME_AX1;
            } else {
                // axis 1 sweep done, advance axis 2 (outer loop)
                cap_ax2_idx++;

                if (cap_ax2_idx <= CAP_AXIS2_SUBDIVISIONS) {
                    cap_ax1_idx = 0;
                    int32_t ax2_target = interp_pos(cap_ax2_start, cap_ax2_end,
                                                    cap_ax2_idx, CAP_AXIS2_SUBDIVISIONS);
                    mot_ctrl_move_axis2_abs(ax2_target, CAP_ROTATION_SPEED);
                    cap_state = CAP_HOME_AX2;
                } else {
                    bsp_uart_send_string("cap_end\r\n");
                    drv_cmd_send_status("CAP: sweep complete");
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
    mot_ctrl_stop_all();
    cap_state = CAP_INIT;
    cap_ax1_idx = 0;
    cap_ax2_idx = 0;
    cap_samples_done = 0;
    cap_samples_total = 0;
    cap_settle_start = 0;
}