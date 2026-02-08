/**
 * @file attestation.h
 * @brief Measured Boot Attestation for Remote Verification
 * 
 * Provides signed boot health reports and telemetry in JSON/CBOR format
 * for remote attestation and forensics.
 */

#ifndef ATTESTATION_H
#define ATTESTATION_H

#include <stdint.h>
#include <stdbool.h>

/* Attestation Report Version */
#define ATTESTATION_VERSION         1

/* Maximum sizes */
#define MAX_MEASUREMENT_COUNT       16
#define MAX_EVENT_LOG_ENTRIES       32
#define ATTESTATION_SIGNATURE_SIZE  64
#define NONCE_SIZE                  16

/* Boot Measurement Structure */
typedef struct {
    uint32_t component_id;       /* Component identifier */
    uint8_t measurement[32];     /* SHA-256 measurement */
    uint32_t measurement_type;   /* Type of measurement */
} boot_measurement_t;

/* Event Log Entry */
typedef struct {
    uint32_t event_type;         /* Type of event */
    uint64_t timestamp;          /* Event timestamp */
    uint32_t event_data;         /* Event-specific data */
    char description[64];        /* Event description */
} event_log_entry_t;

/* Attestation Report Structure */
typedef struct {
    uint32_t version;            /* Report version */
    uint8_t nonce[NONCE_SIZE];   /* Challenge nonce */
    uint32_t boot_count;         /* Number of boots */
    uint32_t firmware_version;   /* Current firmware version */
    
    /* Boot measurements */
    uint32_t measurement_count;
    boot_measurement_t measurements[MAX_MEASUREMENT_COUNT];
    
    /* Event log */
    uint32_t event_count;
    event_log_entry_t events[MAX_EVENT_LOG_ENTRIES];
    
    /* Health status */
    uint32_t tamper_events;      /* Tamper event count */
    uint32_t security_status;    /* Security flags */
    uint64_t uptime;             /* System uptime */
    
    /* Signature */
    uint8_t signature[ATTESTATION_SIGNATURE_SIZE];
} attestation_report_t;

/**
 * @brief Initialize attestation system
 * @return true if initialization successful
 */
bool attestation_init(void);

/**
 * @brief Add boot measurement to report
 * @param component_id Component identifier
 * @param measurement Measurement hash
 * @param type Measurement type
 * @return true if measurement added successfully
 */
bool add_boot_measurement(uint32_t component_id, const uint8_t *measurement, uint32_t type);

/**
 * @brief Add event to log
 * @param event_type Type of event
 * @param event_data Event data
 * @param description Event description
 * @return true if event added successfully
 */
bool add_event_log_entry(uint32_t event_type, uint32_t event_data, const char *description);

/**
 * @brief Generate attestation report
 * @param nonce Challenge nonce for freshness
 * @param report Pointer to report structure to fill
 * @return true if report generated successfully
 */
bool generate_attestation_report(const uint8_t *nonce, attestation_report_t *report);

/**
 * @brief Sign attestation report
 * @param report Pointer to report structure
 * @return true if signing successful
 */
bool sign_attestation_report(attestation_report_t *report);

/**
 * @brief Export report to JSON format
 * @param report Pointer to report structure
 * @param json_buffer Output buffer for JSON
 * @param buffer_size Size of output buffer
 * @return Number of bytes written, or 0 on error
 */
uint32_t export_report_json(const attestation_report_t *report, char *json_buffer, uint32_t buffer_size);

/**
 * @brief Export report to CBOR format
 * @param report Pointer to report structure
 * @param cbor_buffer Output buffer for CBOR
 * @param buffer_size Size of output buffer
 * @return Number of bytes written, or 0 on error
 */
uint32_t export_report_cbor(const attestation_report_t *report, uint8_t *cbor_buffer, uint32_t buffer_size);

#endif /* ATTESTATION_H */
