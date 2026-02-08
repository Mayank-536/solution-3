/**
 * @file secure_boot.h
 * @brief Secure Boot Design with Hardware Root of Trust for EFR32MG26
 * 
 * This header defines the interface for the hardened secure boot system
 * with glitch-resistant control flow and anti-tamper protection.
 */

#ifndef SECURE_BOOT_H
#define SECURE_BOOT_H

#include <stdint.h>
#include <stdbool.h>

/* Security Token Types for Multi-layer Verification */
#define TOKEN_LAYER_1   0x5A3C96E1
#define TOKEN_LAYER_2   0xA5C3691E
#define TOKEN_LAYER_3   0x3C5A1E96
#define TOKEN_LAYER_4   0xC35A961E

/* Token Verification States - Using non-binary values for glitch resistance */
#define TOKEN_STATE_INVALID     0x00000000
#define TOKEN_STATE_LAYER1_OK   0x33CC33CC
#define TOKEN_STATE_LAYER2_OK   0x55AA55AA
#define TOKEN_STATE_LAYER3_OK   0x0F0FF0F0
#define TOKEN_STATE_LAYER4_OK   0xA5A55A5A
#define TOKEN_STATE_ALL_VALID   0xDEADBEEF

/* Anti-Rollback Version */
#define FIRMWARE_VERSION_MAJOR  1
#define FIRMWARE_VERSION_MINOR  0
#define FIRMWARE_VERSION_PATCH  0

/* Boot Status Codes */
typedef enum {
    BOOT_STATUS_INIT = 0x11223344,
    BOOT_STATUS_VERIFYING = 0x55667788,
    BOOT_STATUS_SUCCESS = 0x99AABBCC,
    BOOT_STATUS_FAILURE = 0xDEADDEAD,
    BOOT_STATUS_TAMPER_DETECTED = 0xBADC0FFE
} boot_status_t;

/* Firmware Image Header Structure */
typedef struct __attribute__((packed)) {
    uint32_t magic;              /* Magic number for image validation */
    uint32_t version;            /* Firmware version for anti-rollback */
    uint32_t image_size;         /* Size of firmware image */
    uint32_t load_address;       /* Load address in memory */
    uint32_t entry_point;        /* Entry point address */
    uint8_t signature[64];       /* ECDSA signature */
    uint8_t hash[32];            /* SHA-256 hash of image */
    uint32_t timestamp;          /* Build timestamp */
    uint32_t flags;              /* Security flags */
} firmware_header_t;

/* Boot Context Structure */
typedef struct {
    uint32_t verification_tokens[4];
    uint32_t random_jitter_seed;
    boot_status_t status;
    uint32_t tamper_events;
    uint32_t boot_count;
} boot_context_t;

/**
 * @brief Initialize secure boot system
 * @return boot_status_t Boot initialization status
 */
boot_status_t secure_boot_init(void);

/**
 * @brief Perform layered token verification with glitch resistance
 * @param context Pointer to boot context
 * @return uint32_t Verification state token
 */
uint32_t verify_layered_tokens(boot_context_t *context);

/**
 * @brief Inject random jitter delay for timing desynchronization
 * @param seed Random seed from TRNG
 */
void inject_random_jitter(uint32_t seed);

/**
 * @brief Verify firmware signature using ECDSA
 * @param header Pointer to firmware header
 * @param image Pointer to firmware image
 * @return uint32_t Verification result (non-binary token)
 */
uint32_t verify_firmware_signature(const firmware_header_t *header, const uint8_t *image);

/**
 * @brief Check anti-rollback version
 * @param new_version Version to check
 * @return uint32_t Verification result token
 */
uint32_t check_anti_rollback(uint32_t new_version);

/**
 * @brief Execute secure boot sequence
 * @return boot_status_t Final boot status
 */
boot_status_t execute_secure_boot(void);

#endif /* SECURE_BOOT_H */
