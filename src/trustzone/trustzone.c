/**
 * @file trustzone.c
 * @brief Implementation of TrustZone Secure World isolation
 */

#include "../../include/trustzone.h"
#include <string.h>

/* ARM TrustZone-M registers (Cortex-M33 in EFR32MG26) */
#define SAU_CTRL                  0xE000EDD0
#define SAU_TYPE                  0xE000EDD4
#define SAU_RNR                   0xE000EDD8
#define SAU_RBAR                  0xE000EDDC
#define SAU_RLAR                  0xE000EDE0

/* Security Attribution Unit control bits */
#define SAU_CTRL_ENABLE           0x00000001
#define SAU_CTRL_ALLNS            0x00000002

/* Region attributes */
#define SAU_REGION_ENABLE         0x00000001
#define SAU_REGION_NSC            0x00000002

/* Number of SAU regions */
#define SAU_REGION_COUNT          8

/* Memory region definitions for EFR32MG26 */
#define SECURE_CODE_BASE          0x00000000
#define SECURE_CODE_SIZE          0x00040000  /* 256KB */
#define SECURE_RAM_BASE           0x20000000
#define SECURE_RAM_SIZE           0x00010000  /* 64KB */
#define NSC_REGION_BASE           0x10000000
#define NSC_REGION_SIZE           0x00001000  /* 4KB */

/* NVIC Interrupt Target Non-Secure Register */
#define NVIC_ITNS_BASE            0xE000E380

/* Current security state */
static volatile uint32_t g_security_state = TZ_STATE_SECURE;

/**
 * @brief Configure SAU region
 */
static bool configure_sau_region(uint32_t region_num, uint32_t base_addr, 
                                 uint32_t limit_addr, uint32_t attributes) {
    if (region_num >= SAU_REGION_COUNT) {
        return false;
    }
    
    /* Select region */
    *((volatile uint32_t *)SAU_RNR) = region_num;
    
    /* Set base address */
    *((volatile uint32_t *)SAU_RBAR) = base_addr & 0xFFFFFFE0;
    
    /* Set limit address and attributes */
    *((volatile uint32_t *)SAU_RLAR) = (limit_addr & 0xFFFFFFE0) | attributes;
    
    return true;
}

/**
 * @brief Configure SAU (Security Attribution Unit)
 */
bool configure_sau(void) {
    /* Disable SAU during configuration */
    *((volatile uint32_t *)SAU_CTRL) = 0;
    
    /* Configure Region 0: Secure Code */
    configure_sau_region(0, SECURE_CODE_BASE, 
                        SECURE_CODE_BASE + SECURE_CODE_SIZE - 1,
                        SAU_REGION_ENABLE);
    
    /* Configure Region 1: Secure RAM */
    configure_sau_region(1, SECURE_RAM_BASE,
                        SECURE_RAM_BASE + SECURE_RAM_SIZE - 1,
                        SAU_REGION_ENABLE);
    
    /* Configure Region 2: Non-Secure Callable (NSC) */
    configure_sau_region(2, NSC_REGION_BASE,
                        NSC_REGION_BASE + NSC_REGION_SIZE - 1,
                        SAU_REGION_ENABLE | SAU_REGION_NSC);
    
    /* Configure Region 3: Non-Secure Code */
    configure_sau_region(3, 0x00040000,
                        0x000FFFFF,
                        SAU_REGION_ENABLE);
    
    /* Configure Region 4: Non-Secure RAM */
    configure_sau_region(4, 0x20010000,
                        0x2003FFFF,
                        SAU_REGION_ENABLE);
    
    /* Configure Region 5: Non-Secure Peripherals */
    configure_sau_region(5, 0x40000000,
                        0x4FFFFFFF,
                        SAU_REGION_ENABLE);
    
    /* Enable SAU */
    *((volatile uint32_t *)SAU_CTRL) = SAU_CTRL_ENABLE;
    
    return true;
}

/**
 * @brief Set peripheral security attribution
 */
bool set_peripheral_security(uint32_t peripheral_id, bool secure) {
    /* Peripheral security is controlled via PPC (Peripheral Protection Controller)
     * For EFR32MG26, this is vendor-specific */
    
    volatile uint32_t *ppc_base = (volatile uint32_t *)0x40030000;
    uint32_t reg_offset = peripheral_id / 32;
    uint32_t bit_offset = peripheral_id % 32;
    
    if (secure) {
        ppc_base[reg_offset] |= (1 << bit_offset);
    } else {
        ppc_base[reg_offset] &= ~(1 << bit_offset);
    }
    
    return true;
}

/**
 * @brief Configure interrupt security
 */
static void configure_interrupt_security(void) {
    /* Configure NVIC interrupts as Secure or Non-Secure */
    volatile uint32_t *nvic_itns = (volatile uint32_t *)NVIC_ITNS_BASE;
    
    /* Make critical interrupts Secure-only */
    /* Interrupt 0-31: Make secure (clear bits) */
    nvic_itns[0] = 0x00000000;
    
    /* Interrupt 32-63: Mixed security */
    nvic_itns[1] = 0xFFFF0000;  /* Upper half non-secure */
    
    /* Interrupt 64-95: Mostly non-secure */
    nvic_itns[2] = 0xFFFFFFFF;
}

/**
 * @brief Initialize TrustZone security
 */
bool trustzone_init(void) {
    /* Configure SAU regions */
    if (!configure_sau()) {
        return false;
    }
    
    /* Configure interrupt security */
    configure_interrupt_security();
    
    /* Configure critical peripherals as Secure */
    set_peripheral_security(0, true);   /* Crypto accelerator */
    set_peripheral_security(1, true);   /* TRNG */
    set_peripheral_security(2, true);   /* PUF */
    set_peripheral_security(3, true);   /* Secure Vault */
    
    /* Configure non-critical peripherals as Non-Secure */
    set_peripheral_security(10, false);  /* UART */
    set_peripheral_security(11, false);  /* SPI */
    set_peripheral_security(12, false);  /* I2C */
    
    /* Set security state */
    g_security_state = TZ_STATE_SECURE;
    
    return true;
}

/**
 * @brief Configure memory region security
 */
bool configure_memory_region(const tz_memory_region_t *region) {
    if (!region) {
        return false;
    }
    
    /* Find available SAU region */
    for (uint32_t i = 6; i < SAU_REGION_COUNT; i++) {
        /* Check if region is available (simplified check) */
        *((volatile uint32_t *)SAU_RNR) = i;
        uint32_t rlar = *((volatile uint32_t *)SAU_RLAR);
        
        if (!(rlar & SAU_REGION_ENABLE)) {
            /* Region is available */
            uint32_t sau_attr = SAU_REGION_ENABLE;
            
            if (region->attributes & TZ_ATTR_NSC) {
                sau_attr |= SAU_REGION_NSC;
            }
            
            return configure_sau_region(i, region->base_address,
                                       region->base_address + region->size - 1,
                                       sau_attr);
        }
    }
    
    return false;
}

/**
 * @brief Get current security state
 */
uint32_t get_security_state(void) {
    /* Read security state from control register */
    uint32_t control;
    __asm__ volatile ("MRS %0, CONTROL" : "=r" (control));
    
    /* Check nPRIV bit (bit 0) - not directly security state */
    /* In real implementation, would check CONTROL.nPRIV and CONTROL_NS */
    
    return g_security_state;
}

/**
 * @brief Enable secure world isolation
 */
bool enable_secure_isolation(void) {
    /* Already enabled during trustzone_init */
    /* This function can be used to re-enable if it was disabled */
    
    /* Enable SAU */
    *((volatile uint32_t *)SAU_CTRL) |= SAU_CTRL_ENABLE;
    
    return true;
}
