/**
 * @file secure_boot.c
 * @brief Implementation of hardened secure boot architecture
 */

#include "../include/secure_boot.h"
#include "../include/crypto_primitives.h"
#include "../include/tamper_detection.h"
#include "../include/anti_rollback.h"
#include "../include/attestation.h"
#include "../include/trustzone.h"
#include <string.h>

/* Secure Vault High registers (EFR32MG26 specific) */
#define SECURE_VAULT_BASE         0x4C021000
#define RTSL_STATUS_REG           (SECURE_VAULT_BASE + 0x000)
#define RTSL_LOCK_REG             (SECURE_VAULT_BASE + 0x004)
#define VAULT_STATUS_REG          (SECURE_VAULT_BASE + 0x008)

/* RTSL magic values for verification */
#define RTSL_MAGIC_IMMUTABLE      0x524F4F54  /* "ROOT" */
#define RTSL_LOCKED               0x4C4F434B  /* "LOCK" */

/* Global control flow token */
static volatile uint32_t g_control_flow_token = 0;

/**
 * @brief Read RTSL status register
 */
static inline uint32_t read_rtsl_status(void) {
    return *((volatile uint32_t *)RTSL_STATUS_REG);
}

/**
 * @brief Write to RTSL lock register (one-time programmable)
 */
static inline void lock_rtsl(void) {
    *((volatile uint32_t *)RTSL_LOCK_REG) = RTSL_LOCKED;
}

/**
 * @brief Verify Hardware Root of Trust (RTSL)
 */
uint32_t verify_root_of_trust(void) {
    uint32_t status;
    
    /* Add random jitter to resist timing attacks */
    trng_random_jitter(100, 500);
    
    /* Read RTSL status */
    status = read_rtsl_status();
    
    /* Verify RTSL is properly configured and immutable */
    if (status != RTSL_MAGIC_IMMUTABLE) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Add another jitter point */
    trng_random_jitter(50, 200);
    
    /* Update control flow token */
    g_control_flow_token = CF_TOKEN_RTSL_VERIFIED;
    
    /* Verify token was set correctly (glitch resistance) */
    if (g_control_flow_token != CF_TOKEN_RTSL_VERIFIED) {
        return BOOT_STATUS_ERROR_GLITCH;
    }
    
    /* Lock RTSL to make it immutable */
    lock_rtsl();
    
    return BOOT_STATUS_SUCCESS;
}

/**
 * @brief Execute glitch-resistant control flow check
 */
uint32_t control_flow_check(uint32_t token, uint32_t expected) {
    volatile uint32_t check1, check2, check3;
    
    /* Triple redundant check for glitch resistance */
    check1 = (token == expected);
    trng_random_jitter(10, 50);
    check2 = (token == expected);
    trng_random_jitter(10, 50);
    check3 = (token == expected);
    
    /* All three checks must pass */
    if (!(check1 && check2 && check3)) {
        /* Glitch detected - trigger tamper response */
        handle_tamper_event(TAMPER_GLITCH_DETECTED);
        return BOOT_STATUS_ERROR_GLITCH;
    }
    
    /* Additional verification with inverted logic */
    if ((token != expected) || (check1 == 0) || (check2 == 0) || (check3 == 0)) {
        handle_tamper_event(TAMPER_GLITCH_DETECTED);
        return BOOT_STATUS_ERROR_GLITCH;
    }
    
    return BOOT_STATUS_SUCCESS;
}

/**
 * @brief Initialize secure boot system
 */
uint32_t secure_boot_init(void) {
    uint32_t status;
    
    /* Initialize control flow token */
    g_control_flow_token = CF_TOKEN_INIT;
    
    /* Verify initial token */
    status = control_flow_check(g_control_flow_token, CF_TOKEN_INIT);
    if (status != BOOT_STATUS_SUCCESS) {
        return status;
    }
    
    /* Initialize TrustZone Secure World */
    if (!trustzone_init()) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Initialize TRNG for random jitter */
    if (!trng_init()) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Initialize PUF for key wrapping */
    if (!puf_init()) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Initialize tamper detection */
    if (!tamper_detection_init()) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Initialize OTP counter for anti-rollback */
    if (!otp_counter_init()) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Initialize attestation system */
    if (!attestation_init()) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Verify Hardware Root of Trust */
    status = verify_root_of_trust();
    if (status != BOOT_STATUS_SUCCESS) {
        return status;
    }
    
    /* Check control flow token */
    status = control_flow_check(g_control_flow_token, CF_TOKEN_RTSL_VERIFIED);
    if (status != BOOT_STATUS_SUCCESS) {
        return status;
    }
    
    /* Check for tamper events */
    uint32_t tamper = check_tamper_events();
    if (tamper != TAMPER_NONE) {
        handle_tamper_event(tamper);
        g_control_flow_token = 0;
        return BOOT_STATUS_ERROR_TAMPER;
    }
    
    g_control_flow_token = CF_TOKEN_TAMPER_OK;
    status = control_flow_check(g_control_flow_token, CF_TOKEN_TAMPER_OK);
    if (status != BOOT_STATUS_SUCCESS) {
        return status;
    }
    
    /* Enable continuous tamper monitoring */
    enable_tamper_monitoring();
    
    return BOOT_STATUS_SUCCESS;
}

/**
 * @brief Generate boot attestation
 */
uint32_t generate_attestation(boot_attestation_t *attestation) {
    if (!attestation) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* Clear attestation structure */
    memset(attestation, 0, sizeof(boot_attestation_t));
    
    /* Record boot stage measurements */
    uint8_t measurement[32];
    for (uint32_t stage = 0; stage < 8; stage++) {
        if (get_measurement(stage, measurement)) {
            /* Store first 4 bytes of each measurement */
            attestation->measurements[stage] = 
                (measurement[0] << 24) | (measurement[1] << 16) |
                (measurement[2] << 8) | measurement[3];
        }
    }
    
    /* Set boot stage and timestamp */
    attestation->boot_stage = g_control_flow_token;
    attestation->timestamp = 0; /* Would be real time in actual implementation */
    
    /* Sign attestation */
    uint8_t puf_key[PUF_KEY_SIZE];
    if (!puf_derive_key(puf_key, PUF_KEY_SIZE)) {
        return BOOT_STATUS_ERROR_GENERIC;
    }
    
    /* In real implementation, this would use the PUF key to sign */
    memcpy(attestation->signature, puf_key, 32);
    
    return BOOT_STATUS_SUCCESS;
}
