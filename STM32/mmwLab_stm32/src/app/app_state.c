/**
  ******************************************************************************
  * @file           : app_state.c
  * @brief          : System state machine (main orchestrator)
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "bsp_uart.h"
#include "bsp_adc.h"
#include "bsp_gpio.h"
#include "bsp_timer.h"
#include "mot_axis.h"
#include "mot_ctrl.h"
#include "mot_stepper.h"
#include "app_mode_jog.h"
#include "app_mode_cal.h"
#include "app_mode_cap.h"
#include "app_mode_dec.h"
#include "drv_cmd.h"
#include "drv_store.h"

/* ============================================================================
 * System State Machine
 * ============================================================================ */

typedef enum {
    SYS_INIT,
    SYS_IDLE,
    SYS_RUN,
    SYS_ERROR
} SysState_t;

typedef enum {
    APP_NONE,
    APP_JOG,
    APP_CAL,
    APP_CAP,
    APP_DEC
} AppState_t;

static SysState_t sys_state = SYS_INIT;
static AppState_t app_state = APP_NONE;

/* ============================================================================
 * Initialization
 * ============================================================================ */

void app_state_init(void)
{
    /* Initialize all subsystems */
    /* Peripherals (ADC, UART, Timers) are initialized by main.c (CubeMX) */
    bsp_gpio_init();  /* Initialize GPIO motor states after CubeMX pin config */
    bsp_uart_start(); /* Enable UART RX interrupts */
    
    mot_axis_init();
    mot_stepper_init();
    
    drv_cmd_init();
    drv_store_init();
    
    drv_cmd_send_status("System initialized");
    
    sys_state = SYS_IDLE;
    app_state = APP_NONE;
}

/* ============================================================================
 * Main State Update
 * ============================================================================ */

void app_state_update(void)
{
    Command_t cmd = drv_cmd_get_command();

    switch (sys_state)
    {
        case SYS_INIT:
            app_state_init();
            break;

        case SYS_IDLE:
            if (cmd != CMD_NONE)
            {
                if (cmd == CMD_MODE_JOG)
                {
                    sys_state = SYS_RUN;
                    app_state = APP_JOG;
                    drv_cmd_send_status("JOG mode started");
                }
                else if (cmd == CMD_MODE_CAL)
                {
                    sys_state = SYS_RUN;
                    app_state = APP_CAL;
                    drv_cmd_send_status("CALIBRATION mode started");
                }
                else if (cmd == CMD_MODE_CAP)
                {
                    sys_state = SYS_RUN;
                    app_state = APP_CAP;
                    drv_cmd_send_status("CAPTURE mode started");
                }
                else if (cmd == CMD_MODE_DEC)
                {
                    sys_state = SYS_RUN;
                    app_state = APP_DEC;
                    drv_cmd_send_status("DECODE mode started");
                }
            }
            drv_cmd_clear_command();
            break;

        case SYS_RUN:
            /* Call appropriate mode handler */
            switch (app_state)
            {
                case APP_JOG:
                    app_mode_jog_update();
                    break;
                case APP_CAL:
                    app_mode_cal_update();
                    break;
                case APP_CAP:
                    app_mode_cap_update();
                    break;
                case APP_DEC:
                    app_mode_dec_update();
                    break;
                default:
                    break;
            }

            /* Handle cancel command */
            if (cmd == CMD_CANCEL_OP)
            {
                mot_ctrl_stop_all();
                drv_cmd_send_status("Operation cancelled");
                sys_state = SYS_IDLE;
                app_state = APP_NONE;
            }
            
            drv_cmd_clear_command();
            break;

        case SYS_ERROR:
            drv_cmd_send_status("ERROR state");
            break;

        default:
            sys_state = SYS_INIT;
            break;
    }

    /* Update motor state */
    mot_ctrl_update();
}

/* ============================================================================
 * State Query Functions
 * ============================================================================ */

SysState_t app_state_get_system_state(void)
{
    return sys_state;
}

AppState_t app_state_get_app_state(void)
{
    return app_state;
}

uint8_t app_state_is_running(void)
{
    return (sys_state == SYS_RUN);
}

