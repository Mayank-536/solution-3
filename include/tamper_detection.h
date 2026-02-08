/**
 * @file tamper_detection.h
 * @brief Active Tamper Detection using ACMP/IADC peripherals for EFR32MG26
 * 
 * Implements preemptive anti-tamper monitoring for voltage and temperature
 * anomalies with immediate response capabilities.
 */

#ifndef TAMPER_DETECTION_H
#define TAMPER_DETECTION_H

#include <stdint.h>
#include <stdbool.h>

/* Tamper Event Types */
#define TAMPER_EVENT_NONE           0x00000000
#define TAMPER_EVENT_VOLTAGE_LOW    0x00000001
#define TAMPER_EVENT_VOLTAGE_HIGH   0x00000002
#define TAMPER_EVENT_TEMP_LOW       0x00000004
#define TAMPER_EVENT_TEMP_HIGH      0x00000008
#define TAMPER_EVENT_GLITCH         0x00000010
#define TAMPER_EVENT_CLOCK_ANOMALY  0x00000020

/* Voltage Thresholds (in millivolts) */
#define VOLTAGE_THRESHOLD_LOW_MV    2700
#define VOLTAGE_THRESHOLD_HIGH_MV   3600
#define VOLTAGE_NOMINAL_MV          3300

/* Temperature Thresholds (in Celsius) */
#define TEMP_THRESHOLD_LOW_C        -20
#define TEMP_THRESHOLD_HIGH_C       85
#define TEMP_NOMINAL_C              25

/* ACMP Configuration Structure */
typedef struct {
    uint32_t low_threshold;      /* Low voltage threshold */
    uint32_t high_threshold;     /* High voltage threshold */
    uint32_t hysteresis;         /* Hysteresis value */
    bool interrupt_enabled;      /* Enable interrupt on detection */
} acmp_config_t;

/* IADC Configuration Structure */
typedef struct {
    uint32_t sample_rate;        /* ADC sample rate */
    uint32_t temp_low_threshold; /* Low temperature threshold */
    uint32_t temp_high_threshold;/* High temperature threshold */
    bool continuous_mode;        /* Continuous monitoring mode */
} iadc_config_t;

/* Tamper Context */
typedef struct {
    uint32_t event_flags;        /* Bitmap of tamper events */
    uint32_t event_count;        /* Total tamper events detected */
    uint32_t last_voltage_mv;    /* Last measured voltage */
    int32_t last_temp_c;         /* Last measured temperature */
    uint64_t last_event_time;    /* Timestamp of last event */
} tamper_context_t;

/**
 * @brief Initialize ACMP peripheral for voltage monitoring
 * @param config ACMP configuration parameters
 * @return true if initialization successful
 */
bool acmp_init(const acmp_config_t *config);

/**
 * @brief Initialize IADC peripheral for temperature monitoring
 * @param config IADC configuration parameters
 * @return true if initialization successful
 */
bool iadc_init(const iadc_config_t *config);

/**
 * @brief Start tamper detection monitoring
 * @param context Pointer to tamper context
 * @return true if monitoring started successfully
 */
bool tamper_detection_start(tamper_context_t *context);

/**
 * @brief Check for tamper events
 * @param context Pointer to tamper context
 * @return uint32_t Bitmap of detected tamper events
 */
uint32_t check_tamper_events(tamper_context_t *context);

/**
 * @brief Execute anti-tamper response
 * @param event_flags Bitmap of tamper events
 */
void execute_tamper_response(uint32_t event_flags);

/**
 * @brief ACMP interrupt handler for voltage glitch detection
 */
void acmp_irq_handler(void);

/**
 * @brief IADC interrupt handler for temperature anomaly detection
 */
void iadc_irq_handler(void);

#endif /* TAMPER_DETECTION_H */
