/**
  ******************************************************************************
  * @file           : app_mode_jog.c
  * @brief          : Jog mode (manual motor control via WASD keys)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "bsp_uart.h"
#include "mot_ctrl.h"
#include "mot_axis.h"
#include "bsp_timer.h"

#define JOG_DEBUG       0

#define JOG_SPEED       4000
#define JOG_TIMEOUT_MS  30
#define JOG_STATUS_MS   500

typedef enum {
    JOG_DIR_NONE,
    JOG_DIR_AX1_FWD,
    JOG_DIR_AX1_REV,
    JOG_DIR_AX2_FWD,
    JOG_DIR_AX2_REV
} JogDir_t;

static JogDir_t current_dir = JOG_DIR_NONE;
static uint32_t last_key_tick = 0;
static uint32_t last_status_tick = 0;
static uint32_t jog_start_tick = 0;

#if JOG_DEBUG
static const char* dir_str(JogDir_t d)
{
    switch (d) {
        case JOG_DIR_NONE:    return "NONE";
        case JOG_DIR_AX1_FWD: return "AX1_FWD";
        case JOG_DIR_AX1_REV: return "AX1_REV";
        case JOG_DIR_AX2_FWD: return "AX2_FWD";
        case JOG_DIR_AX2_REV: return "AX2_REV";
        default: return "?";
    }
}

static void dbg_u32(const char *label, uint32_t v)
{
    bsp_uart_send_string(label);
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
#endif

static void jog_start(JogDir_t dir)
{
#if JOG_DEBUG
    bsp_uart_send_string("DBG jog: start req=");
    bsp_uart_send_string(dir_str(dir));
    bsp_uart_send_string(" cur=");
    bsp_uart_send_string(dir_str(current_dir));
    bsp_uart_send_string("\r\n");
#endif

    if (dir == current_dir) {
        return;
    }

    mot_ctrl_stop_all();
    uint8_t rc = 0xFF;

    switch (dir) {
        case JOG_DIR_AX1_FWD:
            rc = mot_ctrl_jog_axis1(1, JOG_SPEED);
            drv_cmd_send_status("axis 1 fwd");
            break;
        case JOG_DIR_AX1_REV:
            rc = mot_ctrl_jog_axis1(0, JOG_SPEED);
            drv_cmd_send_status("axis 1 rev");
            break;
        case JOG_DIR_AX2_FWD:
            rc = mot_ctrl_jog_axis2(1, JOG_SPEED);
            drv_cmd_send_status("axis 2 fwd");
            break;
        case JOG_DIR_AX2_REV:
            rc = mot_ctrl_jog_axis2(0, JOG_SPEED);
            drv_cmd_send_status("axis 2 rev");
            break;
        default:
            break;
    }

    (void)rc;
    current_dir = dir;
    jog_start_tick = HAL_GetTick();
    last_status_tick = jog_start_tick;
}

static void jog_stop(void)
{
    if (current_dir == JOG_DIR_NONE) return;

#if JOG_DEBUG
    uint32_t elapsed = HAL_GetTick() - jog_start_tick;
    bsp_uart_send_string("DBG jog: stop after ");
    dbg_u32("", elapsed);
    bsp_uart_send_string(" ms\r\n");
#endif

    mot_ctrl_stop_all();
    current_dir = JOG_DIR_NONE;
    drv_cmd_send_status("jog stopped");
}

static void jog_limit_update(void)
{
    if (current_dir == JOG_DIR_AX1_FWD && mot_axis1_limit_triggered()) {
        jog_stop();
        mot_axis1_set_homed();
        drv_cmd_send_status("axis 1 limit");
    }

    if (current_dir == JOG_DIR_AX2_FWD && mot_axis2_limit_triggered()) {
        jog_stop();
        mot_axis2_set_homed();
        drv_cmd_send_status("axis 2 limit");
    }
}

void app_mode_jog_update(void)
{
    Command_t cmd = drv_cmd_get_last_command();
    uint32_t now = HAL_GetTick();

#if JOG_DEBUG >= 2
    if (cmd != CMD_NONE || current_dir != JOG_DIR_NONE) {
        bsp_uart_send_string("DBG jog loop: cmd=");
        dbg_u32("", (uint32_t)cmd);
        bsp_uart_send_string(" dir=");
        bsp_uart_send_string(dir_str(current_dir));
        bsp_uart_send_string(" gap=");
        dbg_u32("", now - last_key_tick);
        bsp_uart_send_string("\r\n");
    }
#endif

    switch (cmd) {
        case CMD_JOG_UP:    last_key_tick = now; jog_start(JOG_DIR_AX2_FWD); break;
        case CMD_JOG_DOWN:  last_key_tick = now; jog_start(JOG_DIR_AX2_REV); break;
        case CMD_JOG_LEFT:  last_key_tick = now; jog_start(JOG_DIR_AX1_REV); break;
        case CMD_JOG_RIGHT: last_key_tick = now; jog_start(JOG_DIR_AX1_FWD); break;
        default: break;
    }

    jog_limit_update();

#if JOG_DEBUG
    if (current_dir != JOG_DIR_NONE && (now - last_status_tick) > JOG_STATUS_MS) {
        last_status_tick = now;
        bsp_uart_send_string("DBG jog: alive dir=");
        bsp_uart_send_string(dir_str(current_dir));
        bsp_uart_send_string(" ax1_pos=");
        int32_t p = mot_axis1_get_position();
        if (p < 0) { bsp_uart_send_char('-'); p = -p; }
        dbg_u32("", (uint32_t)p);
        bsp_uart_send_string(" ax1_moving=");
        dbg_u32("", mot_axis1_is_moving());
        bsp_uart_send_string(" ax2_pos=");
        p = mot_axis2_get_position();
        if (p < 0) { bsp_uart_send_char('-'); p = -p; }
        dbg_u32("", (uint32_t)p);
        bsp_uart_send_string(" ax2_moving=");
        dbg_u32("", mot_axis2_is_moving());
        bsp_uart_send_string(" key_gap=");
        dbg_u32("", now - last_key_tick);
        bsp_uart_send_string("\r\n");
    }
#endif

    if (current_dir != JOG_DIR_NONE && (now - last_key_tick) > JOG_TIMEOUT_MS) {
        jog_stop();
    }
}

void app_mode_jog_reset(void)
{
    jog_stop();
    last_key_tick = 0;
    last_status_tick = 0;
    jog_start_tick = 0;
}
