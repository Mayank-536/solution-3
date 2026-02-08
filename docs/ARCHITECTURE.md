## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                  EFR32MG26 Secure Boot Architecture             │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                      Hardware Root of Trust                      │
│  ┌──────────────┐  ┌──────────────┐  ┌───────────────────┐     │
│  │ Secure Vault │  │ Immutable    │  │   PUF Silicon     │     │
│  │    High      │  │    RTSL      │  │   Fingerprint     │     │
│  └──────────────┘  └──────────────┘  └───────────────────┘     │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                      TrustZone Secure World                      │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ SAU: Security Attribution Unit                          │   │
│  │ - Secure Code/RAM regions                              │   │
│  │ - Non-Secure Callable (NSC) gateway                    │   │
│  │ - Peripheral security assignment                       │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Cryptographic Primitives                      │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐  ┌──────────┐   │
│  │   TRNG   │  │   PUF    │  │  AES Engine  │  │  SHA-256 │   │
│  │  Random  │  │   Key    │  │ Key Wrapping │  │   Hash   │   │
│  │  Jitter  │  │ Deriv.   │  │              │  │          │   │
│  └──────────┘  └──────────┘  └──────────────┘  └──────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                     Tamper Detection System                      │
│  ┌─────────────────────┐         ┌──────────────────────┐      │
│  │  ACMP Voltage Mon.  │         │  IADC Temperature    │      │
│  │  - Min: 1.7V        │         │  - Range: -40 to 85°C│      │
│  │  - Max: 2.0V        │         │  - Continuous ADC    │      │
│  │  - Glitch detect    │         │  - Sensor reading    │      │
│  └─────────────────────┘         └──────────────────────┘      │
│                              │                                   │
│                              ▼                                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │         Preemptive Tamper Response                      │   │
│  │  - Erase cryptographic keys                            │   │
│  │  - Lock device permanently                             │   │
│  │  - Trigger system reset                                │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│              Glitch-Resistant Control Flow                       │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ Token Progression:                                      │   │
│  │ INIT → RTSL_OK → TAMPER_OK → ROLLBACK_OK → SIG_OK      │   │
│  │                                                         │   │
│  │ Each transition:                                        │   │
│  │ - Triple redundant check                               │   │
│  │ - TRNG random jitter                                   │   │
│  │ - Inverted logic verification                          │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                  Anti-Rollback Protection                        │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ OTP Counter (One-Time Programmable)                     │   │
│  │ - Monotonic counter in OTP memory                      │   │
│  │ - Firmware version >= counter required                 │   │
│  │ - Auto-increment on version update                     │   │
│  │ - Irreversible write operations                        │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Measured Boot & Attestation                   │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ Boot Stage Measurements:                                │   │
│  │ 1. Bootloader    → SHA-256 hash                        │   │
│  │ 2. Secure Vault  → SHA-256 hash                        │   │
│  │ 3. RTSL          → SHA-256 hash                        │   │
│  │ 4. Firmware      → SHA-256 hash                        │   │
│  │ 5. Application   → SHA-256 hash                        │   │
│  │                                                         │   │
│  │ JSON Attestation Report:                                │   │
│  │ - All measurements in structured format                │   │
│  │ - Timestamp and version info                           │   │
│  │ - Signed with PUF-derived key                          │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                   Secure Debug Interface                         │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ Certificate-Based Authentication:                       │   │
│  │ - Debug port locked by default                         │   │
│  │ - Certificate bound to device unique ID                │   │
│  │ - Optional expiration time                             │   │
│  │ - Signature verification with public key               │   │
│  │ - Temporary or permanent unlock modes                  │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Application Launch                            │
│           Secure Boot Complete - All Checks Passed               │
└─────────────────────────────────────────────────────────────────┘
```

## Security Layers

1. **Hardware Layer**: Secure Vault High, RTSL, PUF
2. **Isolation Layer**: TrustZone-M, SAU regions
3. **Cryptographic Layer**: TRNG, PUF key derivation, signatures
4. **Detection Layer**: ACMP/IADC tamper monitoring
5. **Protection Layer**: Anti-rollback, glitch resistance
6. **Verification Layer**: Measured boot, attestation
7. **Access Control Layer**: Secure debug authentication

## Data Flow

```
Power-On Reset
     │
     ▼
[RTSL Verification] ──────────────────┐
     │                                 │
     ▼                              [Fail]
[TrustZone Init]                       │
     │                                 ▼
     ▼                           [Tamper Response]
[Crypto Init: TRNG, PUF]               │
     │                                 └──► [Erase Keys]
     ▼                                      [Lock Device]
[Tamper Detection Enable]                  [Reset/Halt]
     │
     ▼
[Anti-Rollback Check] ────────────────┐
     │                                 │
     ▼                              [Fail]
[Control Flow Checks]                  │
     │                                 │
     ▼                                 │
[Measure Boot Stages]                  │
     │                                 │
     ▼                                 │
[Generate Attestation] ←──────────────┘
     │
     ▼
[Verify Attestation]
     │
     ▼
[Application Launch]
```
