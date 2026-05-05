/**
  ******************************************************************************
  * @file           : cfg_sys.h
  * @brief          : System configuration information
  ******************************************************************************
*/

#ifndef CFG_SYS_H
#define CFG_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ADC & Sampling Configuration
 * ============================================================================ */
#define SAMPLE_SIZE         1024            // Number of samples per acquisition
#define ADC_DELAY           100             // Delay between ADC reads (ms)
#define ADC_TIMEOUT         1000            // ADC operation timeout (ms)

/* ============================================================================
 * Motor Control Configuration
 * ============================================================================ */
#define NEMA23_STEPS_PER_REV  1600          // NEMA23 with microstepping
#define GEAR_RATIO          1.0             // Mechanical gear ratio (if applicable)
#define MICROSTEP_FACTOR    16              // Microstepping level
#define STEPS_PER_DEGREE    (NEMA23_STEPS_PER_REV / 360)

#define AZIMUTH_INCREMENT   5               // 5 degree steps
#define ROTATE_DELAY        100             // Rotation step delay (ms)
#define MOTOR_ACCEL_TIME    500             // Acceleration ramp time (ms)
#define MOTOR_MAX_SPEED     200             // Max stepper frequency (steps/sec)

/* ============================================================================
 * State Machine Timeouts
 * ============================================================================ */
#define CAL_TIMEOUT         5000            // Calibration timeout (ms)
#define CAP_TIMEOUT         10000           // Capture timeout (ms)
#define DEC_TIMEOUT         30000           // Decode timeout (ms)
#define JOG_TIMEOUT         3000            // Jog timeout (ms)

/* ============================================================================
 * Communication Configuration
 * ============================================================================ */
#define BAUDRATE            9600            // UART baud rate
#define UART_RX_BUFFER_SIZE 256             // RX buffer size
#define UART_TX_BUFFER_SIZE 256             // TX buffer size
#define CMD_TIMEOUT         1000            // Command response timeout (ms)

/* ============================================================================
 * System Timing
 * ============================================================================ */
#define TICK_RATE_MS        10              // System tick rate (milliseconds)
#define TICK_RATE_HZ        (1000 / TICK_RATE_MS)

#ifdef __cplusplus
}
#endif

#endif /* CFG_SYS_H */

