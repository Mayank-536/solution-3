/**
 * @file puf.c
 * @brief Physically Unclonable Function (PUF) Implementation
 * 
 * Implements PUF-based key derivation and wrapping using the EFR32MG26
 * Secure Vault to protect against memory dump extraction.
 */

#include "puf.h"
#include <string.h>

/* PUF state */
static puf_config_t g_puf_config;
static bool g_puf_initialized = false;
static uint8_t g_puf_key[PUF_KEY_SIZE];

/* Simulated PUF helper data (in production, stored in OTP) */
static uint8_t g_puf_helper_data[64];

/**
 * @brief Secure memory zeroization
 */
void secure_zeroize(uint8_t *key, uint32_t size) {
    /* Use volatile to prevent compiler optimization */
    volatile uint8_t *p = key;
    
    while (size--) {
        *p++ = 0;
    }
    
    /* Memory barrier to ensure writes complete */
    __asm__ volatile ("" ::: "memory");
}

/**
 * @brief Initialize PUF subsystem
 */
bool puf_init(void) {
    if (g_puf_initialized) {
        return true;
    }
    
    /* Initialize PUF configuration */
    memset(&g_puf_config, 0, sizeof(puf_config_t));
    
    /* In production: Check if PUF has been enrolled
     * Read enrollment status from OTP or secure storage
     */
    g_puf_config.enrollment_done = 0;  /* Assume not enrolled */
    g_puf_config.reconstruction_count = 0;
    g_puf_config.error_threshold = 5;  /* 5-bit error correction */
    
    /* In production: Initialize Secure Vault PUF peripheral
     * Reference: EFR32 Secure Vault User Guide
     */
    
    /* Enable Secure Vault clock */
    /* CMU->CLKEN1 |= CMU_CLKEN1_SEMAILBOX; */
    
    /* Initialize SE (Secure Element) mailbox for PUF operations */
    /* SE_executeCommand(&cmd); */
    
    g_puf_initialized = true;
    
    return true;
}

/**
 * @brief Enroll PUF (first-time initialization)
 */
bool puf_enroll(void) {
    if (!g_puf_initialized) {
        return false;
    }
    
    if (g_puf_config.enrollment_done) {
        return true;  /* Already enrolled */
    }
    
    /* In production: Use Secure Vault to perform PUF enrollment
     * 1. Activate PUF circuitry
     * 2. Extract raw PUF response
     * 3. Apply fuzzy extractor / error correction
     * 4. Generate helper data for reconstruction
     * 5. Store helper data in OTP
     */
    
    /* Simulated enrollment - generate placeholder helper data */
    for (int i = 0; i < sizeof(g_puf_helper_data); i++) {
        g_puf_helper_data[i] = (uint8_t)(i ^ 0xA5);
    }
    
    /* Mark enrollment as complete */
    g_puf_config.enrollment_done = 1;
    
    /* In production: Write enrollment status to OTP */
    /* write_otp_value(OTP_PUF_ENROLLED_ADDR, 1); */
    
    return true;
}

/**
 * @brief Reconstruct PUF key
 */
bool puf_reconstruct_key(uint8_t *key_output, uint32_t key_size) {
    if (!g_puf_initialized || key_output == NULL) {
        return false;
    }
    
    if (key_size != PUF_KEY_SIZE) {
        return false;
    }
    
    if (!g_puf_config.enrollment_done) {
        return false;  /* PUF not enrolled */
    }
    
    /* In production: Use Secure Vault to reconstruct PUF key
     * 1. Read helper data from OTP
     * 2. Activate PUF circuitry
     * 3. Extract raw PUF response
     * 4. Apply error correction using helper data
     * 5. Reconstruct stable key
     */
    
    /* Simulated key reconstruction */
    for (int i = 0; i < PUF_KEY_SIZE; i++) {
        g_puf_key[i] = g_puf_helper_data[i % sizeof(g_puf_helper_data)] ^ 0x5A;
    }
    
    memcpy(key_output, g_puf_key, PUF_KEY_SIZE);
    
    g_puf_config.reconstruction_count++;
    
    return true;
}

/**
 * @brief Derive key from PUF with additional context
 */
bool puf_derive_key(const uint8_t *context, uint32_t context_len,
                    uint8_t *derived_key, uint32_t key_size) {
    if (!g_puf_initialized || derived_key == NULL) {
        return false;
    }
    
    /* Reconstruct base PUF key */
    uint8_t base_key[PUF_KEY_SIZE];
    if (!puf_reconstruct_key(base_key, sizeof(base_key))) {
        return false;
    }
    
    /* In production: Use HKDF (HMAC-based Key Derivation Function)
     * or similar standard KDF with Secure Vault crypto accelerator
     * 
     * HKDF-Expand(base_key, context, key_size)
     */
    
    /* Simulated key derivation using simple XOR (NOT SECURE - for demo only) */
    for (uint32_t i = 0; i < key_size; i++) {
        derived_key[i] = base_key[i % PUF_KEY_SIZE];
        
        if (context != NULL && context_len > 0) {
            derived_key[i] ^= context[i % context_len];
        }
    }
    
    /* Zeroize base key */
    secure_zeroize(base_key, sizeof(base_key));
    
    return true;
}

/**
 * @brief Wrap a key using PUF-derived wrapping key
 */
bool puf_wrap_key(const uint8_t *plaintext_key, uint32_t key_size,
                  key_type_t key_type, wrapped_key_t *wrapped) {
    if (!g_puf_initialized || plaintext_key == NULL || wrapped == NULL) {
        return false;
    }
    
    if (key_size > WRAPPED_KEY_SIZE - 16) {
        return false;  /* Key too large */
    }
    
    /* Derive wrapping key from PUF */
    uint8_t wrapping_key[PUF_KEY_SIZE];
    const uint8_t kek_context[] = "KEY_WRAPPING_v1";
    
    if (!puf_derive_key(kek_context, sizeof(kek_context) - 1,
                        wrapping_key, sizeof(wrapping_key))) {
        return false;
    }
    
    /* In production: Use AES-KW (Key Wrap) algorithm with Secure Vault
     * Reference: NIST SP 800-38F
     * 
     * AES_KW_Wrap(wrapping_key, plaintext_key) -> wrapped_key
     */
    
    /* Simulated key wrapping (NOT SECURE - for demo only) */
    memset(wrapped, 0, sizeof(wrapped_key_t));
    
    /* Simple XOR encryption (replace with AES-KW in production) */
    for (uint32_t i = 0; i < key_size; i++) {
        wrapped->wrapped_key[i] = plaintext_key[i] ^ wrapping_key[i % PUF_KEY_SIZE];
    }
    
    wrapped->key_type = key_type;
    wrapped->version = 1;
    
    /* Generate authentication tag (in production: use AES-GCM or similar) */
    for (int i = 0; i < 16; i++) {
        wrapped->tag[i] = wrapping_key[i] ^ plaintext_key[i % key_size];
    }
    
    /* Zeroize wrapping key */
    secure_zeroize(wrapping_key, sizeof(wrapping_key));
    
    return true;
}

/**
 * @brief Unwrap a key using PUF-derived wrapping key
 */
bool puf_unwrap_key(const wrapped_key_t *wrapped, uint8_t *plaintext_key,
                    uint32_t key_size) {
    if (!g_puf_initialized || wrapped == NULL || plaintext_key == NULL) {
        return false;
    }
    
    /* Derive wrapping key from PUF */
    uint8_t wrapping_key[PUF_KEY_SIZE];
    const uint8_t kek_context[] = "KEY_WRAPPING_v1";
    
    if (!puf_derive_key(kek_context, sizeof(kek_context) - 1,
                        wrapping_key, sizeof(wrapping_key))) {
        return false;
    }
    
    /* In production: Use AES-KW unwrap algorithm with Secure Vault
     * AES_KW_Unwrap(wrapping_key, wrapped_key) -> plaintext_key
     */
    
    /* Simulated key unwrapping (NOT SECURE - for demo only) */
    for (uint32_t i = 0; i < key_size; i++) {
        plaintext_key[i] = wrapped->wrapped_key[i] ^ wrapping_key[i % PUF_KEY_SIZE];
    }
    
    /* Verify authentication tag (in production: use proper MAC verification) */
    uint8_t expected_tag[16];
    for (int i = 0; i < 16; i++) {
        expected_tag[i] = wrapping_key[i] ^ plaintext_key[i % key_size];
    }
    
    /* Constant-time comparison to prevent timing attacks */
    volatile uint8_t diff = 0;
    for (int i = 0; i < 16; i++) {
        diff |= (expected_tag[i] ^ wrapped->tag[i]);
    }
    
    /* Zeroize wrapping key */
    secure_zeroize(wrapping_key, sizeof(wrapping_key));
    
    if (diff != 0) {
        /* Tag verification failed - zeroize output */
        secure_zeroize(plaintext_key, key_size);
        return false;
    }
    
    return true;
}
