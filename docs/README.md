# Hardened Secure Boot Architecture for EFR32MG26

## Overview

This implementation provides a comprehensive hardened secure boot architecture for the Silicon Labs EFR32MG26 microcontroller with ARM Cortex-M33 and TrustZone-M support.

## Security Features

### 1. Hardware Root of Trust
- **Secure Vault High**: Utilizes EFR32MG26's Secure Vault High security level
- **Immutable RTSL**: Root of Trust and Secure Loader locked during boot
- **One-time programmable**: RTSL configuration locked after initialization

### 2. Glitch-Resistant Control Flow
- **Layered Token Checks**: Multi-stage verification tokens throughout boot process
- **Triple Redundant Checks**: Critical decisions verified three times with jitter
- **Token Validation**: Each boot stage requires correct token from previous stage
- **Inverted Logic Checks**: Additional verification with complementary logic

### 3. TRNG-Based Random Jitter
- **Hardware TRNG**: Uses EFR32MG26's True Random Number Generator
- **Timing Randomization**: Random delays between critical operations
- **Glitch Detection**: Makes timing-based attacks unreliable
- **Configurable Range**: Min/max cycle counts for jitter delays

### 4. Tamper Detection System

#### Voltage Monitoring (ACMP)
- Analog Comparator monitors supply voltage
- Configurable thresholds (1.7V - 2.0V typical)
- Real-time detection of voltage glitching attempts
- Interrupt-driven monitoring

#### Temperature Monitoring (IADC)
- Incremental ADC with temperature sensor
- Operating range: -40°C to 85°C
- Detects environmental and physical tampering
- Continuous background monitoring

#### Preemptive Responses
- **Key Erasure**: Cryptographic keys wiped on tamper detection
- **Device Lock**: Permanent lock preventing further access
- **System Reset**: Immediate reset on glitch detection

### 5. Anti-Rollback Protection
- **OTP Counters**: One-Time Programmable monotonic counters
- **Version Verification**: Firmware version checked against stored counter
- **Irreversible Updates**: Counter increments are permanent
- **Rollback Prevention**: Older firmware versions rejected

### 6. Secure Debug Interface
- **Certificate Authentication**: Debug access requires signed certificate
- **Device Binding**: Certificates tied to unique device ID
- **Expiration Support**: Time-limited debug access
- **Default Locked**: Debug port locked by default

### 7. Measured Boot
- **SHA-256 Measurements**: Each boot stage measured
- **JSON Attestation**: Structured attestation report
- **Digital Signature**: Attestation signed with PUF-derived key
- **Boot Stages**:
  - Bootloader
  - Secure Vault
  - RTSL
  - Firmware
  - Application

### 8. TrustZone Secure World
- **ARM TrustZone-M**: Cortex-M33 security extensions
- **SAU Configuration**: Security Attribution Unit regions
- **Secure/Non-Secure**: Memory and peripheral isolation
- **NSC Regions**: Non-Secure Callable entry points
- **Interrupt Security**: NVIC security attribution

### 9. PUF-Based Key Wrapping
- **Physical Unclonable Function**: Hardware-based unique keys
- **Key Derivation**: Generates device-unique encryption keys
- **Key Wrapping**: AES-KW for key encryption
- **No Key Storage**: Keys derived on-demand from silicon
- **Tamper Evident**: Keys destroyed if PUF tampered

## Boot Sequence

```
1. Initialize TrustZone Secure World (SAU, secure/non-secure split)
2. Initialize TRNG for random jitter generation
3. Initialize PUF for key derivation
4. Verify Hardware Root of Trust (RTSL)
   - Check RTSL status register
   - Verify immutability
   - Lock RTSL configuration
5. Initialize tamper detection
   - Configure ACMP for voltage monitoring
   - Configure IADC for temperature monitoring
   - Enable continuous monitoring
6. Verify firmware version (anti-rollback)
   - Read OTP counter
   - Verify firmware version >= counter
   - Update counter if needed
7. Perform control flow checks
   - Verify boot stage tokens
   - Triple redundant verification
   - Random jitter between checks
8. Generate boot measurements
   - Measure each boot component
   - Calculate SHA-256 hashes
9. Generate signed attestation
   - Create JSON attestation report
   - Sign with PUF-derived key
10. Transfer control to application
```

## Memory Map

### Secure World
- **Code**: 0x00000000 - 0x0003FFFF (256KB)
- **RAM**: 0x20000000 - 0x2000FFFF (64KB)
- **NSC**: 0x10000000 - 0x10000FFF (4KB)

### Non-Secure World
- **Code**: 0x00040000 - 0x000FFFFF
- **RAM**: 0x20010000 - 0x2003FFFF
- **Peripherals**: 0x40000000 - 0x4FFFFFFF

### Secure Peripherals
- Secure Vault: 0x4C021000
- TRNG: 0x4C022000
- PUF: 0x4C023000
- AES: 0x4C024000
- Debug Control: 0x4C025000

## API Reference

See individual header files in `include/` directory:
- `secure_boot.h` - Main boot functions
- `crypto_primitives.h` - TRNG, PUF, signatures
- `tamper_detection.h` - ACMP/IADC monitoring
- `anti_rollback.h` - OTP counter management
- `secure_debug.h` - Certificate authentication
- `attestation.h` - Measured boot
- `trustzone.h` - TrustZone configuration

## Building

```bash
make clean
make
```

Requires ARM GCC toolchain for Cortex-M33.

## Configuration

Edit `config/secure_boot_config.json` to customize:
- Voltage/temperature thresholds
- Jitter timing parameters
- Boot stages
- Security features

## Security Considerations

1. **Private Keys**: Store signature verification keys in Secure Vault
2. **Debug Certificates**: Protect certificate private keys
3. **OTP Programming**: OTP writes are irreversible
4. **Tamper Response**: Understand key erasure implications
5. **TrustZone**: Properly configure SAU regions for your application

## License

Copyright (c) 2024. All rights reserved.
