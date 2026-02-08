/**
 * @file trustzone.h
 * @brief TrustZone Configuration for EFR32MG26 Secure World Isolation
 * 
 * Configures ARM TrustZone-M to isolate critical boot logic in the
 * Secure World, protecting against application-layer vulnerabilities.
 */

#ifndef TRUSTZONE_H
#define TRUSTZONE_H

#include <stdint.h>
#include <stdbool.h>

/* Memory Region Types */
#define REGION_TYPE_SECURE      0x00
#define REGION_TYPE_NON_SECURE  0x01

/* SAU Region Configuration */
typedef struct {
    uint32_t start_address;      /* Region start address */
    uint32_t end_address;        /* Region end address */
    uint32_t region_type;        /* Secure or Non-Secure */
    bool enable;                 /* Region enabled */
} sau_region_config_t;

/* Secure Gateway Configuration */
typedef struct {
    uint32_t gateway_address;    /* Secure gateway entry point */
    uint32_t function_id;        /* Function identifier */
    bool enabled;                /* Gateway enabled */
} secure_gateway_t;

/* TrustZone Configuration */
typedef struct {
    sau_region_config_t flash_secure;      /* Secure flash region */
    sau_region_config_t flash_non_secure;  /* Non-secure flash region */
    sau_region_config_t ram_secure;        /* Secure RAM region */
    sau_region_config_t ram_non_secure;    /* Non-secure RAM region */
    sau_region_config_t peripheral_secure; /* Secure peripherals */
    uint32_t gateway_count;                /* Number of secure gateways */
    secure_gateway_t gateways[16];         /* Secure gateway entries */
} trustzone_config_t;

/**
 * @brief Initialize TrustZone and configure SAU regions
 * @param config TrustZone configuration
 * @return true if initialization successful
 */
bool trustzone_init(const trustzone_config_t *config);

/**
 * @brief Configure SAU region
 * @param region_number SAU region number (0-7)
 * @param config Region configuration
 * @return true if configuration successful
 */
bool sau_configure_region(uint32_t region_number, const sau_region_config_t *config);

/**
 * @brief Enable SAU
 * @return true if SAU enabled successfully
 */
bool sau_enable(void);

/**
 * @brief Register secure gateway function
 * @param gateway Gateway configuration
 * @return true if gateway registered successfully
 */
bool register_secure_gateway(const secure_gateway_t *gateway);

/**
 * @brief Transition to Non-Secure state
 * @param ns_reset_handler Non-Secure reset handler address
 * @param ns_stack_pointer Non-Secure stack pointer
 */
void transition_to_nonsecure(uint32_t ns_reset_handler, uint32_t ns_stack_pointer);

/**
 * @brief Check if address is in Secure region
 * @param address Address to check
 * @return true if address is secure
 */
bool is_address_secure(uint32_t address);

#endif /* TRUSTZONE_H */
