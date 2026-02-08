/**
 * @file attestation.c
 * @brief Implementation of measured boot with signed JSON attestation
 */

#include "../../include/attestation.h"
#include "../../include/crypto_primitives.h"
#include <string.h>
#include <stdio.h>

/* SHA-256 digest size */
#define SHA256_DIGEST_SIZE        32

/* Attestation storage */
typedef struct {
    uint32_t stage;
    uint8_t measurement[SHA256_DIGEST_SIZE];
    bool valid;
} measurement_entry_t;

static measurement_entry_t g_measurements[MAX_MEASUREMENTS];
static uint32_t g_measurement_count = 0;

/**
 * @brief Calculate SHA-256 hash (simplified)
 */
static void calculate_sha256(const uint8_t *data, size_t length, uint8_t hash[32]) {
    /* In real implementation, this would use hardware SHA-256 accelerator */
    /* This is a simplified version for demonstration */
    
    memset(hash, 0, 32);
    
    for (size_t i = 0; i < length; i++) {
        hash[i % 32] ^= data[i];
        /* Simple mixing */
        if (i % 4 == 0) {
            for (int j = 0; j < 32; j++) {
                hash[j] = (hash[j] << 1) | (hash[j] >> 7);
            }
        }
    }
}

/**
 * @brief Initialize attestation system
 */
bool attestation_init(void) {
    /* Clear measurement storage */
    memset(g_measurements, 0, sizeof(g_measurements));
    g_measurement_count = 0;
    
    return true;
}

/**
 * @brief Record measurement for boot stage
 */
bool record_measurement(uint32_t stage, const uint8_t *data, size_t length) {
    if (!data || length == 0 || g_measurement_count >= MAX_MEASUREMENTS) {
        return false;
    }
    
    /* Calculate measurement (SHA-256 hash) */
    uint8_t measurement[SHA256_DIGEST_SIZE];
    calculate_sha256(data, length, measurement);
    
    /* Store measurement */
    g_measurements[g_measurement_count].stage = stage;
    memcpy(g_measurements[g_measurement_count].measurement, measurement, SHA256_DIGEST_SIZE);
    g_measurements[g_measurement_count].valid = true;
    g_measurement_count++;
    
    return true;
}

/**
 * @brief Get measurement for specific boot stage
 */
bool get_measurement(uint32_t stage, uint8_t measurement[32]) {
    if (!measurement) {
        return false;
    }
    
    /* Find measurement for stage */
    for (uint32_t i = 0; i < g_measurement_count; i++) {
        if (g_measurements[i].valid && g_measurements[i].stage == stage) {
            memcpy(measurement, g_measurements[i].measurement, SHA256_DIGEST_SIZE);
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Convert bytes to hex string
 */
static void bytes_to_hex(const uint8_t *bytes, size_t len, char *hex) {
    const char hex_chars[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        hex[i * 2] = hex_chars[(bytes[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_chars[bytes[i] & 0x0F];
    }
    hex[len * 2] = '\0';
}

/**
 * @brief Generate signed JSON attestation report
 */
size_t generate_attestation_report(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size < ATTESTATION_BUFFER_SIZE) {
        return 0;
    }
    
    /* Start JSON object */
    int offset = 0;
    offset += snprintf(buffer + offset, buffer_size - offset, "{\n");
    offset += snprintf(buffer + offset, buffer_size - offset, "  \"version\": \"1.0\",\n");
    offset += snprintf(buffer + offset, buffer_size - offset, "  \"timestamp\": %u,\n", 0);
    offset += snprintf(buffer + offset, buffer_size - offset, "  \"measurements\": [\n");
    
    /* Add all measurements */
    for (uint32_t i = 0; i < g_measurement_count; i++) {
        if (!g_measurements[i].valid) {
            continue;
        }
        
        char hex_measurement[SHA256_DIGEST_SIZE * 2 + 1];
        bytes_to_hex(g_measurements[i].measurement, SHA256_DIGEST_SIZE, hex_measurement);
        
        const char *stage_name;
        switch (g_measurements[i].stage) {
            case STAGE_BOOTLOADER: stage_name = "bootloader"; break;
            case STAGE_SECURE_VAULT: stage_name = "secure_vault"; break;
            case STAGE_RTSL: stage_name = "rtsl"; break;
            case STAGE_FIRMWARE: stage_name = "firmware"; break;
            case STAGE_APPLICATION: stage_name = "application"; break;
            default: stage_name = "unknown"; break;
        }
        
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "    {\n"
                          "      \"stage\": \"%s\",\n"
                          "      \"measurement\": \"%s\"\n"
                          "    }%s\n",
                          stage_name, hex_measurement,
                          (i < g_measurement_count - 1) ? "," : "");
    }
    
    offset += snprintf(buffer + offset, buffer_size - offset, "  ],\n");
    
    /* Generate signature over the measurements */
    uint8_t puf_key[PUF_KEY_SIZE];
    if (!puf_derive_key(puf_key, PUF_KEY_SIZE)) {
        return 0;
    }
    
    /* Calculate signature (simplified - real impl would use ECDSA/Ed25519) */
    uint8_t signature[SIGNATURE_SIZE];
    memset(signature, 0, SIGNATURE_SIZE);
    calculate_sha256((uint8_t *)buffer, offset, signature);
    
    /* XOR with PUF key for demonstration */
    for (int i = 0; i < 32; i++) {
        signature[i] ^= puf_key[i];
    }
    
    /* Add signature to JSON */
    char hex_signature[SIGNATURE_SIZE * 2 + 1];
    bytes_to_hex(signature, SIGNATURE_SIZE, hex_signature);
    
    offset += snprintf(buffer + offset, buffer_size - offset,
                      "  \"signature\": \"%s\"\n", hex_signature);
    offset += snprintf(buffer + offset, buffer_size - offset, "}\n");
    
    return offset;
}

/**
 * @brief Verify attestation signature
 */
bool verify_attestation(const char *attestation, size_t attestation_len) {
    if (!attestation || attestation_len == 0) {
        return false;
    }
    
    /* In real implementation, this would:
     * 1. Parse JSON attestation
     * 2. Extract signature
     * 3. Recalculate hash over measurements
     * 4. Verify signature using public key
     */
    
    /* For demonstration, perform basic validation */
    if (attestation_len < 100) {
        return false;
    }
    
    /* Check for required fields */
    if (strstr(attestation, "\"version\"") == NULL ||
        strstr(attestation, "\"measurements\"") == NULL ||
        strstr(attestation, "\"signature\"") == NULL) {
        return false;
    }
    
    return true;
}
