/**
 * @file anti_rollback.h
 * @brief Anti-rollback protection using OTP counters
 */

#ifndef ANTI_ROLLBACK_H
#define ANTI_ROLLBACK_H

#include <stdint.h>
#include <stdbool.h>

/* OTP counter configuration */
#define OTP_COUNTER_BASE          0x0FE00000
#define OTP_COUNTER_SIZE          32
#define MAX_ROLLBACK_COUNTER      0xFFFFFFFF

/**
 * @brief Initialize OTP counter system
 * @return true on success, false on failure
 */
bool otp_counter_init(void);

/**
 * @brief Read current OTP counter value
 * @return Current counter value
 */
uint32_t otp_read_counter(void);

/**
 * @brief Increment OTP counter (irreversible)
 * @return true on success, false on failure
 */
bool otp_increment_counter(void);

/**
 * @brief Verify firmware version against rollback counter
 * @param firmware_version Version to check
 * @return true if version is valid, false if rollback detected
 */
bool verify_no_rollback(uint32_t firmware_version);

/**
 * @brief Lock OTP counter (make immutable)
 * @return true on success, false on failure
 */
bool otp_lock_counter(void);

#endif /* ANTI_ROLLBACK_H */
