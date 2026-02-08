/**
 * @file secure_debug.h
 * @brief Secure debug interface with certificate authentication
 */

#ifndef SECURE_DEBUG_H
#define SECURE_DEBUG_H

#include <stdint.h>
#include <stdbool.h>

/* Debug authentication status */
#define DEBUG_AUTH_LOCKED         0x00
#define DEBUG_AUTH_UNLOCKED       0x01
#define DEBUG_AUTH_TEMPORARY      0x02

/* Debug certificate structure */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint8_t  device_id[16];
    uint32_t permissions;
    uint32_t expiration_time;
    uint8_t  signature[64];
} debug_certificate_t;

/**
 * @brief Initialize secure debug interface
 * @return true on success, false on failure
 */
bool secure_debug_init(void);

/**
 * @brief Authenticate debug certificate
 * @param cert Debug certificate
 * @return true if certificate is valid, false otherwise
 */
bool authenticate_debug_cert(const debug_certificate_t *cert);

/**
 * @brief Enable debug access with certificate
 * @param cert Debug certificate
 * @return true on success, false on failure
 */
bool enable_debug_access(const debug_certificate_t *cert);

/**
 * @brief Disable debug access
 */
void disable_debug_access(void);

/**
 * @brief Get current debug authentication status
 * @return Debug authentication status
 */
uint32_t get_debug_status(void);

#endif /* SECURE_DEBUG_H */
