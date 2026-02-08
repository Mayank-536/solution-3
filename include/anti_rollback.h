/**
 * @file anti_rollback.h
 * @brief Anti-Rollback Protection using OTP Counters
 * 
 * Implements version control and OTP counter management to prevent
 * firmware downgrade attacks.
 */

#ifndef ANTI_ROLLBACK_H
#define ANTI_ROLLBACK_H

#include <stdint.h>
#include <stdbool.h>

/* OTP Counter Configuration */
#define OTP_COUNTER_BASE_ADDRESS    0x0FE00000  /* EFR32 OTP base */
#define OTP_VERSION_COUNTER_OFFSET  0x100       /* Version counter offset */
#define MAX_OTP_COUNTERS            8           /* Number of OTP counters */

/* Version Structure */
typedef struct {
    uint8_t major;
    uint8_t minor;
    uint16_t patch;
} version_t;

/* Anti-Rollback Status */
typedef enum {
    ROLLBACK_CHECK_PASS = 0xAA55AA55,
    ROLLBACK_CHECK_FAIL = 0x55AA55AA,
    ROLLBACK_VERSION_EQUAL = 0x33CC33CC,
    ROLLBACK_VERSION_HIGHER = 0xCC3333CC
} rollback_status_t;

/**
 * @brief Initialize anti-rollback system
 * @return true if initialization successful
 */
bool anti_rollback_init(void);

/**
 * @brief Read current version from OTP
 * @param version Pointer to version structure
 * @return true if read successful
 */
bool read_otp_version(version_t *version);

/**
 * @brief Write new version to OTP (one-time programmable)
 * @param version Pointer to version structure
 * @return true if write successful
 */
bool write_otp_version(const version_t *version);

/**
 * @brief Compare firmware version against OTP
 * @param new_version Version to check
 * @return rollback_status_t Comparison result
 */
rollback_status_t check_version_rollback(const version_t *new_version);

/**
 * @brief Increment OTP counter (monotonic)
 * @param counter_index Counter index (0-7)
 * @return true if increment successful
 */
bool increment_otp_counter(uint32_t counter_index);

/**
 * @brief Read OTP counter value
 * @param counter_index Counter index (0-7)
 * @param value Pointer to receive counter value
 * @return true if read successful
 */
bool read_otp_counter(uint32_t counter_index, uint32_t *value);

/**
 * @brief Verify firmware version meets anti-rollback requirements
 * @param firmware_version Version from firmware header
 * @return rollback_status_t Verification result
 */
rollback_status_t verify_firmware_version(uint32_t firmware_version);

#endif /* ANTI_ROLLBACK_H */
