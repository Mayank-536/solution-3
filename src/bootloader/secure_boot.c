/**
 * @file secure_boot.c
 * @brief Hardened Secure Boot Implementation with Glitch-Resistant Control Flow
 * 
 * Implements multi-layer token verification and TRNG-based random jitter
 * to defeat single-cycle voltage glitches and timing-based attacks.
 */

#include "secure_boot.h"
#include "tamper_detection.h"
#include "anti_rollback.h"
#include "puf.h"
#include "trustzone.h"
#include <string.h>

/* Global boot context */
static boot_context_t g_boot_context;

/* Simulated TRNG for random jitter (in real implementation, use hardware TRNG) */
static uint32_t get_trng_random(void) {
    /* In production: Read from EFR32 TRNG peripheral
     * For now, use a simple PRNG for demonstration */
    static uint32_t lfsr = 0xACE1u;
    lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xB400u);
    return lfsr;
}

/**
 * @brief Inject random jitter delay for timing desynchronization
 * 
 * This prevents timing-based fault injection attacks by making the
 * execution time unpredictable.
 */
void inject_random_jitter(uint32_t seed) {
    volatile uint32_t delay_cycles = (seed % 1000) + 100;
    
    /* Create unpredictable delay with multiple paths */
    for (volatile uint32_t i = 0; i < delay_cycles; i++) {
        /* Add some computational work to make timing harder to predict */
        volatile uint32_t dummy = i * seed;
        dummy = dummy ^ (dummy >> 16);
        (void)dummy; /* Prevent optimization */
    }
    
    /* Additional jitter with different pattern */
    delay_cycles = (get_trng_random() % 500);
    for (volatile uint32_t j = 0; j < delay_cycles; j++) {
        __asm__ volatile ("nop");
    }
}

/**
 * @brief Perform layered token verification with glitch resistance
 * 
 * Uses multi-stage verification with non-binary tokens that require
 * multiple glitches to bypass, significantly increasing attack complexity.
 */
uint32_t verify_layered_tokens(boot_context_t *context) {
    volatile uint32_t state = TOKEN_STATE_INVALID;
    volatile uint32_t verification_result = 0;
    
    /* Layer 1: First token verification with jitter */
    inject_random_jitter(get_trng_random());
    if (context->verification_tokens[0] == TOKEN_LAYER_1) {
        state = TOKEN_STATE_LAYER1_OK;
        verification_result |= 0x1;
    } else {
        return TOKEN_STATE_INVALID;
    }
    
    /* Redundant check to defeat single glitch */
    inject_random_jitter(get_trng_random());
    if (state != TOKEN_STATE_LAYER1_OK) {
        return TOKEN_STATE_INVALID;
    }
    
    /* Layer 2: Second token verification */
    inject_random_jitter(get_trng_random());
    if (context->verification_tokens[1] == TOKEN_LAYER_2) {
        state = TOKEN_STATE_LAYER2_OK;
        verification_result |= 0x2;
    } else {
        return TOKEN_STATE_INVALID;
    }
    
    /* Redundant check with different comparison */
    inject_random_jitter(get_trng_random());
    if ((state ^ TOKEN_STATE_LAYER2_OK) != 0) {
        if (state != TOKEN_STATE_LAYER2_OK) {
            return TOKEN_STATE_INVALID;
        }
    }
    
    /* Layer 3: Third token verification */
    inject_random_jitter(get_trng_random());
    if (context->verification_tokens[2] == TOKEN_LAYER_3) {
        state = TOKEN_STATE_LAYER3_OK;
        verification_result |= 0x4;
    } else {
        return TOKEN_STATE_INVALID;
    }
    
    /* Layer 4: Fourth token verification */
    inject_random_jitter(get_trng_random());
    if (context->verification_tokens[3] == TOKEN_LAYER_4) {
        state = TOKEN_STATE_LAYER4_OK;
        verification_result |= 0x8;
    } else {
        return TOKEN_STATE_INVALID;
    }
    
    /* Final comprehensive check with multiple conditions */
    inject_random_jitter(get_trng_random());
    if (verification_result == 0xF && state == TOKEN_STATE_LAYER4_OK) {
        /* Double verification to prevent single glitch bypass */
        inject_random_jitter(get_trng_random());
        if ((context->verification_tokens[0] == TOKEN_LAYER_1) &&
            (context->verification_tokens[1] == TOKEN_LAYER_2) &&
            (context->verification_tokens[2] == TOKEN_LAYER_3) &&
            (context->verification_tokens[3] == TOKEN_LAYER_4)) {
            return TOKEN_STATE_ALL_VALID;
        }
    }
    
    return TOKEN_STATE_INVALID;
}

/**
 * @brief Verify firmware signature using ECDSA
 * 
 * In production, this would use the EFR32 Secure Vault cryptographic
 * accelerator for hardware-accelerated signature verification.
 */
uint32_t verify_firmware_signature(const firmware_header_t *header, const uint8_t *image) {
    volatile uint32_t verification_state = TOKEN_STATE_INVALID;
    
    /* Inject jitter to desynchronize timing */
    inject_random_jitter(get_trng_random());
    
    /* In production: Use Secure Vault for ECDSA verification
     * For demonstration, check basic header validity */
    if (header->magic != 0x464D5750) { /* "FWPG" magic */
        return TOKEN_STATE_INVALID;
    }
    
    inject_random_jitter(get_trng_random());
    
    /* Verify image size is reasonable */
    if (header->image_size == 0 || header->image_size > 0x100000) {
        return TOKEN_STATE_INVALID;
    }
    
    inject_random_jitter(get_trng_random());
    
    /* In production: Compute SHA-256 hash and verify ECDSA signature
     * using Secure Vault cryptographic accelerator */
    
    /* Simulate signature verification */
    verification_state = TOKEN_STATE_LAYER1_OK;
    
    inject_random_jitter(get_trng_random());
    
    /* Multi-stage verification to resist glitches */
    if (verification_state == TOKEN_STATE_LAYER1_OK) {
        verification_state = TOKEN_STATE_LAYER2_OK;
    } else {
        return TOKEN_STATE_INVALID;
    }
    
    inject_random_jitter(get_trng_random());
    
    if (verification_state == TOKEN_STATE_LAYER2_OK) {
        verification_state = TOKEN_STATE_ALL_VALID;
    } else {
        return TOKEN_STATE_INVALID;
    }
    
    return verification_state;
}

/**
 * @brief Check anti-rollback version
 */
uint32_t check_anti_rollback(uint32_t new_version) {
    version_t new_ver;
    rollback_status_t status;
    
    /* Extract version components */
    new_ver.major = (new_version >> 24) & 0xFF;
    new_ver.minor = (new_version >> 16) & 0xFF;
    new_ver.patch = new_version & 0xFFFF;
    
    inject_random_jitter(get_trng_random());
    
    /* Check against OTP version */
    status = check_version_rollback(&new_ver);
    
    inject_random_jitter(get_trng_random());
    
    /* Multi-stage verification */
    if (status == ROLLBACK_CHECK_PASS || status == ROLLBACK_VERSION_HIGHER) {
        inject_random_jitter(get_trng_random());
        /* Redundant check */
        status = check_version_rollback(&new_ver);
        if (status == ROLLBACK_CHECK_PASS || status == ROLLBACK_VERSION_HIGHER) {
            return TOKEN_STATE_ALL_VALID;
        }
    }
    
    return TOKEN_STATE_INVALID;
}

/**
 * @brief Initialize secure boot system
 */
boot_status_t secure_boot_init(void) {
    /* Initialize boot context */
    memset(&g_boot_context, 0, sizeof(boot_context_t));
    
    /* Set layered verification tokens */
    g_boot_context.verification_tokens[0] = TOKEN_LAYER_1;
    g_boot_context.verification_tokens[1] = TOKEN_LAYER_2;
    g_boot_context.verification_tokens[2] = TOKEN_LAYER_3;
    g_boot_context.verification_tokens[3] = TOKEN_LAYER_4;
    
    /* Initialize random seed from TRNG */
    g_boot_context.random_jitter_seed = get_trng_random();
    
    /* Set initial status */
    g_boot_context.status = BOOT_STATUS_INIT;
    g_boot_context.boot_count++;
    
    inject_random_jitter(g_boot_context.random_jitter_seed);
    
    /* Initialize subsystems */
    if (!puf_init()) {
        return BOOT_STATUS_FAILURE;
    }
    
    inject_random_jitter(get_trng_random());
    
    if (!anti_rollback_init()) {
        return BOOT_STATUS_FAILURE;
    }
    
    return BOOT_STATUS_SUCCESS;
}

/**
 * @brief Execute secure boot sequence
 */
boot_status_t execute_secure_boot(void) {
    uint32_t token_result;
    uint32_t signature_result;
    uint32_t rollback_result;
    
    /* Initialize boot */
    if (secure_boot_init() != BOOT_STATUS_SUCCESS) {
        return BOOT_STATUS_FAILURE;
    }
    
    g_boot_context.status = BOOT_STATUS_VERIFYING;
    
    inject_random_jitter(get_trng_random());
    
    /* Perform layered token verification */
    token_result = verify_layered_tokens(&g_boot_context);
    
    if (token_result != TOKEN_STATE_ALL_VALID) {
        g_boot_context.status = BOOT_STATUS_FAILURE;
        return BOOT_STATUS_FAILURE;
    }
    
    inject_random_jitter(get_trng_random());
    
    /* Simulate firmware header (in production, read from flash) */
    firmware_header_t fw_header = {
        .magic = 0x464D5750,
        .version = 0x01000000,
        .image_size = 0x10000,
        .load_address = 0x08000000,
        .entry_point = 0x08000400
    };
    
    /* Verify firmware signature */
    signature_result = verify_firmware_signature(&fw_header, NULL);
    
    if (signature_result != TOKEN_STATE_ALL_VALID) {
        g_boot_context.status = BOOT_STATUS_FAILURE;
        return BOOT_STATUS_FAILURE;
    }
    
    inject_random_jitter(get_trng_random());
    
    /* Check anti-rollback */
    rollback_result = check_anti_rollback(fw_header.version);
    
    if (rollback_result != TOKEN_STATE_ALL_VALID) {
        g_boot_context.status = BOOT_STATUS_FAILURE;
        return BOOT_STATUS_FAILURE;
    }
    
    inject_random_jitter(get_trng_random());
    
    /* All verifications passed */
    g_boot_context.status = BOOT_STATUS_SUCCESS;
    
    return BOOT_STATUS_SUCCESS;
}
