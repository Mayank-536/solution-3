/**
 * @file tamper_detection.c
 * @brief Implementation of ACMP/IADC tamper detection
 */

#include "../../include/tamper_detection.h"
#include <string.h>

/* ACMP (Analog Comparator) registers */
#define ACMP_BASE                 0x40078000
#define ACMP_CTRL_REG             (ACMP_BASE + 0x000)
#define ACMP_INPUTSEL_REG         (ACMP_BASE + 0x004)
#define ACMP_STATUS_REG           (ACMP_BASE + 0x008)
#define ACMP_IF_REG               (ACMP_BASE + 0x00C)

/* IADC (Incremental ADC) registers */
#define IADC_BASE                 0x40080000
#define IADC_CTRL_REG             (IADC_BASE + 0x000)
#define IADC_CFG_REG              (IADC_BASE + 0x008)
#define IADC_DATA_REG             (IADC_BASE + 0x010)
#define IADC_STATUS_REG           (IADC_BASE + 0x014)

/* Control bits */
#define ACMP_ENABLE               0x00000001
#define ACMP_VOLTAGE_TRIGGER      0x00000010
#define IADC_ENABLE               0x00000001
#define IADC_TEMP_SENSOR          0x00000100

/* Tamper response actions */
#define TAMPER_ACTION_ERASE_KEYS  0x01
#define TAMPER_ACTION_RESET       0x02
#define TAMPER_ACTION_LOCK        0x04

/* Global tamper state */
static volatile uint32_t g_tamper_events = TAMPER_NONE;
static volatile bool g_tamper_monitoring_enabled = false;

/**
 * @brief Initialize ACMP for voltage monitoring
 */
bool acmp_init(void) {
    /* Configure ACMP for voltage monitoring */
    *((volatile uint32_t *)ACMP_CTRL_REG) = ACMP_ENABLE | ACMP_VOLTAGE_TRIGGER;
    
    /* Set voltage thresholds */
    *((volatile uint32_t *)ACMP_INPUTSEL_REG) = 
        ((VOLTAGE_MIN_MV & 0xFFF) << 0) | ((VOLTAGE_MAX_MV & 0xFFF) << 16);
    
    return true;
}

/**
 * @brief Initialize IADC for temperature monitoring
 */
bool iadc_init(void) {
    /* Enable IADC with temperature sensor */
    *((volatile uint32_t *)IADC_CTRL_REG) = IADC_ENABLE | IADC_TEMP_SENSOR;
    
    /* Configure for continuous conversion */
    *((volatile uint32_t *)IADC_CFG_REG) = 0x00000001;
    
    return true;
}

/**
 * @brief Initialize tamper detection system
 */
bool tamper_detection_init(void) {
    /* Initialize ACMP */
    if (!acmp_init()) {
        return false;
    }
    
    /* Initialize IADC */
    if (!iadc_init()) {
        return false;
    }
    
    /* Clear tamper events */
    g_tamper_events = TAMPER_NONE;
    
    return true;
}

/**
 * @brief Read current supply voltage
 */
uint32_t read_supply_voltage(void) {
    /* Read ACMP status register */
    uint32_t status = *((volatile uint32_t *)ACMP_STATUS_REG);
    
    /* Extract voltage reading (simplified) */
    /* In real implementation, this would read actual ADC value */
    uint32_t voltage_mv = (status & 0xFFF);
    
    /* Return voltage in millivolts */
    return voltage_mv;
}

/**
 * @brief Read current temperature
 */
int32_t read_temperature(void) {
    /* Trigger temperature conversion */
    *((volatile uint32_t *)IADC_CTRL_REG) |= 0x00000002;
    
    /* Wait for conversion to complete */
    uint32_t timeout = 10000;
    while (timeout--) {
        if (*((volatile uint32_t *)IADC_STATUS_REG) & 0x01) {
            break;
        }
    }
    
    /* Read temperature data */
    uint32_t data = *((volatile uint32_t *)IADC_DATA_REG);
    
    /* Convert to Celsius (simplified formula) */
    /* In real implementation, this would use calibration data */
    int32_t temp_c = (int32_t)((data - 500) / 10);
    
    return temp_c;
}

/**
 * @brief Check for tamper events
 */
uint32_t check_tamper_events(void) {
    uint32_t events = TAMPER_NONE;
    
    /* Check voltage */
    uint32_t voltage = read_supply_voltage();
    if (voltage < VOLTAGE_MIN_MV) {
        events |= TAMPER_VOLTAGE_LOW;
    }
    if (voltage > VOLTAGE_MAX_MV) {
        events |= TAMPER_VOLTAGE_HIGH;
    }
    
    /* Check temperature */
    int32_t temperature = read_temperature();
    if (temperature < TEMP_MIN_C) {
        events |= TAMPER_TEMP_LOW;
    }
    if (temperature > TEMP_MAX_C) {
        events |= TAMPER_TEMP_HIGH;
    }
    
    /* Check for glitch attempts via interrupt flags */
    uint32_t acmp_if = *((volatile uint32_t *)ACMP_IF_REG);
    if (acmp_if & 0xFF00) {  /* Unexpected interrupts indicate glitch */
        events |= TAMPER_GLITCH_DETECTED;
    }
    
    /* Update global tamper state */
    g_tamper_events |= events;
    
    return events;
}

/**
 * @brief Erase cryptographic keys from memory
 */
static void erase_cryptographic_keys(void) {
    /* In real implementation, this would securely erase keys from:
     * - Key storage memory
     * - PUF-derived keys
     * - Session keys
     * - Any cached cryptographic material
     */
    
    /* Clear key storage regions (example addresses) */
    volatile uint32_t *key_storage = (volatile uint32_t *)0x20000000;
    for (int i = 0; i < 256; i++) {
        key_storage[i] = 0;
    }
}

/**
 * @brief Lock device to prevent further access
 */
static void lock_device(void) {
    /* Set device lock bit in Secure Vault */
    volatile uint32_t *lock_reg = (volatile uint32_t *)0x4C021010;
    *lock_reg = 0xDEADBEEF;  /* Lock magic value */
}

/**
 * @brief Reset device
 */
static void reset_device(void) {
    /* Trigger system reset via SCB */
    volatile uint32_t *aircr = (volatile uint32_t *)0xE000ED0C;
    *aircr = 0x05FA0004;  /* SYSRESETREQ */
}

/**
 * @brief Handle tamper event with preemptive response
 */
void handle_tamper_event(uint32_t tamper_events) {
    /* Log tamper event (in real system, this would be persistent) */
    g_tamper_events = tamper_events;
    
    /* Determine response based on severity */
    uint32_t action = 0;
    
    if (tamper_events & (TAMPER_VOLTAGE_LOW | TAMPER_VOLTAGE_HIGH)) {
        /* Voltage tamper - erase keys and lock */
        action |= TAMPER_ACTION_ERASE_KEYS | TAMPER_ACTION_LOCK;
    }
    
    if (tamper_events & (TAMPER_TEMP_LOW | TAMPER_TEMP_HIGH)) {
        /* Temperature tamper - may be environmental, just lock */
        action |= TAMPER_ACTION_LOCK;
    }
    
    if (tamper_events & TAMPER_GLITCH_DETECTED) {
        /* Glitch detected - severe threat, erase keys and reset */
        action |= TAMPER_ACTION_ERASE_KEYS | TAMPER_ACTION_RESET;
    }
    
    /* Execute preemptive responses */
    if (action & TAMPER_ACTION_ERASE_KEYS) {
        erase_cryptographic_keys();
    }
    
    if (action & TAMPER_ACTION_LOCK) {
        lock_device();
    }
    
    if (action & TAMPER_ACTION_RESET) {
        reset_device();
    }
}

/**
 * @brief Enable continuous tamper monitoring
 */
void enable_tamper_monitoring(void) {
    g_tamper_monitoring_enabled = true;
    
    /* Enable ACMP interrupts for voltage monitoring */
    *((volatile uint32_t *)ACMP_CTRL_REG) |= 0x00000100;
    
    /* Enable IADC interrupts for temperature monitoring */
    *((volatile uint32_t *)IADC_CTRL_REG) |= 0x00000100;
}

/**
 * @brief Disable tamper monitoring
 */
void disable_tamper_monitoring(void) {
    g_tamper_monitoring_enabled = false;
    
    /* Disable interrupts */
    *((volatile uint32_t *)ACMP_CTRL_REG) &= ~0x00000100;
    *((volatile uint32_t *)IADC_CTRL_REG) &= ~0x00000100;
}

/**
 * @brief ACMP interrupt handler (called on voltage events)
 */
void ACMP_IRQHandler(void) {
    if (g_tamper_monitoring_enabled) {
        uint32_t events = check_tamper_events();
        if (events != TAMPER_NONE) {
            handle_tamper_event(events);
        }
    }
    
    /* Clear interrupt flag */
    *((volatile uint32_t *)ACMP_IF_REG) = 0xFFFFFFFF;
}

/**
 * @brief IADC interrupt handler (called on temperature events)
 */
void IADC_IRQHandler(void) {
    if (g_tamper_monitoring_enabled) {
        uint32_t events = check_tamper_events();
        if (events != TAMPER_NONE) {
            handle_tamper_event(events);
        }
    }
    
    /* Clear interrupt flag */
    *((volatile uint32_t *)IADC_STATUS_REG) = 0xFFFFFFFF;
}
