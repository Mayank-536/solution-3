/**
 * @file trustzone.c
 * @brief TrustZone-M Configuration for Secure World Isolation
 * 
 * Configures ARM TrustZone-M Security Attribution Unit (SAU) to isolate
 * critical boot logic in the Secure World.
 */

#include "trustzone.h"
#include <string.h>

/* SAU register definitions (simplified for demonstration) */
#define SAU_CTRL_ENABLE     (1UL << 0)
#define SAU_CTRL_ALLNS      (1UL << 1)

/* Global TrustZone configuration */
static trustzone_config_t g_tz_config;
static bool g_tz_initialized = false;

/**
 * @brief Configure SAU region
 */
bool sau_configure_region(uint32_t region_number, const sau_region_config_t *config) {
    if (region_number >= 8 || config == NULL) {
        return false;  /* EFR32MG26 has 8 SAU regions */
    }
    
    /* In production: Configure actual SAU registers
     * Reference: ARM Cortex-M33 TrustZone documentation
     */
    
    /* SAU->RNR = region_number; */  /* Select region */
    /* SAU->RBAR = config->start_address; */  /* Set base address */
    /* SAU->RLAR = config->end_address | (config->enable ? 1 : 0); */  /* Set limit and enable */
    
    /* Set region as Secure or Non-Secure */
    if (config->region_type == REGION_TYPE_SECURE) {
        /* SAU->RLAR &= ~SAU_RLAR_NSC_Msk; */  /* Secure, not NSC */
    } else {
        /* SAU->RLAR |= SAU_RLAR_NSC_Msk; */  /* Non-Secure Callable */
    }
    
    return true;
}

/**
 * @brief Enable SAU
 */
bool sau_enable(void) {
    /* In production: Enable SAU via control register */
    /* SAU->CTRL = SAU_CTRL_ENABLE; */
    
    /* Enable SecureFault exception */
    /* SCB->SHCSR |= SCB_SHCSR_SECUREFAULTENA_Msk; */
    
    return true;
}

/**
 * @brief Initialize TrustZone and configure SAU regions
 */
bool trustzone_init(const trustzone_config_t *config) {
    if (config == NULL || g_tz_initialized) {
        return false;
    }
    
    /* Store configuration */
    memcpy(&g_tz_config, config, sizeof(trustzone_config_t));
    
    /* Configure Secure Flash region
     * Typically: 0x00000000 - 0x00040000 (256KB for bootloader and secure code)
     */
    if (!sau_configure_region(0, &config->flash_secure)) {
        return false;
    }
    
    /* Configure Non-Secure Flash region
     * Typically: 0x00040000 - 0x00100000 (remaining flash for application)
     */
    if (!sau_configure_region(1, &config->flash_non_secure)) {
        return false;
    }
    
    /* Configure Secure RAM region
     * Typically: 0x20000000 - 0x20008000 (32KB for secure data)
     */
    if (!sau_configure_region(2, &config->ram_secure)) {
        return false;
    }
    
    /* Configure Non-Secure RAM region
     * Typically: 0x20008000 - 0x20020000 (remaining RAM for application)
     */
    if (!sau_configure_region(3, &config->ram_non_secure)) {
        return false;
    }
    
    /* Configure Secure Peripheral region
     * Typically: Critical peripherals like Secure Vault, OTP
     */
    if (!sau_configure_region(4, &config->peripheral_secure)) {
        return false;
    }
    
    /* Enable SAU */
    if (!sau_enable()) {
        return false;
    }
    
    /* Configure interrupt target states
     * Critical interrupts should target Secure state
     */
    /* NVIC->ITNS[0] = 0x00000000; */  /* All interrupts initially Secure */
    
    /* Configure specific interrupts as Non-Secure if needed */
    
    g_tz_initialized = true;
    
    return true;
}

/**
 * @brief Register secure gateway function
 */
bool register_secure_gateway(const secure_gateway_t *gateway) {
    if (gateway == NULL || !g_tz_initialized) {
        return false;
    }
    
    if (g_tz_config.gateway_count >= 16) {
        return false;  /* Maximum gateways reached */
    }
    
    /* Store gateway configuration */
    memcpy(&g_tz_config.gateways[g_tz_config.gateway_count], 
           gateway, sizeof(secure_gateway_t));
    
    g_tz_config.gateway_count++;
    
    /* In production: Set up SG veneer in Non-Secure Callable region
     * The gateway function must be in an NSC region and start with SG instruction
     */
    
    return true;
}

/**
 * @brief Transition to Non-Secure state
 */
void transition_to_nonsecure(uint32_t ns_reset_handler, uint32_t ns_stack_pointer) {
    /* Prepare for transition to Non-Secure state
     * This is typically called after secure boot completes
     */
    
    /* Set up Non-Secure stack pointer */
    /* __TZ_set_MSP_NS(ns_stack_pointer); */
    
    /* Set up Non-Secure reset handler */
    /* __TZ_set_CONTROL_NS(0); */
    
    /* Clear any sensitive data from Secure world registers */
    /* ... */
    
    /* Jump to Non-Secure reset handler */
    /* typedef void (*funcptr_NS)(void) __attribute__((cmse_nonsecure_call)); */
    /* funcptr_NS fp_NS = (funcptr_NS)ns_reset_handler; */
    /* fp_NS(); */
    
    /* In production: Use proper CMSE (ARM Cortex-M Security Extensions) API
     * to perform secure transition
     */
}

/**
 * @brief Check if address is in Secure region
 */
bool is_address_secure(uint32_t address) {
    if (!g_tz_initialized) {
        return false;
    }
    
    /* Check against configured SAU regions */
    
    /* Check Secure Flash */
    if (address >= g_tz_config.flash_secure.start_address &&
        address < g_tz_config.flash_secure.end_address &&
        g_tz_config.flash_secure.region_type == REGION_TYPE_SECURE) {
        return true;
    }
    
    /* Check Secure RAM */
    if (address >= g_tz_config.ram_secure.start_address &&
        address < g_tz_config.ram_secure.end_address &&
        g_tz_config.ram_secure.region_type == REGION_TYPE_SECURE) {
        return true;
    }
    
    /* Check Secure Peripherals */
    if (address >= g_tz_config.peripheral_secure.start_address &&
        address < g_tz_config.peripheral_secure.end_address &&
        g_tz_config.peripheral_secure.region_type == REGION_TYPE_SECURE) {
        return true;
    }
    
    return false;
}
