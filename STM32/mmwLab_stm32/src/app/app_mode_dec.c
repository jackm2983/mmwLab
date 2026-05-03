/**
  ******************************************************************************
  * @file           : app_mode_dec.c
  * @brief          : Decode I and Q samples
  ******************************************************************************
*/

#include "main.h"
#include "cfg_sys.h"
#include "drv_cmd.h"
#include "bsp_adc.h"

typedef enum {
    DEC_INIT,
    DEC_STREAMING,
    DEC_STOP
} DecState_t;

static DecState_t dec_state = DEC_INIT;
static uint32_t dec_sample_count = 0;

void app_mode_dec_update(void)
{
    switch (dec_state)
    {
        case DEC_INIT:
            drv_cmd_send_status("DEC: Starting ADC stream");
            dec_sample_count = 0;
            bsp_adc_sampling_reset();
            bsp_adc_start_continuous();
            dec_state = DEC_STREAMING;
            break;

        case DEC_STREAMING:
            if (bsp_adc_sampling_complete())
            {
                drv_cmd_send_status("DEC: Sample buffer full");
                dec_state = DEC_STOP;
            }
            break;

        case DEC_STOP:
            bsp_adc_stop();
            drv_cmd_send_status("DEC: Stream complete");
            break;

        default:
            break;
    }
}

void app_mode_dec_reset(void)
{
    dec_state = DEC_INIT;
    dec_sample_count = 0;
} 

