/**
 * @file trustzone.h
 * @brief TrustZone Secure World isolation for EFR32MG26
 */

#ifndef TRUSTZONE_H
#define TRUSTZONE_H

#include <stdint.h>
#include <stdbool.h>

/* TrustZone security states */
#define TZ_STATE_SECURE           0x01
#define TZ_STATE_NON_SECURE       0x00

/* Memory region attributes */
#define TZ_ATTR_SECURE            0x01
#define TZ_ATTR_NON_SECURE        0x02
#define TZ_ATTR_NSC               0x04  /* Non-Secure Callable */

/* Memory region structure */
typedef struct {
    uint32_t base_address;
    uint32_t size;
    uint32_t attributes;
} tz_memory_region_t;

/**
 * @brief Initialize TrustZone security
 * @return true on success, false on failure
 */
bool trustzone_init(void);

/**
 * @brief Configure memory region security
 * @param region Memory region configuration
 * @return true on success, false on failure
 */
bool configure_memory_region(const tz_memory_region_t *region);

/**
 * @brief Set peripheral security attribution
 * @param peripheral_id Peripheral identifier
 * @param secure true for secure, false for non-secure
 * @return true on success, false on failure
 */
bool set_peripheral_security(uint32_t peripheral_id, bool secure);

/**
 * @brief Get current security state
 * @return TZ_STATE_SECURE or TZ_STATE_NON_SECURE
 */
uint32_t get_security_state(void);

/**
 * @brief Enable secure world isolation
 * @return true on success, false on failure
 */
bool enable_secure_isolation(void);

/**
 * @brief Configure SAU (Security Attribution Unit)
 * @return true on success, false on failure
 */
bool configure_sau(void);

#endif /* TRUSTZONE_H */
