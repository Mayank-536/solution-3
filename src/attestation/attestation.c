/**
 * @file attestation.c
 * @brief Measured Boot Attestation Implementation
 * 
 * Generates signed boot health reports in JSON/CBOR format for
 * remote attestation and forensics.
 */

#include "attestation.h"
#include <string.h>
#include <stdio.h>

/* Global attestation state */
static attestation_report_t g_attestation_report;
static bool g_attestation_initialized = false;

/**
 * @brief Initialize attestation system
 */
bool attestation_init(void) {
    if (g_attestation_initialized) {
        return true;
    }
    
    /* Initialize report structure */
    memset(&g_attestation_report, 0, sizeof(attestation_report_t));
    
    g_attestation_report.version = ATTESTATION_VERSION;
    g_attestation_report.boot_count = 0;
    g_attestation_report.measurement_count = 0;
    g_attestation_report.event_count = 0;
    
    g_attestation_initialized = true;
    
    return true;
}

/**
 * @brief Add boot measurement to report
 */
bool add_boot_measurement(uint32_t component_id, const uint8_t *measurement, uint32_t type) {
    if (!g_attestation_initialized) {
        return false;
    }
    
    if (g_attestation_report.measurement_count >= MAX_MEASUREMENT_COUNT) {
        return false;  /* Report full */
    }
    
    if (measurement == NULL) {
        return false;
    }
    
    boot_measurement_t *m = &g_attestation_report.measurements[g_attestation_report.measurement_count];
    
    m->component_id = component_id;
    memcpy(m->measurement, measurement, 32);
    m->measurement_type = type;
    
    g_attestation_report.measurement_count++;
    
    return true;
}

/**
 * @brief Add event to log
 */
bool add_event_log_entry(uint32_t event_type, uint32_t event_data, const char *description) {
    if (!g_attestation_initialized) {
        return false;
    }
    
    if (g_attestation_report.event_count >= MAX_EVENT_LOG_ENTRIES) {
        return false;  /* Log full */
    }
    
    event_log_entry_t *e = &g_attestation_report.events[g_attestation_report.event_count];
    
    e->event_type = event_type;
    e->event_data = event_data;
    e->timestamp = 0;  /* In production: use hardware RTC */
    
    if (description != NULL) {
        strncpy(e->description, description, sizeof(e->description) - 1);
        e->description[sizeof(e->description) - 1] = '\0';
    } else {
        e->description[0] = '\0';
    }
    
    g_attestation_report.event_count++;
    
    return true;
}

/**
 * @brief Generate attestation report
 */
bool generate_attestation_report(const uint8_t *nonce, attestation_report_t *report) {
    if (!g_attestation_initialized || report == NULL) {
        return false;
    }
    
    /* Copy nonce for freshness proof */
    if (nonce != NULL) {
        memcpy(g_attestation_report.nonce, nonce, NONCE_SIZE);
    }
    
    /* Update counters and status */
    g_attestation_report.boot_count++;
    
    /* In production: Read actual system uptime from RTC */
    g_attestation_report.uptime = 0;
    
    /* Copy report to output */
    memcpy(report, &g_attestation_report, sizeof(attestation_report_t));
    
    return true;
}

/**
 * @brief Sign attestation report
 */
bool sign_attestation_report(attestation_report_t *report) {
    if (report == NULL) {
        return false;
    }
    
    /* In production: Use Secure Vault for ECDSA signing
     * 1. Compute SHA-256 hash of report (excluding signature field)
     * 2. Sign hash with attestation private key (stored in PUF-wrapped form)
     * 3. Store signature in report
     */
    
    /* For demonstration: Generate placeholder signature */
    uint8_t placeholder_sig[ATTESTATION_SIGNATURE_SIZE] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE
    };
    
    memcpy(report->signature, placeholder_sig, ATTESTATION_SIGNATURE_SIZE);
    
    return true;
}

/**
 * @brief Export report to JSON format
 */
uint32_t export_report_json(const attestation_report_t *report, char *json_buffer, uint32_t buffer_size) {
    if (report == NULL || json_buffer == NULL || buffer_size == 0) {
        return 0;
    }
    
    int written = 0;
    char *buf = json_buffer;
    uint32_t remaining = buffer_size;
    
    /* Start JSON object */
    written = snprintf(buf, remaining, "{\n");
    buf += written; remaining -= written;
    
    /* Version */
    written = snprintf(buf, remaining, "  \"version\": %u,\n", report->version);
    buf += written; remaining -= written;
    
    /* Boot count */
    written = snprintf(buf, remaining, "  \"boot_count\": %u,\n", report->boot_count);
    buf += written; remaining -= written;
    
    /* Firmware version */
    written = snprintf(buf, remaining, "  \"firmware_version\": \"0x%08X\",\n", report->firmware_version);
    buf += written; remaining -= written;
    
    /* Security status */
    written = snprintf(buf, remaining, "  \"security_status\": \"0x%08X\",\n", report->security_status);
    buf += written; remaining -= written;
    
    /* Tamper events */
    written = snprintf(buf, remaining, "  \"tamper_events\": %u,\n", report->tamper_events);
    buf += written; remaining -= written;
    
    /* Uptime */
    written = snprintf(buf, remaining, "  \"uptime\": %llu,\n", (unsigned long long)report->uptime);
    buf += written; remaining -= written;
    
    /* Measurements */
    written = snprintf(buf, remaining, "  \"measurements\": [\n");
    buf += written; remaining -= written;
    
    for (uint32_t i = 0; i < report->measurement_count; i++) {
        const boot_measurement_t *m = &report->measurements[i];
        written = snprintf(buf, remaining, "    {\n      \"component_id\": %u,\n", m->component_id);
        buf += written; remaining -= written;
        
        written = snprintf(buf, remaining, "      \"measurement\": \"");
        buf += written; remaining -= written;
        
        for (int j = 0; j < 32; j++) {
            written = snprintf(buf, remaining, "%02X", m->measurement[j]);
            buf += written; remaining -= written;
        }
        
        written = snprintf(buf, remaining, "\",\n      \"type\": %u\n    }", 
                          m->measurement_type);
        buf += written; remaining -= written;
        
        if (i < report->measurement_count - 1) {
            written = snprintf(buf, remaining, ",\n");
        } else {
            written = snprintf(buf, remaining, "\n");
        }
        buf += written; remaining -= written;
    }
    
    written = snprintf(buf, remaining, "  ],\n");
    buf += written; remaining -= written;
    
    /* Events */
    written = snprintf(buf, remaining, "  \"events\": [\n");
    buf += written; remaining -= written;
    
    for (uint32_t i = 0; i < report->event_count; i++) {
        const event_log_entry_t *e = &report->events[i];
        written = snprintf(buf, remaining, 
                          "    {\n      \"type\": %u,\n      \"data\": %u,\n"
                          "      \"timestamp\": %llu,\n      \"description\": \"%s\"\n    }",
                          e->event_type, e->event_data, 
                          (unsigned long long)e->timestamp, e->description);
        buf += written; remaining -= written;
        
        if (i < report->event_count - 1) {
            written = snprintf(buf, remaining, ",\n");
        } else {
            written = snprintf(buf, remaining, "\n");
        }
        buf += written; remaining -= written;
    }
    
    written = snprintf(buf, remaining, "  ],\n");
    buf += written; remaining -= written;
    
    /* Signature */
    written = snprintf(buf, remaining, "  \"signature\": \"");
    buf += written; remaining -= written;
    
    for (int i = 0; i < ATTESTATION_SIGNATURE_SIZE; i++) {
        written = snprintf(buf, remaining, "%02X", report->signature[i]);
        buf += written; remaining -= written;
    }
    
    written = snprintf(buf, remaining, "\"\n}\n");
    buf += written; remaining -= written;
    
    return (buffer_size - remaining);
}

/**
 * @brief Export report to CBOR format
 */
uint32_t export_report_cbor(const attestation_report_t *report, uint8_t *cbor_buffer, uint32_t buffer_size) {
    if (report == NULL || cbor_buffer == NULL || buffer_size == 0) {
        return 0;
    }
    
    /* Simplified CBOR encoding for demonstration
     * In production: Use a full CBOR library like QCBOR or tinycbor
     */
    
    uint32_t offset = 0;
    
    /* CBOR map header (major type 5) */
    cbor_buffer[offset++] = 0xA8;  /* Map with 8 entries */
    
    /* Version */
    cbor_buffer[offset++] = 0x01;  /* Key: 1 */
    cbor_buffer[offset++] = 0x18;  /* Unsigned int (1 byte) */
    cbor_buffer[offset++] = report->version;
    
    /* Boot count */
    cbor_buffer[offset++] = 0x02;  /* Key: 2 */
    cbor_buffer[offset++] = 0x1A;  /* Unsigned int (4 bytes) */
    cbor_buffer[offset++] = (report->boot_count >> 24) & 0xFF;
    cbor_buffer[offset++] = (report->boot_count >> 16) & 0xFF;
    cbor_buffer[offset++] = (report->boot_count >> 8) & 0xFF;
    cbor_buffer[offset++] = report->boot_count & 0xFF;
    
    /* Firmware version */
    cbor_buffer[offset++] = 0x03;  /* Key: 3 */
    cbor_buffer[offset++] = 0x1A;  /* Unsigned int (4 bytes) */
    cbor_buffer[offset++] = (report->firmware_version >> 24) & 0xFF;
    cbor_buffer[offset++] = (report->firmware_version >> 16) & 0xFF;
    cbor_buffer[offset++] = (report->firmware_version >> 8) & 0xFF;
    cbor_buffer[offset++] = report->firmware_version & 0xFF;
    
    /* Add more fields as needed... */
    /* This is a simplified demonstration */
    
    return offset;
}
