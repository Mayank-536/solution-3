/**
 * @file tamper_detection.h
 * @brief ACMP/IADC-based tamper detection system
 * 
 * Implements voltage and temperature monitoring with preemptive responses
 */

#ifndef TAMPER_DETECTION_H
#define TAMPER_DETECTION_H

#include <stdint.h>
#include <stdbool.h>

/* Tamper detection thresholds */
#define VOLTAGE_MIN_MV            1700
#define VOLTAGE_MAX_MV            2000
#define TEMP_MIN_C                -40
#define TEMP_MAX_C                85

/* Tamper event types */
#define TAMPER_NONE               0x00
#define TAMPER_VOLTAGE_LOW        0x01
#define TAMPER_VOLTAGE_HIGH       0x02
#define TAMPER_TEMP_LOW           0x04
#define TAMPER_TEMP_HIGH          0x08
#define TAMPER_GLITCH_DETECTED    0x10

/**
 * @brief Initialize tamper detection system
 * @return true on success, false on failure
 */
bool tamper_detection_init(void);

/**
 * @brief Initialize ACMP for voltage monitoring
 * @return true on success, false on failure
 */
bool acmp_init(void);

/**
 * @brief Initialize IADC for temperature monitoring
 * @return true on success, false on failure
 */
bool iadc_init(void);

/**
 * @brief Check for tamper events
 * @return Bitmask of detected tamper events
 */
uint32_t check_tamper_events(void);

/**
 * @brief Handle tamper event with preemptive response
 * @param tamper_events Bitmask of tamper events
 */
void handle_tamper_event(uint32_t tamper_events);

/**
 * @brief Read current supply voltage
 * @return Voltage in millivolts
 */
uint32_t read_supply_voltage(void);

/**
 * @brief Read current temperature
 * @return Temperature in Celsius
 */
int32_t read_temperature(void);

/**
 * @brief Enable continuous tamper monitoring
 */
void enable_tamper_monitoring(void);

/**
 * @brief Disable tamper monitoring
 */
void disable_tamper_monitoring(void);

#endif /* TAMPER_DETECTION_H */
