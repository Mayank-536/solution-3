/**
 * @file attestation.h
 * @brief Measured boot with signed JSON attestation
 */

#ifndef ATTESTATION_H
#define ATTESTATION_H

#include <stdint.h>
#include <stdbool.h>

/* Attestation configuration */
#define MAX_MEASUREMENTS          16
#define ATTESTATION_BUFFER_SIZE   2048

/* Boot stages for measurement */
#define STAGE_BOOTLOADER          0
#define STAGE_SECURE_VAULT        1
#define STAGE_RTSL                2
#define STAGE_FIRMWARE            3
#define STAGE_APPLICATION         4

/**
 * @brief Initialize attestation system
 * @return true on success, false on failure
 */
bool attestation_init(void);

/**
 * @brief Record measurement for boot stage
 * @param stage Boot stage identifier
 * @param data Data to measure
 * @param length Length of data
 * @return true on success, false on failure
 */
bool record_measurement(uint32_t stage, const uint8_t *data, size_t length);

/**
 * @brief Generate signed JSON attestation report
 * @param buffer Output buffer for JSON attestation
 * @param buffer_size Size of output buffer
 * @return Number of bytes written, or 0 on error
 */
size_t generate_attestation_report(char *buffer, size_t buffer_size);

/**
 * @brief Verify attestation signature
 * @param attestation JSON attestation string
 * @param attestation_len Length of attestation
 * @return true if signature is valid, false otherwise
 */
bool verify_attestation(const char *attestation, size_t attestation_len);

/**
 * @brief Get measurement for specific boot stage
 * @param stage Boot stage identifier
 * @param measurement Output buffer for measurement
 * @return true on success, false on failure
 */
bool get_measurement(uint32_t stage, uint8_t measurement[32]);

#endif /* ATTESTATION_H */
