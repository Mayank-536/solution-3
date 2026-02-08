/**
 * @file crypto_primitives.c
 * @brief Implementation of cryptographic primitives
 */

#include "../../include/crypto_primitives.h"
#include <string.h>

/* TRNG registers (EFR32MG26 specific) */
#define TRNG_BASE                 0x4C022000
#define TRNG_CONTROL_REG          (TRNG_BASE + 0x000)
#define TRNG_FIFO_REG             (TRNG_BASE + 0x004)
#define TRNG_STATUS_REG           (TRNG_BASE + 0x008)

/* PUF registers (Secure Vault High) */
#define PUF_BASE                  0x4C023000
#define PUF_CONTROL_REG           (PUF_BASE + 0x000)
#define PUF_OUTPUT_REG            (PUF_BASE + 0x004)
#define PUF_STATUS_REG            (PUF_BASE + 0x008)

/* TRNG control bits */
#define TRNG_ENABLE               0x00000001
#define TRNG_READY                0x00000001

/* PUF control bits */
#define PUF_ENABLE                0x00000001
#define PUF_DERIVE                0x00000002
#define PUF_READY                 0x00000001

/* AES registers for key wrapping */
#define AES_BASE                  0x4C024000
#define AES_CONTROL_REG           (AES_BASE + 0x000)
#define AES_KEY_REG               (AES_BASE + 0x010)
#define AES_DATA_REG              (AES_BASE + 0x020)

/**
 * @brief Initialize TRNG
 */
bool trng_init(void) {
    /* Enable TRNG */
    *((volatile uint32_t *)TRNG_CONTROL_REG) = TRNG_ENABLE;
    
    /* Wait for TRNG to be ready */
    uint32_t timeout = 100000;
    while (timeout--) {
        if (*((volatile uint32_t *)TRNG_STATUS_REG) & TRNG_READY) {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Generate random bytes using TRNG
 */
bool trng_generate(uint8_t *buffer, size_t length) {
    if (!buffer || length == 0) {
        return false;
    }
    
    for (size_t i = 0; i < length; i += 4) {
        /* Wait for TRNG ready */
        uint32_t timeout = 10000;
        while (timeout--) {
            if (*((volatile uint32_t *)TRNG_STATUS_REG) & TRNG_READY) {
                break;
            }
        }
        if (timeout == 0) {
            return false;
        }
        
        /* Read random word */
        uint32_t random = *((volatile uint32_t *)TRNG_FIFO_REG);
        
        /* Copy to buffer */
        size_t bytes_to_copy = (length - i) < 4 ? (length - i) : 4;
        memcpy(&buffer[i], &random, bytes_to_copy);
    }
    
    return true;
}

/**
 * @brief Add random jitter delay for glitch resistance
 */
void trng_random_jitter(uint32_t min_cycles, uint32_t max_cycles) {
    uint8_t random_byte;
    
    if (!trng_generate(&random_byte, 1)) {
        /* Fallback to fixed delay if TRNG fails */
        for (volatile uint32_t i = 0; i < min_cycles; i++) {
            __asm__ volatile ("nop");
        }
        return;
    }
    
    /* Calculate jitter based on random value */
    uint32_t range = max_cycles - min_cycles;
    uint32_t jitter = min_cycles + ((random_byte * range) / 256);
    
    /* Execute delay */
    for (volatile uint32_t i = 0; i < jitter; i++) {
        __asm__ volatile ("nop");
    }
}

/**
 * @brief Initialize PUF
 */
bool puf_init(void) {
    /* Enable PUF */
    *((volatile uint32_t *)PUF_CONTROL_REG) = PUF_ENABLE;
    
    /* Wait for PUF to be ready */
    uint32_t timeout = 100000;
    while (timeout--) {
        if (*((volatile uint32_t *)PUF_STATUS_REG) & PUF_READY) {
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Derive key from PUF
 */
bool puf_derive_key(uint8_t *key, size_t key_size) {
    if (!key || key_size == 0 || key_size > PUF_KEY_SIZE) {
        return false;
    }
    
    /* Trigger PUF key derivation */
    *((volatile uint32_t *)PUF_CONTROL_REG) = PUF_ENABLE | PUF_DERIVE;
    
    /* Wait for derivation to complete */
    uint32_t timeout = 100000;
    while (timeout--) {
        if (*((volatile uint32_t *)PUF_STATUS_REG) & PUF_READY) {
            break;
        }
    }
    if (timeout == 0) {
        return false;
    }
    
    /* Read derived key */
    for (size_t i = 0; i < key_size; i += 4) {
        uint32_t key_word = *((volatile uint32_t *)PUF_OUTPUT_REG);
        size_t bytes_to_copy = (key_size - i) < 4 ? (key_size - i) : 4;
        memcpy(&key[i], &key_word, bytes_to_copy);
    }
    
    return true;
}

/**
 * @brief Wrap key using PUF-derived KEK
 */
bool puf_wrap_key(const uint8_t *plaintext_key, uint8_t *wrapped_key) {
    if (!plaintext_key || !wrapped_key) {
        return false;
    }
    
    uint8_t kek[PUF_KEY_SIZE];
    
    /* Derive KEK from PUF */
    if (!puf_derive_key(kek, PUF_KEY_SIZE)) {
        return false;
    }
    
    /* Generate random IV */
    uint8_t iv[16];
    if (!trng_generate(iv, 16)) {
        return false;
    }
    
    /* Copy IV to wrapped key */
    memcpy(wrapped_key, iv, 16);
    
    /* Encrypt key with KEK using AES-256 (simplified) */
    /* In real implementation, this would use hardware AES */
    memcpy(&wrapped_key[16], plaintext_key, PUF_KEY_SIZE);
    
    /* XOR with KEK for demonstration (real impl would use AES-KW) */
    for (size_t i = 0; i < PUF_KEY_SIZE; i++) {
        wrapped_key[16 + i] ^= kek[i];
    }
    
    /* Clear KEK from memory */
    memset(kek, 0, PUF_KEY_SIZE);
    
    return true;
}

/**
 * @brief Unwrap key using PUF-derived KEK
 */
bool puf_unwrap_key(const uint8_t *wrapped_key, uint8_t *plaintext_key) {
    if (!wrapped_key || !plaintext_key) {
        return false;
    }
    
    uint8_t kek[PUF_KEY_SIZE];
    
    /* Derive KEK from PUF */
    if (!puf_derive_key(kek, PUF_KEY_SIZE)) {
        return false;
    }
    
    /* Decrypt key (simplified) */
    memcpy(plaintext_key, &wrapped_key[16], PUF_KEY_SIZE);
    
    /* XOR with KEK */
    for (size_t i = 0; i < PUF_KEY_SIZE; i++) {
        plaintext_key[i] ^= kek[i];
    }
    
    /* Clear KEK from memory */
    memset(kek, 0, PUF_KEY_SIZE);
    
    return true;
}

/**
 * @brief Verify digital signature (simplified Ed25519-like)
 */
bool verify_signature(const uint8_t *data, size_t data_len,
                     const uint8_t *signature, const uint8_t *public_key) {
    if (!data || !signature || !public_key || data_len == 0) {
        return false;
    }
    
    /* In real implementation, this would use hardware crypto accelerator
     * to verify Ed25519 or ECDSA signature */
    
    /* For demonstration, perform a simple check */
    /* This is NOT secure and only for structural demonstration */
    uint8_t hash[32] = {0};
    
    /* Simple hash calculation (real impl would use SHA-256) */
    for (size_t i = 0; i < data_len; i++) {
        hash[i % 32] ^= data[i];
    }
    
    /* Verify first 32 bytes of signature match hash */
    for (size_t i = 0; i < 32; i++) {
        if (signature[i] != hash[i]) {
            return false;
        }
    }
    
    return true;
}
