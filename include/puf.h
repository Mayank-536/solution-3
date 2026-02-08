/**
 * @file puf.h
 * @brief Physically Unclonable Function (PUF) based Key Wrapping
 * 
 * Implements PUF-based key derivation and wrapping to protect against
 * memory dump extraction attacks.
 */

#ifndef PUF_H
#define PUF_H

#include <stdint.h>
#include <stdbool.h>

/* Key sizes */
#define PUF_KEY_SIZE            32   /* 256-bit key */
#define WRAPPED_KEY_SIZE        48   /* Wrapped key with metadata */
#define KEY_DERIVATION_SALT_SIZE 16  /* Salt for key derivation */

/* Key Types */
typedef enum {
    KEY_TYPE_ENCRYPTION = 0x01,
    KEY_TYPE_SIGNING = 0x02,
    KEY_TYPE_ATTESTATION = 0x03,
    KEY_TYPE_STORAGE = 0x04
} key_type_t;

/* PUF Configuration */
typedef struct {
    uint32_t enrollment_done;    /* PUF enrollment status */
    uint32_t reconstruction_count;/* Number of key reconstructions */
    uint32_t error_threshold;    /* Error correction threshold */
} puf_config_t;

/* Wrapped Key Structure */
typedef struct {
    uint8_t wrapped_key[WRAPPED_KEY_SIZE];  /* Encrypted key material */
    uint32_t key_type;                       /* Type of key */
    uint32_t version;                        /* Key version */
    uint8_t tag[16];                         /* Authentication tag */
} wrapped_key_t;

/**
 * @brief Initialize PUF subsystem
 * @return true if initialization successful
 */
bool puf_init(void);

/**
 * @brief Enroll PUF (first-time initialization)
 * @return true if enrollment successful
 */
bool puf_enroll(void);

/**
 * @brief Reconstruct PUF key
 * @param key_output Buffer to receive reconstructed key
 * @param key_size Size of key buffer
 * @return true if reconstruction successful
 */
bool puf_reconstruct_key(uint8_t *key_output, uint32_t key_size);

/**
 * @brief Derive key from PUF with additional context
 * @param context Context string for key derivation
 * @param context_len Length of context
 * @param derived_key Output buffer for derived key
 * @param key_size Size of derived key
 * @return true if derivation successful
 */
bool puf_derive_key(const uint8_t *context, uint32_t context_len, 
                    uint8_t *derived_key, uint32_t key_size);

/**
 * @brief Wrap a key using PUF-derived wrapping key
 * @param plaintext_key Key to wrap
 * @param key_size Size of plaintext key
 * @param key_type Type of key being wrapped
 * @param wrapped Output wrapped key structure
 * @return true if wrapping successful
 */
bool puf_wrap_key(const uint8_t *plaintext_key, uint32_t key_size,
                  key_type_t key_type, wrapped_key_t *wrapped);

/**
 * @brief Unwrap a key using PUF-derived wrapping key
 * @param wrapped Wrapped key structure
 * @param plaintext_key Output buffer for unwrapped key
 * @param key_size Size of output buffer
 * @return true if unwrapping successful
 */
bool puf_unwrap_key(const wrapped_key_t *wrapped, uint8_t *plaintext_key, 
                    uint32_t key_size);

/**
 * @brief Zeroize key material from memory
 * @param key Pointer to key buffer
 * @param size Size of key buffer
 */
void secure_zeroize(uint8_t *key, uint32_t size);

#endif /* PUF_H */
