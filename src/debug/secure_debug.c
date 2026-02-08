/**
 * @file secure_debug.c
 * @brief Implementation of secure debug with certificate authentication
 */

#include "../../include/secure_debug.h"
#include "../../include/crypto_primitives.h"
#include <string.h>

/* Debug port control registers */
#define DEBUG_CTRL_BASE           0x4C025000
#define DEBUG_AUTH_REG            (DEBUG_CTRL_BASE + 0x000)
#define DEBUG_LOCK_REG            (DEBUG_CTRL_BASE + 0x004)
#define DEBUG_STATUS_REG          (DEBUG_CTRL_BASE + 0x008)

/* Debug authentication magic */
#define DEBUG_MAGIC               0x44454247  /* "DEBG" */
#define DEBUG_UNLOCK_KEY          0x554E4C4B  /* "UNLK" */

/* Public key for debug certificate verification (stored in secure vault) */
static const uint8_t debug_public_key[64] = {
    0x04, 0x8B, 0xF2, 0x30, 0x5C, 0x9E, 0x73, 0x42,
    0xA1, 0xD9, 0x6F, 0x84, 0x2C, 0x5E, 0xB3, 0x71,
    0xF9, 0x45, 0x2A, 0xC6, 0x89, 0x1D, 0x7E, 0x23,
    0x5B, 0x91, 0x3F, 0xD4, 0x6C, 0xA8, 0x54, 0x1E,
    0x2F, 0xB7, 0x93, 0xC5, 0x61, 0x8A, 0x4D, 0x27,
    0xE9, 0x0C, 0x75, 0x3B, 0xA2, 0x68, 0x1F, 0x94,
    0x5D, 0xC3, 0x86, 0x2E, 0xB1, 0x79, 0x4A, 0x0F,
    0xD5, 0x37, 0x9C, 0x62, 0xAE, 0x18, 0x83, 0x4B
};

/* Current debug state */
static volatile uint32_t g_debug_status = DEBUG_AUTH_LOCKED;

/**
 * @brief Initialize secure debug interface
 */
bool secure_debug_init(void) {
    /* Lock debug port by default */
    *((volatile uint32_t *)DEBUG_LOCK_REG) = 0xFFFFFFFF;
    
    /* Set initial status */
    g_debug_status = DEBUG_AUTH_LOCKED;
    *((volatile uint32_t *)DEBUG_STATUS_REG) = DEBUG_AUTH_LOCKED;
    
    return true;
}

/**
 * @brief Read device unique ID
 */
static void read_device_id(uint8_t device_id[16]) {
    /* Read device unique ID from chip (EFR32MG26 specific) */
    volatile uint32_t *unique_id = (volatile uint32_t *)0x0FE081F0;
    
    for (int i = 0; i < 4; i++) {
        uint32_t word = unique_id[i];
        device_id[i * 4 + 0] = (word >> 0) & 0xFF;
        device_id[i * 4 + 1] = (word >> 8) & 0xFF;
        device_id[i * 4 + 2] = (word >> 16) & 0xFF;
        device_id[i * 4 + 3] = (word >> 24) & 0xFF;
    }
}

/**
 * @brief Get current timestamp (simplified)
 */
static uint32_t get_current_time(void) {
    /* In real implementation, this would read RTC */
    /* For demonstration, return a fixed value */
    return 0x12345678;
}

/**
 * @brief Authenticate debug certificate
 */
bool authenticate_debug_cert(const debug_certificate_t *cert) {
    if (!cert) {
        return false;
    }
    
    /* Verify certificate magic */
    if (cert->magic != DEBUG_MAGIC) {
        return false;
    }
    
    /* Verify certificate is for this device */
    uint8_t device_id[16];
    read_device_id(device_id);
    
    if (memcmp(cert->device_id, device_id, 16) != 0) {
        return false;
    }
    
    /* Check certificate expiration */
    uint32_t current_time = get_current_time();
    if (current_time > cert->expiration_time) {
        return false;
    }
    
    /* Verify certificate signature */
    uint8_t cert_data[sizeof(debug_certificate_t) - SIGNATURE_SIZE];
    memcpy(cert_data, cert, sizeof(cert_data));
    
    if (!verify_signature(cert_data, sizeof(cert_data), 
                         cert->signature, debug_public_key)) {
        return false;
    }
    
    return true;
}

/**
 * @brief Enable debug access with certificate
 */
bool enable_debug_access(const debug_certificate_t *cert) {
    /* Authenticate certificate */
    if (!authenticate_debug_cert(cert)) {
        return false;
    }
    
    /* Unlock debug port */
    *((volatile uint32_t *)DEBUG_AUTH_REG) = DEBUG_UNLOCK_KEY;
    *((volatile uint32_t *)DEBUG_LOCK_REG) = 0x00000000;
    
    /* Update status */
    if (cert->expiration_time > 0) {
        g_debug_status = DEBUG_AUTH_TEMPORARY;
    } else {
        g_debug_status = DEBUG_AUTH_UNLOCKED;
    }
    
    *((volatile uint32_t *)DEBUG_STATUS_REG) = g_debug_status;
    
    return true;
}

/**
 * @brief Disable debug access
 */
void disable_debug_access(void) {
    /* Lock debug port */
    *((volatile uint32_t *)DEBUG_LOCK_REG) = 0xFFFFFFFF;
    *((volatile uint32_t *)DEBUG_AUTH_REG) = 0x00000000;
    
    /* Update status */
    g_debug_status = DEBUG_AUTH_LOCKED;
    *((volatile uint32_t *)DEBUG_STATUS_REG) = DEBUG_AUTH_LOCKED;
}

/**
 * @brief Get current debug authentication status
 */
uint32_t get_debug_status(void) {
    return g_debug_status;
}
