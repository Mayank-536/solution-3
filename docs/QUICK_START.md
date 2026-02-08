# Quick Start Guide

## Introduction

This guide provides a quick introduction to building and deploying the EFR32MG26 Secure Boot system.

## Prerequisites

Before you begin, ensure you have:

1. **Development Environment**
   - ARM GCC Toolchain (`arm-none-eabi-gcc`)
   - Make utility
   - Git

2. **Hardware** (for deployment)
   - EFR32MG26 development board
   - JTAG/SWD debugger (J-Link, SEGGER, etc.)
   - Silicon Labs Simplicity Studio

3. **Knowledge**
   - Basic understanding of embedded C
   - Familiarity with ARM Cortex-M architecture
   - Understanding of secure boot concepts

## Quick Start

### 1. Clone the Repository

```bash
git clone https://github.com/Mayank-536/solution-3.git
cd solution-3
```

### 2. Build the Project

```bash
# Build everything
make all

# Or build individual components
make bootloader
make tamper
make attestation
```

### 3. Verify Build Output

After successful build:
```
build/
├── bin/
│   ├── efr32mg26_secure_boot.elf
│   ├── efr32mg26_secure_boot.bin
│   └── efr32mg26_secure_boot.hex
└── obj/
    └── [object files]
```

### 4. Flash to Device (Production)

```bash
# Using J-Link
JLinkExe -device EFR32MG26B310F3200 -if SWD -speed 4000
> loadbin build/bin/efr32mg26_secure_boot.bin 0x0
> verifybin build/bin/efr32mg26_secure_boot.bin 0x0
> r
> g
> exit
```

## Configuration

### TrustZone Memory Regions

Edit `config/example_config.c` to configure memory regions:

```c
static const trustzone_config_t example_tz_config = {
    .flash_secure = {
        .start_address = 0x00000000,
        .end_address = 0x00040000,  // Adjust as needed
        .region_type = REGION_TYPE_SECURE,
        .enable = true
    },
    // ... more regions
};
```

### Tamper Detection Thresholds

Adjust in `include/tamper_detection.h`:

```c
#define VOLTAGE_THRESHOLD_LOW_MV    2700  // Minimum voltage
#define VOLTAGE_THRESHOLD_HIGH_MV   3600  // Maximum voltage
#define TEMP_THRESHOLD_LOW_C        -20   // Minimum temperature
#define TEMP_THRESHOLD_HIGH_C       85    // Maximum temperature
```

## Code Examples

### Minimal Secure Boot

```c
#include "secure_boot.h"

int main(void) {
    // Initialize and execute secure boot
    boot_status_t status = execute_secure_boot();
    
    if (status == BOOT_STATUS_SUCCESS) {
        // Boot successful - jump to application
        // In production: transition_to_nonsecure(app_addr, stack_addr);
    } else {
        // Boot failed - halt system
        while(1);
    }
    
    return 0;
}
```

### Complete Initialization

```c
#include "secure_boot.h"
#include "tamper_detection.h"
#include "attestation.h"
#include "trustzone.h"
#include "puf.h"

int main(void) {
    // 1. Initialize TrustZone
    if (!trustzone_init(&tz_config)) {
        while(1);  // Fatal error
    }
    
    // 2. Initialize PUF and enroll
    puf_init();
    puf_enroll();
    
    // 3. Start tamper detection
    tamper_context_t tamper_ctx;
    tamper_detection_start(&tamper_ctx);
    
    // 4. Initialize attestation
    attestation_init();
    
    // 5. Execute secure boot
    boot_status_t status = execute_secure_boot();
    
    if (status == BOOT_STATUS_SUCCESS) {
        // 6. Generate attestation report
        attestation_report_t report;
        uint8_t nonce[16] = {0};
        
        generate_attestation_report(nonce, &report);
        sign_attestation_report(&report);
        
        // 7. Export report
        char json[4096];
        export_report_json(&report, json, sizeof(json));
        
        // 8. Transition to application
        // transition_to_nonsecure(0x00040000, 0x20020000);
    }
    
    return 0;
}
```

## Testing

### Verify Glitch Resistance

The multi-layer token verification can be tested by simulating glitches:

```c
// In test environment only!
void test_glitch_resistance(void) {
    boot_context_t ctx;
    
    // Initialize with correct tokens
    ctx.verification_tokens[0] = TOKEN_LAYER_1;
    ctx.verification_tokens[1] = TOKEN_LAYER_2;
    ctx.verification_tokens[2] = TOKEN_LAYER_3;
    ctx.verification_tokens[3] = TOKEN_LAYER_4;
    
    // This should succeed
    uint32_t result = verify_layered_tokens(&ctx);
    assert(result == TOKEN_STATE_ALL_VALID);
    
    // Corrupt one token - should fail
    ctx.verification_tokens[1] = 0x12345678;
    result = verify_layered_tokens(&ctx);
    assert(result == TOKEN_STATE_INVALID);
}
```

### Test Tamper Detection

```c
void test_tamper_detection(void) {
    tamper_context_t ctx;
    
    // Start monitoring
    tamper_detection_start(&ctx);
    
    // Check for events (should be none initially)
    uint32_t events = check_tamper_events(&ctx);
    assert(events == TAMPER_EVENT_NONE);
    
    // In production: trigger actual tamper condition
    // and verify response
}
```

### Test Anti-Rollback

```c
void test_anti_rollback(void) {
    anti_rollback_init();
    
    version_t current_ver;
    read_otp_version(&current_ver);
    
    // Try to install newer version - should pass
    version_t new_ver = {
        .major = current_ver.major + 1,
        .minor = 0,
        .patch = 0
    };
    
    rollback_status_t status = check_version_rollback(&new_ver);
    assert(status == ROLLBACK_VERSION_HIGHER);
    
    // Try to install older version - should fail
    version_t old_ver = {
        .major = current_ver.major - 1,
        .minor = 0,
        .patch = 0
    };
    
    status = check_version_rollback(&old_ver);
    assert(status == ROLLBACK_CHECK_FAIL);
}
```

## Debugging

### Enable Debug Output

Add to your main.c:

```c
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
#define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif
```

### Common Issues

**Issue**: Build fails with "arm-none-eabi-gcc: command not found"
- **Solution**: Install ARM GCC toolchain or add to PATH

**Issue**: Linker error about undefined references
- **Solution**: Ensure all source files are included in Makefile

**Issue**: Device doesn't boot after flashing
- **Solution**: Verify correct start address (0x00000000 for bootloader)

**Issue**: Tamper detection triggers immediately
- **Solution**: Adjust voltage/temperature thresholds for your environment

## Production Deployment

### Security Checklist

Before deploying to production:

- [ ] Program OTP with production keys
- [ ] Set correct firmware version in OTP
- [ ] Lock debug interfaces
- [ ] Enable all tamper detection
- [ ] Configure TrustZone regions
- [ ] Test attestation flow
- [ ] Verify anti-rollback enforcement
- [ ] Burn security fuses
- [ ] Perform security audit
- [ ] Test against glitch attacks

### OTP Programming

```c
// WARNING: OTP can only be written ONCE!
// This is a ONE-TIME operation!

void production_init(void) {
    // Set initial version
    version_t initial_version = {
        .major = 1,
        .minor = 0,
        .patch = 0
    };
    
    // Write to OTP - IRREVERSIBLE!
    write_otp_version(&initial_version);
    
    // Program production keys
    // ... key programming code ...
    
    // Lock debug interface
    // ... lock code ...
}
```

## Performance Optimization

### Reduce Boot Time

1. **Reduce jitter iterations** (trade-off with security):
```c
// In inject_random_jitter()
volatile uint32_t delay_cycles = (seed % 100) + 10;  // Faster, less secure
```

2. **Cache signature verification results** (if multiple boots):
```c
static uint32_t cached_signature = 0;
if (cached_signature == VALID_TOKEN) {
    // Skip re-verification
}
```

3. **Optimize attestation report size**:
```c
// Use CBOR instead of JSON for smaller reports
export_report_cbor(&report, cbor_buffer, sizeof(cbor_buffer));
```

## Next Steps

1. Read [ARCHITECTURE.md](docs/ARCHITECTURE.md) for detailed design
2. Review [VALIDATION_REPORT.md](validation_report/VALIDATION_REPORT.md) for security analysis
3. Customize configuration for your application
4. Perform security testing
5. Plan certification path (FIPS 140-3, Common Criteria)

## Support

- **Documentation**: See `/docs` directory
- **Issues**: Open GitHub issue
- **Security**: Email security team (do not disclose publicly)

## License

See LICENSE file for details.

---

**Last Updated**: 2026-02-08  
**Document Version**: 1.0
