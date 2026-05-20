/**
  ******************************************************************************
  * @file           : app_mode_dec.c
  * @brief          : Continuous I/Q read and emit, no movement
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "bsp_adc.h"
#include "bsp_uart.h"

#define DEC_INTERVAL_MS     100

typedef enum {
    DEC_INIT,
    DEC_STREAMING,
    DEC_STOP
} DecState_t;

static DecState_t dec_state = DEC_INIT;
static uint32_t dec_sample_count = 0;
static uint32_t dec_last_emit = 0;

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

static void dec_emit_sample(uint16_t i_val, uint16_t q_val)
{
    bsp_uart_send_string("cap,");
    send_int(0);
    bsp_uart_send_char(',');
    send_int(0);
    bsp_uart_send_char(',');
    send_int(0);
    bsp_uart_send_char(',');
    send_int(0);
    bsp_uart_send_char(',');
    send_int(i_val);
    bsp_uart_send_char(',');
    send_int(q_val);
    bsp_uart_send_string("\r\n");
}

void app_mode_dec_update(void)
{
    switch (dec_state)
    {
        case DEC_INIT:
            drv_cmd_send_status("DEC: starting I/Q stream");
            dec_sample_count = 0;
            dec_last_emit = HAL_GetTick();
            dec_state = DEC_STREAMING;
            break;

        case DEC_STREAMING:
            if ((HAL_GetTick() - dec_last_emit) >= DEC_INTERVAL_MS) {
                dec_last_emit = HAL_GetTick();

                uint16_t i_val = 0;
                uint16_t q_val = 0;
                bsp_adc_read_single(&i_val, &q_val);

                dec_emit_sample(i_val, q_val);
                dec_sample_count++;
            }
            break;

        case DEC_STOP:
            drv_cmd_send_status("DEC: stream complete");
            break;

        default:
            dec_state = DEC_STOP;
            break;
    }
}

void app_mode_dec_reset(void)
{
    dec_state = DEC_INIT;
    dec_sample_count = 0;
    dec_last_emit = 0;
}