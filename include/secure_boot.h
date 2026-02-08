/**
 * @file secure_boot.h
 * @brief Hardened Secure Boot Architecture for EFR32MG26
 * 
 * Implements a comprehensive secure boot system with:
 * - Secure Vault High with immutable RTSL as Hardware Root of Trust
 * - Glitch-resistant control flow with layered token checks
 * - TRNG-based random jitter for timing attack mitigation
 * - ACMP/IADC tamper detection for voltage and temperature monitoring
 * - Anti-rollback protection using OTP counters
 * - Secure debug with certificate authentication
 * - Measured boot with signed JSON attestation
 * - TrustZone Secure World isolation
 * - PUF-based key wrapping
 */

#ifndef SECURE_BOOT_H
#define SECURE_BOOT_H

#include <stdint.h>
#include <stdbool.h>

/* Boot stage status codes */
#define BOOT_STATUS_SUCCESS           0x00000000
#define BOOT_STATUS_ERROR_GENERIC     0xFFFFFFFF
#define BOOT_STATUS_ERROR_TAMPER      0xFFFFFFFE
#define BOOT_STATUS_ERROR_ROLLBACK    0xFFFFFFFD
#define BOOT_STATUS_ERROR_SIGNATURE   0xFFFFFFFC
#define BOOT_STATUS_ERROR_GLITCH      0xFFFFFFFB

/* Control flow tokens for glitch resistance */
#define CF_TOKEN_INIT                 0xA5A5A5A5
#define CF_TOKEN_RTSL_VERIFIED        0x5A5A5A5A
#define CF_TOKEN_TAMPER_OK            0xC3C3C3C3
#define CF_TOKEN_ROLLBACK_OK          0x3C3C3C3C
#define CF_TOKEN_SIGNATURE_OK         0x69696969
#define CF_TOKEN_BOOT_COMPLETE        0x96969696

/* Boot configuration structure */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t rollback_counter;
    uint8_t  signature[64];
    uint32_t checksum;
} boot_config_t;

/* Attestation structure */
typedef struct {
    uint32_t boot_stage;
    uint32_t timestamp;
    uint32_t measurements[8];
    uint8_t  signature[64];
} boot_attestation_t;

/**
 * @brief Initialize secure boot system
 * @return Boot status code
 */
uint32_t secure_boot_init(void);

/**
 * @brief Verify Hardware Root of Trust (RTSL)
 * @return Boot status code
 */
uint32_t verify_root_of_trust(void);

/**
 * @brief Execute glitch-resistant control flow check
 * @param token Current control flow token
 * @param expected Expected token value
 * @return Boot status code
 */
uint32_t control_flow_check(uint32_t token, uint32_t expected);

/**
 * @brief Generate boot attestation
 * @param attestation Pointer to attestation structure
 * @return Boot status code
 */
uint32_t generate_attestation(boot_attestation_t *attestation);

#endif /* SECURE_BOOT_H */
