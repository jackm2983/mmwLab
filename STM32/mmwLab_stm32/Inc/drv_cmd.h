/**
  ******************************************************************************
  * @file           : drv_cmd.h
  * @brief          : Driver command parsing header
  ******************************************************************************
*/

#ifndef DRV_CMD_H
#define DRV_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Command Enumeration
 * ============================================================================ */

typedef enum {
    CMD_NONE,
    CMD_JOG_UP,
    CMD_JOG_DOWN,
    CMD_JOG_LEFT,
    CMD_JOG_RIGHT,
    CMD_CANCEL_OP,
    CMD_MARK_POINT,
    CMD_MODE_JOG,
    CMD_MODE_CAL,
    CMD_MODE_CAP,
    CMD_MODE_DEC,
} Command_t;

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

void drv_cmd_init(void);
Command_t drv_cmd_get_command(void);
Command_t drv_cmd_get_last_command(void);
void drv_cmd_clear_command(void);
void drv_cmd_send_status(const char *status);
void drv_cmd_send_position(const char *axis, int32_t pos);

#ifdef __cplusplus
}
#endif

#endif /* DRV_CMD_H */
