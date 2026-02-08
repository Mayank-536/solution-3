# EFR32MG26 Hardened Secure Boot Architecture

A comprehensive hardened secure boot implementation for Silicon Labs EFR32MG26 microcontroller with ARM Cortex-M33 and TrustZone-M.

## Features

### Core Security Components

✅ **Hardware Root of Trust**
- Secure Vault High with immutable RTSL
- One-time programmable root configuration
- Hardware-backed security foundation

✅ **Glitch-Resistant Control Flow**
- Layered token-based verification
- Triple redundant checks with random jitter
- Protection against fault injection attacks

✅ **TRNG-Based Random Jitter**
- Hardware True Random Number Generator
- Timing randomization for glitch resistance
- Configurable delay ranges

✅ **ACMP/IADC Tamper Detection**
- Voltage monitoring via Analog Comparator (1.7V-2.0V)
- Temperature monitoring via IADC (-40°C to 85°C)
- Preemptive responses: key erasure, device lock, reset

✅ **Anti-Rollback Protection**
- OTP (One-Time Programmable) monotonic counters
- Firmware version verification
- Irreversible version updates

✅ **Secure Debug Interface**
- Certificate-based authentication
- Device-bound certificates
- Time-limited access support

✅ **Measured Boot**
- SHA-256 measurement of all boot stages
- Signed JSON attestation reports
- Cryptographic boot verification

✅ **TrustZone Secure World**
- ARM TrustZone-M isolation
- SAU (Security Attribution Unit) configuration
- Secure/Non-Secure memory and peripheral separation

✅ **PUF-Based Key Wrapping**
- Physical Unclonable Function for unique keys
- Hardware-derived encryption keys
- No persistent key storage required

## Project Structure

```
.
├── include/              # Header files
│   ├── secure_boot.h
│   ├── crypto_primitives.h
│   ├── tamper_detection.h
│   ├── anti_rollback.h
│   ├── secure_debug.h
│   ├── attestation.h
│   └── trustzone.h
├── src/                  # Source files
│   ├── main.c
│   ├── bootloader/
│   │   ├── secure_boot.c
│   │   └── anti_rollback.c
│   ├── crypto/
│   │   └── crypto_primitives.c
│   ├── tamper/
│   │   └── tamper_detection.c
│   ├── debug/
│   │   └── secure_debug.c
│   ├── attestation/
│   │   └── attestation.c
│   └── trustzone/
│       └── trustzone.c
├── config/               # Configuration files
│   └── secure_boot_config.json
├── docs/                 # Documentation
│   ├── README.md
│   └── ARCHITECTURE.md
└── Makefile             # Build system

```

## Quick Start

### Prerequisites

- ARM GCC toolchain for Cortex-M33
- Make build system
- EFR32MG26 development board (for deployment)

### Building

```bash
# Build the project
make

# Clean build artifacts
make clean
```

### Output Files

- `build/secure_boot.elf` - ELF executable
- `build/secure_boot.hex` - Intel HEX format
- `build/secure_boot.bin` - Raw binary
- `build/secure_boot.map` - Memory map

## Configuration

Edit `config/secure_boot_config.json` to customize security parameters:

- Tamper detection thresholds
- TRNG jitter timing
- Boot measurement stages
- TrustZone memory regions

## Documentation

- **[README.md](docs/README.md)** - Detailed feature documentation
- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** - System architecture and diagrams

## Security Features Summary

| Feature | Implementation | Status |
|---------|---------------|--------|
| Hardware Root of Trust | Secure Vault High + Immutable RTSL | ✅ |
| Glitch Resistance | Layered tokens + Triple checks | ✅ |
| Random Jitter | TRNG-based timing randomization | ✅ |
| Voltage Tamper | ACMP monitoring (1.7V-2.0V) | ✅ |
| Temperature Tamper | IADC monitoring (-40°C to 85°C) | ✅ |
| Anti-Rollback | OTP monotonic counters | ✅ |
| Secure Debug | Certificate authentication | ✅ |
| Measured Boot | SHA-256 + JSON attestation | ✅ |
| TrustZone | Secure World isolation (SAU) | ✅ |
| PUF Key Wrapping | Hardware-derived keys | ✅ |

## Boot Sequence

1. Initialize TrustZone Secure World
2. Verify Hardware Root of Trust (RTSL)
3. Initialize cryptographic primitives (TRNG, PUF)
4. Enable tamper detection (ACMP/IADC)
5. Verify firmware version (anti-rollback)
6. Measure boot components (SHA-256)
7. Generate signed attestation (JSON)
8. Launch application

## License

Copyright (c) 2024. All rights reserved.

## Target Platform

- **MCU**: Silicon Labs EFR32MG26
- **Core**: ARM Cortex-M33 with TrustZone-M
- **Security**: Secure Vault High
- **Features**: TRNG, PUF, ACMP, IADC, SAU