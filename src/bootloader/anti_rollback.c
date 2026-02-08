/**
 * @file anti_rollback.c
 * @brief Anti-Rollback Protection Implementation
 * 
 * Implements OTP counter management and version verification to prevent
 * firmware downgrade attacks.
 */

#include "anti_rollback.h"
#include <string.h>

/* Simulated OTP storage (in production, use actual EFR32 OTP) */
static uint32_t g_otp_counters[MAX_OTP_COUNTERS];
static version_t g_otp_version;
static bool g_anti_rollback_initialized = false;

/**
 * @brief Initialize anti-rollback system
 */
bool anti_rollback_init(void) {
    if (g_anti_rollback_initialized) {
        return true;
    }
    
    /* In production: Read OTP counters and version from hardware
     * Reference: EFR32 OTP Memory Map
     */
    
    /* Initialize simulated OTP storage */
    for (int i = 0; i < MAX_OTP_COUNTERS; i++) {
        g_otp_counters[i] = 0;
    }
    
    /* Read current version from OTP */
    g_otp_version.major = FIRMWARE_VERSION_MAJOR;
    g_otp_version.minor = FIRMWARE_VERSION_MINOR;
    g_otp_version.patch = FIRMWARE_VERSION_PATCH;
    
    g_anti_rollback_initialized = true;
    
    return true;
}

/**
 * @brief Read current version from OTP
 */
bool read_otp_version(version_t *version) {
    if (!g_anti_rollback_initialized || version == NULL) {
        return false;
    }
    
    /* In production: Read from actual OTP memory
     * uint32_t *otp_version = (uint32_t *)(OTP_COUNTER_BASE_ADDRESS + OTP_VERSION_COUNTER_OFFSET);
     * version->major = (*otp_version >> 24) & 0xFF;
     * version->minor = (*otp_version >> 16) & 0xFF;
     * version->patch = *otp_version & 0xFFFF;
     */
    
    memcpy(version, &g_otp_version, sizeof(version_t));
    
    return true;
}

/**
 * @brief Write new version to OTP (one-time programmable)
 */
bool write_otp_version(const version_t *version) {
    if (!g_anti_rollback_initialized || version == NULL) {
        return false;
    }
    
    /* Verify new version is higher than current */
    version_t current_version;
    if (!read_otp_version(&current_version)) {
        return false;
    }
    
    /* Compare versions */
    if (version->major < current_version.major) {
        return false;  /* Downgrade not allowed */
    }
    
    if (version->major == current_version.major &&
        version->minor < current_version.minor) {
        return false;  /* Downgrade not allowed */
    }
    
    if (version->major == current_version.major &&
        version->minor == current_version.minor &&
        version->patch <= current_version.patch) {
        return false;  /* Downgrade or same version not allowed */
    }
    
    /* In production: Write to actual OTP memory (ONE-TIME OPERATION!)
     * WARNING: OTP can only be programmed once!
     * 
     * uint32_t version_word = (version->major << 24) | 
     *                         (version->minor << 16) | 
     *                         version->patch;
     * write_otp_word(OTP_COUNTER_BASE_ADDRESS + OTP_VERSION_COUNTER_OFFSET, version_word);
     */
    
    /* Update simulated OTP */
    memcpy(&g_otp_version, version, sizeof(version_t));
    
    return true;
}

/**
 * @brief Compare firmware version against OTP
 */
rollback_status_t check_version_rollback(const version_t *new_version) {
    if (!g_anti_rollback_initialized || new_version == NULL) {
        return ROLLBACK_CHECK_FAIL;
    }
    
    version_t current_version;
    if (!read_otp_version(&current_version)) {
        return ROLLBACK_CHECK_FAIL;
    }
    
    /* Compare major version */
    if (new_version->major > current_version.major) {
        return ROLLBACK_VERSION_HIGHER;
    }
    
    if (new_version->major < current_version.major) {
        return ROLLBACK_CHECK_FAIL;  /* Downgrade detected */
    }
    
    /* Major versions equal, compare minor */
    if (new_version->minor > current_version.minor) {
        return ROLLBACK_VERSION_HIGHER;
    }
    
    if (new_version->minor < current_version.minor) {
        return ROLLBACK_CHECK_FAIL;  /* Downgrade detected */
    }
    
    /* Major and minor equal, compare patch */
    if (new_version->patch > current_version.patch) {
        return ROLLBACK_VERSION_HIGHER;
    }
    
    if (new_version->patch < current_version.patch) {
        return ROLLBACK_CHECK_FAIL;  /* Downgrade detected */
    }
    
    /* Versions are equal */
    return ROLLBACK_VERSION_EQUAL;
}

/**
 * @brief Increment OTP counter (monotonic)
 */
bool increment_otp_counter(uint32_t counter_index) {
    if (!g_anti_rollback_initialized || counter_index >= MAX_OTP_COUNTERS) {
        return false;
    }
    
    /* In production: Increment actual OTP counter
     * OTP counters are implemented as chains of bits that can only transition 0->1
     * Incrementing means programming the next bit in the chain
     * 
     * Reference: EFR32 OTP Counter Implementation
     */
    
    /* Simulated increment */
    if (g_otp_counters[counter_index] < 0xFFFFFFFF) {
        g_otp_counters[counter_index]++;
        return true;
    }
    
    return false;  /* Counter overflow */
}

/**
 * @brief Read OTP counter value
 */
bool read_otp_counter(uint32_t counter_index, uint32_t *value) {
    if (!g_anti_rollback_initialized || counter_index >= MAX_OTP_COUNTERS || value == NULL) {
        return false;
    }
    
    /* In production: Read from actual OTP memory
     * uint32_t *otp_counter = (uint32_t *)(OTP_COUNTER_BASE_ADDRESS + (counter_index * 4));
     * *value = popcount(*otp_counter);  // Count number of 1 bits
     */
    
    *value = g_otp_counters[counter_index];
    
    return true;
}

/**
 * @brief Verify firmware version meets anti-rollback requirements
 */
rollback_status_t verify_firmware_version(uint32_t firmware_version) {
    if (!g_anti_rollback_initialized) {
        return ROLLBACK_CHECK_FAIL;
    }
    
    /* Extract version components from packed format */
    version_t new_version;
    new_version.major = (firmware_version >> 24) & 0xFF;
    new_version.minor = (firmware_version >> 16) & 0xFF;
    new_version.patch = firmware_version & 0xFFFF;
    
    /* Check against OTP version */
    rollback_status_t status = check_version_rollback(&new_version);
    
    /* Only allow equal or higher versions */
    if (status == ROLLBACK_CHECK_FAIL) {
        return ROLLBACK_CHECK_FAIL;
    }
    
    return ROLLBACK_CHECK_PASS;
}
