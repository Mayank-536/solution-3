/**
 * @file example_config.c
 * @brief Example Configuration for EFR32MG26 Secure Boot System
 * 
 * Demonstrates how to configure and initialize all secure boot components.
 */

#include "secure_boot.h"
#include "tamper_detection.h"
#include "attestation.h"
#include "trustzone.h"
#include "puf.h"
#include "anti_rollback.h"

/**
 * @brief Example TrustZone configuration for EFR32MG26
 */
static const trustzone_config_t example_tz_config = {
    /* Secure Flash: 0x00000000 - 0x00040000 (256KB) */
    .flash_secure = {
        .start_address = 0x00000000,
        .end_address = 0x00040000,
        .region_type = REGION_TYPE_SECURE,
        .enable = true
    },
    
    /* Non-Secure Flash: 0x00040000 - 0x00100000 (768KB) */
    .flash_non_secure = {
        .start_address = 0x00040000,
        .end_address = 0x00100000,
        .region_type = REGION_TYPE_NON_SECURE,
        .enable = true
    },
    
    /* Secure RAM: 0x20000000 - 0x20008000 (32KB) */
    .ram_secure = {
        .start_address = 0x20000000,
        .end_address = 0x20008000,
        .region_type = REGION_TYPE_SECURE,
        .enable = true
    },
    
    /* Non-Secure RAM: 0x20008000 - 0x20020000 (96KB) */
    .ram_non_secure = {
        .start_address = 0x20008000,
        .end_address = 0x20020000,
        .region_type = REGION_TYPE_NON_SECURE,
        .enable = true
    },
    
    /* Secure Peripherals: Secure Vault, OTP, etc. */
    .peripheral_secure = {
        .start_address = 0x40000000,
        .end_address = 0x50000000,
        .region_type = REGION_TYPE_SECURE,
        .enable = true
    },
    
    .gateway_count = 0
};

/**
 * @brief Main secure boot initialization
 */
int main(void) {
    boot_status_t boot_status;
    tamper_context_t tamper_ctx;
    
    /* Initialize TrustZone first - isolate Secure World */
    if (!trustzone_init(&example_tz_config)) {
        /* TrustZone initialization failed - halt */
        while (1);
    }
    
    /* Initialize PUF for key derivation */
    if (!puf_init()) {
        while (1);
    }
    
    /* Enroll PUF if first boot */
    if (!puf_enroll()) {
        /* PUF enrollment failed - halt */
        while (1);
    }
    
    /* Initialize tamper detection */
    if (!tamper_detection_start(&tamper_ctx)) {
        while (1);
    }
    
    /* Initialize attestation system */
    if (!attestation_init()) {
        while (1);
    }
    
    /* Execute secure boot sequence */
    boot_status = execute_secure_boot();
    
    if (boot_status == BOOT_STATUS_SUCCESS) {
        /* Boot successful - add boot measurement */
        uint8_t boot_measurement[32] = {0};  /* In production: actual measurement */
        add_boot_measurement(1, boot_measurement, 0);
        
        /* Add success event */
        add_event_log_entry(1, 0, "Secure boot completed successfully");
        
        /* Generate attestation report */
        attestation_report_t report;
        uint8_t nonce[NONCE_SIZE] = {0};  /* In production: from remote verifier */
        
        if (generate_attestation_report(nonce, &report)) {
            sign_attestation_report(&report);
            
            /* Export report (example: JSON) */
            char json_buffer[4096];
            uint32_t json_len = export_report_json(&report, json_buffer, sizeof(json_buffer));
            
            /* In production: Send report to remote attestation server */
        }
        
        /* Transition to Non-Secure application */
        /* transition_to_nonsecure(0x00040000, 0x20020000); */
    } else {
        /* Boot failed - log and halt */
        add_event_log_entry(2, boot_status, "Secure boot failed");
        
        /* Check for tamper events */
        uint32_t tamper_events = check_tamper_events(&tamper_ctx);
        if (tamper_events != TAMPER_EVENT_NONE) {
            execute_tamper_response(tamper_events);
        }
        
        while (1);
    }
    
    /* Should never reach here */
    return 0;
}
