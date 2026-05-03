/**
  ******************************************************************************
  * @file           : drv_cmd.c
  * @brief          : Driver for UART command parsing
  ******************************************************************************
*/

#include "main.h"
#include "bsp_uart.h"

/* ============================================================================
 * Command Definitions
 * ============================================================================ */

#define CMD_ARROW_UP    'w'
#define CMD_ARROW_DOWN  's'
#define CMD_ARROW_LEFT  'a'
#define CMD_ARROW_RIGHT 'd'
#define CMD_CANCEL      'x'
#define CMD_MARK        'm'

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

static Command_t last_command = CMD_NONE;

/* ============================================================================
 * Command Processing
 * ============================================================================ */

void drv_cmd_init(void)
{
    bsp_uart_init();
}

Command_t drv_cmd_parse_char(uint8_t c)
{
    Command_t cmd = CMD_NONE;

    switch (c)
    {
        case CMD_ARROW_UP:
            cmd = CMD_JOG_UP;
            break;
        case CMD_ARROW_DOWN:
            cmd = CMD_JOG_DOWN;
            break;
        case CMD_ARROW_LEFT:
            cmd = CMD_JOG_LEFT;
            break;
        case CMD_ARROW_RIGHT:
            cmd = CMD_JOG_RIGHT;
            break;
        case CMD_CANCEL:
            cmd = CMD_CANCEL_OP;
            break;
        case CMD_MARK:
            cmd = CMD_MARK_POINT;
            break;
        case 'j':
            cmd = CMD_MODE_JOG;
            break;
        case 'c':
            cmd = CMD_MODE_CAL;
            break;
        case 'p':
            cmd = CMD_MODE_CAP;
            break;
        case 'd':
            cmd = CMD_MODE_DEC;
            break;
        default:
            cmd = CMD_NONE;
            break;
    }

    return cmd;
}

Command_t drv_cmd_get_command(void)
{
    if (bsp_uart_data_available())
    {
        uint8_t c = bsp_uart_read_char();
        last_command = drv_cmd_parse_char(c);
        return last_command;
    }

    return CMD_NONE;
}

Command_t drv_cmd_get_last_command(void)
{
    return last_command;
}

void drv_cmd_clear_command(void)
{
    last_command = CMD_NONE;
}

/* ============================================================================
 * UART Output Helpers
 * ============================================================================ */

void drv_cmd_send_status(const char *status)
{
    bsp_uart_send_string("STATUS: ");
    bsp_uart_send_string(status);
    bsp_uart_send_string("\r\n");
}

void drv_cmd_send_position(const char *axis, int32_t pos)
{
    char buf[32];
    bsp_uart_send_string(axis);
    bsp_uart_send_string(" POS: ");
    
    if (pos < 0)
    {
        bsp_uart_send_char('-');
        pos = -pos;
    }
    
    /* Simple int to string */
    if (pos >= 10000) bsp_uart_send_char('0' + (pos / 10000) % 10);
    if (pos >= 1000) bsp_uart_send_char('0' + (pos / 1000) % 10);
    if (pos >= 100) bsp_uart_send_char('0' + (pos / 100) % 10);
    if (pos >= 10) bsp_uart_send_char('0' + (pos / 10) % 10);
    bsp_uart_send_char('0' + (pos % 10));
    bsp_uart_send_string("\r\n");
}

  // commands for setting the start and stop position in cal

  