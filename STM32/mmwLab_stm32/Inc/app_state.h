/**
  ******************************************************************************
  * @file           : app_state.h
  * @brief          : Application state machine header
  ******************************************************************************
*/

#ifndef APP_STATE_H
#define APP_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ============================================================================
 * Forward Declarations - Mode Functions
 * ============================================================================ */

void app_mode_jog_update(void);
void app_mode_cal_update(void);
void app_mode_cap_update(void);
void app_mode_dec_update(void);

void app_mode_jog_reset(void);
void app_mode_cal_reset(void);
void app_mode_cap_reset(void);
void app_mode_dec_reset(void);

/* ============================================================================
 * Public Function Declarations
 * ============================================================================ */

void app_state_init(void);
void app_state_update(void);

uint8_t app_state_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_STATE_H */
