/**
 * @file tamper_detection.c
 * @brief Active Tamper Detection Implementation for EFR32MG26
 * 
 * Implements preemptive anti-tamper monitoring using ACMP for voltage
 * glitch detection and IADC for temperature anomaly detection.
 */

#include "tamper_detection.h"
#include <string.h>

/* Global tamper context */
static tamper_context_t g_tamper_context;
static acmp_config_t g_acmp_config;
static iadc_config_t g_iadc_config;

/* Simulated peripheral registers (in production, use actual EFR32 registers) */
static volatile uint32_t ACMP_STATUS = 0;
static volatile uint32_t IADC_TEMP_VALUE = 25;

/**
 * @brief Initialize ACMP peripheral for voltage monitoring
 */
bool acmp_init(const acmp_config_t *config) {
    if (config == NULL) {
        return false;
    }
    
    /* Store configuration */
    memcpy(&g_acmp_config, config, sizeof(acmp_config_t));
    
    /* In production: Configure EFR32 ACMP peripheral
     * - Set up voltage comparator thresholds
     * - Configure hysteresis
     * - Enable interrupts
     * - Connect to supply voltage monitoring
     */
    
    /* Configure ACMP for voltage monitoring
     * Reference: EFR32MG26 Reference Manual, ACMP chapter */
    
    /* Enable ACMP clock */
    /* CMU->CLKEN0 |= CMU_CLKEN0_ACMP0; */
    
    /* Configure ACMP input channels */
    /* ACMP0->INPUTSEL = ACMP_INPUTSEL_POSSEL_AVDD | ACMP_INPUTSEL_NEGSEL_VREFDIV2V5; */
    
    /* Set threshold levels */
    /* Use ACMP capsense mode for precise voltage detection */
    
    /* Enable edge interrupts for glitch detection */
    if (config->interrupt_enabled) {
        /* ACMP0->IEN = ACMP_IEN_EDGE | ACMP_IEN_WARMUP; */
    }
    
    /* Enable ACMP */
    /* ACMP0->EN = ACMP_EN_EN; */
    
    return true;
}

/**
 * @brief Initialize IADC peripheral for temperature monitoring
 */
bool iadc_init(const iadc_config_t *config) {
    if (config == NULL) {
        return false;
    }
    
    /* Store configuration */
    memcpy(&g_iadc_config, config, sizeof(iadc_config_t));
    
    /* In production: Configure EFR32 IADC peripheral
     * - Set up temperature sensor input
     * - Configure sample rate and resolution
     * - Set threshold comparators
     * - Enable continuous monitoring if requested
     */
    
    /* Configure IADC for temperature monitoring
     * Reference: EFR32MG26 Reference Manual, IADC chapter */
    
    /* Enable IADC clock */
    /* CMU->CLKEN0 |= CMU_CLKEN0_IADC0; */
    
    /* Configure IADC for temperature sensor */
    /* IADC0->SINGLEFIFOCFG = IADC_SINGLEFIFOCFG_ALIGNMENT_RIGHT12; */
    
    /* Select internal temperature sensor */
    /* IADC0->SCAN0CFG = IADC_SCAN0CFG_FIFODVL_VALID1; */
    
    /* Set sample rate */
    /* IADC0->TIMER = config->sample_rate; */
    
    /* Configure thresholds for temperature monitoring */
    /* IADC0->CMPTHR = (config->temp_high_threshold << 16) | config->temp_low_threshold; */
    
    /* Enable continuous mode if requested */
    if (config->continuous_mode) {
        /* IADC0->TRIGGER = IADC_TRIGGER_SCANTRIGSEL_TIMER; */
    }
    
    /* Enable IADC */
    /* IADC0->EN = IADC_EN_EN; */
    
    return true;
}

/**
 * @brief Start tamper detection monitoring
 */
bool tamper_detection_start(tamper_context_t *context) {
    if (context == NULL) {
        return false;
    }
    
    /* Initialize context */
    memset(context, 0, sizeof(tamper_context_t));
    g_tamper_context = *context;
    
    /* Configure ACMP for voltage monitoring */
    acmp_config_t acmp_cfg = {
        .low_threshold = VOLTAGE_THRESHOLD_LOW_MV,
        .high_threshold = VOLTAGE_THRESHOLD_HIGH_MV,
        .hysteresis = 50,  /* 50mV hysteresis */
        .interrupt_enabled = true
    };
    
    if (!acmp_init(&acmp_cfg)) {
        return false;
    }
    
    /* Configure IADC for temperature monitoring */
    iadc_config_t iadc_cfg = {
        .sample_rate = 1000,  /* 1kHz sampling */
        .temp_low_threshold = TEMP_THRESHOLD_LOW_C,
        .temp_high_threshold = TEMP_THRESHOLD_HIGH_C,
        .continuous_mode = true
    };
    
    if (!iadc_init(&iadc_cfg)) {
        return false;
    }
    
    /* Set initial measurements */
    g_tamper_context.last_voltage_mv = VOLTAGE_NOMINAL_MV;
    g_tamper_context.last_temp_c = TEMP_NOMINAL_C;
    
    return true;
}

/**
 * @brief Check for tamper events
 */
uint32_t check_tamper_events(tamper_context_t *context) {
    uint32_t events = TAMPER_EVENT_NONE;
    
    /* In production: Read actual ACMP and IADC status registers */
    
    /* Check voltage levels using ACMP */
    /* Simulated voltage check - in production, read ACMP->STATUS */
    uint32_t current_voltage = VOLTAGE_NOMINAL_MV;
    
    if (current_voltage < g_acmp_config.low_threshold) {
        events |= TAMPER_EVENT_VOLTAGE_LOW;
        g_tamper_context.event_count++;
    }
    
    if (current_voltage > g_acmp_config.high_threshold) {
        events |= TAMPER_EVENT_VOLTAGE_HIGH;
        g_tamper_context.event_count++;
    }
    
    /* Check for rapid voltage changes (glitch detection) */
    if (current_voltage != g_tamper_context.last_voltage_mv) {
        int32_t voltage_delta = current_voltage - g_tamper_context.last_voltage_mv;
        if (voltage_delta > 200 || voltage_delta < -200) {  /* >200mV change */
            events |= TAMPER_EVENT_GLITCH;
            g_tamper_context.event_count++;
        }
    }
    
    g_tamper_context.last_voltage_mv = current_voltage;
    
    /* Check temperature using IADC */
    /* Simulated temperature check - in production, read IADC->SINGLEDATA */
    int32_t current_temp = TEMP_NOMINAL_C;
    
    if (current_temp < g_iadc_config.temp_low_threshold) {
        events |= TAMPER_EVENT_TEMP_LOW;
        g_tamper_context.event_count++;
    }
    
    if (current_temp > g_iadc_config.temp_high_threshold) {
        events |= TAMPER_EVENT_TEMP_HIGH;
        g_tamper_context.event_count++;
    }
    
    g_tamper_context.last_temp_c = current_temp;
    
    /* Update context */
    g_tamper_context.event_flags |= events;
    
    if (context != NULL) {
        *context = g_tamper_context;
    }
    
    return events;
}

/**
 * @brief Execute anti-tamper response
 */
void execute_tamper_response(uint32_t event_flags) {
    /* Immediate response to tamper events */
    
    if (event_flags & TAMPER_EVENT_VOLTAGE_LOW) {
        /* Voltage glitch detected - potential attack */
        /* Actions:
         * 1. Halt boot process immediately
         * 2. Zeroize sensitive data in RAM
         * 3. Lock debug interfaces
         * 4. Log event to secure storage
         */
    }
    
    if (event_flags & TAMPER_EVENT_VOLTAGE_HIGH) {
        /* Overvoltage detected */
        /* Similar protective actions */
    }
    
    if (event_flags & TAMPER_EVENT_GLITCH) {
        /* Rapid voltage change - likely glitch attack */
        /* This is critical - execute maximum response:
         * 1. Immediate system halt
         * 2. Disable all peripherals
         * 3. Zeroize all RAM
         * 4. Trigger hardware tamper response if available
         */
        
        /* In production: Trigger EFR32 tamper response */
        /* SMU->IF = SMU_IF_PPUSEC; */
    }
    
    if (event_flags & TAMPER_EVENT_TEMP_LOW) {
        /* Extreme cold - possible freeze attack */
        /* Monitor and prepare countermeasures */
    }
    
    if (event_flags & TAMPER_EVENT_TEMP_HIGH) {
        /* Extreme heat - possible environmental attack */
        /* Monitor and prepare countermeasures */
    }
    
    if (event_flags & TAMPER_EVENT_CLOCK_ANOMALY) {
        /* Clock glitching detected */
        /* Halt immediately - this is a direct attack */
    }
    
    /* If any critical event detected, halt system */
    if (event_flags & (TAMPER_EVENT_GLITCH | TAMPER_EVENT_CLOCK_ANOMALY)) {
        /* Enter infinite loop to prevent further execution */
        while (1) {
            /* Wait for watchdog reset */
            __asm__ volatile ("nop");
        }
    }
}

/**
 * @brief ACMP interrupt handler for voltage glitch detection
 */
void acmp_irq_handler(void) {
    /* In production: Clear interrupt flags */
    /* uint32_t flags = ACMP0->IF; */
    /* ACMP0->IF = flags; */
    
    /* Check for edge events indicating voltage change */
    /* if (flags & ACMP_IF_EDGE) { */
        uint32_t events = check_tamper_events(&g_tamper_context);
        if (events != TAMPER_EVENT_NONE) {
            execute_tamper_response(events);
        }
    /* } */
}

/**
 * @brief IADC interrupt handler for temperature anomaly detection
 */
void iadc_irq_handler(void) {
    /* In production: Clear interrupt flags */
    /* uint32_t flags = IADC0->IF; */
    /* IADC0->IF = flags; */
    
    /* Check for threshold events */
    /* if (flags & IADC_IF_SINGLECMP) { */
        uint32_t events = check_tamper_events(&g_tamper_context);
        if (events != TAMPER_EVENT_NONE) {
            execute_tamper_response(events);
        }
    /* } */
}
