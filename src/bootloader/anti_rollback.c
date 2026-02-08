/**
 * @file anti_rollback.c
 * @brief Implementation of anti-rollback protection using OTP counters
 */

#include "../../include/anti_rollback.h"
#include <string.h>

/* OTP (One-Time Programmable) memory layout */
#define OTP_COUNTER_ADDR          OTP_COUNTER_BASE
#define OTP_LOCK_ADDR             (OTP_COUNTER_BASE + 0x100)
#define OTP_WRITE_ENABLE          0x4F545057  /* "OTPW" */

/* OTP status register */
#define OTP_STATUS_REG            (OTP_COUNTER_BASE + 0x200)
#define OTP_LOCKED                0x00000001

/**
 * @brief Write to OTP memory (one-time only)
 */
static bool otp_write(uint32_t address, uint32_t value) {
    volatile uint32_t *otp_ctrl = (volatile uint32_t *)(OTP_COUNTER_BASE + 0x300);
    volatile uint32_t *otp_addr = (volatile uint32_t *)(OTP_COUNTER_BASE + 0x304);
    volatile uint32_t *otp_data = (volatile uint32_t *)(OTP_COUNTER_BASE + 0x308);
    
    /* Check if OTP is locked */
    if (*((volatile uint32_t *)OTP_STATUS_REG) & OTP_LOCKED) {
        return false;
    }
    
    /* Enable OTP write */
    *otp_ctrl = OTP_WRITE_ENABLE;
    
    /* Set address and data */
    *otp_addr = address;
    *otp_data = value;
    
    /* Trigger write */
    *otp_ctrl |= 0x00000001;
    
    /* Wait for write to complete */
    uint32_t timeout = 100000;
    while (timeout--) {
        if ((*otp_ctrl & 0x00000002) == 0) {
            break;
        }
    }
    
    /* Disable write enable */
    *otp_ctrl = 0;
    
    return (timeout > 0);
}

/**
 * @brief Read from OTP memory
 */
static uint32_t otp_read(uint32_t address) {
    volatile uint32_t *otp_mem = (volatile uint32_t *)address;
    return *otp_mem;
}

/**
 * @brief Initialize OTP counter system
 */
bool otp_counter_init(void) {
    /* Verify OTP region is accessible */
    uint32_t test_read = otp_read(OTP_COUNTER_ADDR);
    
    /* If counter is 0xFFFFFFFF (erased state), initialize to 0 */
    if (test_read == 0xFFFFFFFF) {
        return otp_write(OTP_COUNTER_ADDR, 0);
    }
    
    return true;
}

/**
 * @brief Read current OTP counter value
 */
uint32_t otp_read_counter(void) {
    return otp_read(OTP_COUNTER_ADDR);
}

/**
 * @brief Increment OTP counter (irreversible)
 */
bool otp_increment_counter(void) {
    uint32_t current = otp_read_counter();
    
    /* Check for overflow */
    if (current >= MAX_ROLLBACK_COUNTER) {
        return false;
    }
    
    /* Increment counter */
    uint32_t new_value = current + 1;
    
    /* Write new value to OTP */
    return otp_write(OTP_COUNTER_ADDR, new_value);
}

/**
 * @brief Verify firmware version against rollback counter
 */
bool verify_no_rollback(uint32_t firmware_version) {
    uint32_t stored_version = otp_read_counter();
    
    /* Firmware version must be >= stored version */
    if (firmware_version < stored_version) {
        /* Rollback detected */
        return false;
    }
    
    /* If firmware version is newer, update counter */
    if (firmware_version > stored_version) {
        /* Calculate how many increments needed */
        uint32_t increments = firmware_version - stored_version;
        
        /* Perform increments (each is irreversible) */
        for (uint32_t i = 0; i < increments; i++) {
            if (!otp_increment_counter()) {
                return false;
            }
        }
    }
    
    return true;
}

/**
 * @brief Lock OTP counter (make immutable)
 */
bool otp_lock_counter(void) {
    /* Write lock bit to OTP lock register */
    bool result = otp_write(OTP_LOCK_ADDR, 0xDEADC0DE);
    
    if (result) {
        /* Set lock status bit */
        *((volatile uint32_t *)OTP_STATUS_REG) |= OTP_LOCKED;
    }
    
    return result;
}
