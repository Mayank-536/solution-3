/**
 * @file crypto_primitives.h
 * @brief Cryptographic primitives for secure boot
 * 
 * Implements PUF-based key wrapping, TRNG operations, and signature verification
 */

#ifndef CRYPTO_PRIMITIVES_H
#define CRYPTO_PRIMITIVES_H

#include <stdint.h>
#include <stdbool.h>

/* Key sizes */
#define PUF_KEY_SIZE              32
#define WRAPPED_KEY_SIZE          48
#define SIGNATURE_SIZE            64
#define TRNG_SEED_SIZE            32

/**
 * @brief Initialize TRNG (True Random Number Generator)
 * @return true on success, false on failure
 */
bool trng_init(void);

/**
 * @brief Generate random bytes using TRNG
 * @param buffer Output buffer
 * @param length Number of bytes to generate
 * @return true on success, false on failure
 */
bool trng_generate(uint8_t *buffer, size_t length);

/**
 * @brief Add random jitter delay for glitch resistance
 * @param min_cycles Minimum delay cycles
 * @param max_cycles Maximum delay cycles
 */
void trng_random_jitter(uint32_t min_cycles, uint32_t max_cycles);

/**
 * @brief Initialize PUF (Physical Unclonable Function)
 * @return true on success, false on failure
 */
bool puf_init(void);

/**
 * @brief Derive key from PUF
 * @param key Output key buffer
 * @param key_size Size of key to derive
 * @return true on success, false on failure
 */
bool puf_derive_key(uint8_t *key, size_t key_size);

/**
 * @brief Wrap key using PUF-derived key encryption key
 * @param plaintext_key Key to wrap
 * @param wrapped_key Output wrapped key
 * @return true on success, false on failure
 */
bool puf_wrap_key(const uint8_t *plaintext_key, uint8_t *wrapped_key);

/**
 * @brief Unwrap key using PUF-derived key encryption key
 * @param wrapped_key Wrapped key input
 * @param plaintext_key Output unwrapped key
 * @return true on success, false on failure
 */
bool puf_unwrap_key(const uint8_t *wrapped_key, uint8_t *plaintext_key);

/**
 * @brief Verify digital signature
 * @param data Data to verify
 * @param data_len Length of data
 * @param signature Signature bytes
 * @param public_key Public key for verification
 * @return true if signature is valid, false otherwise
 */
bool verify_signature(const uint8_t *data, size_t data_len, 
                     const uint8_t *signature, const uint8_t *public_key);

#endif /* CRYPTO_PRIMITIVES_H */
