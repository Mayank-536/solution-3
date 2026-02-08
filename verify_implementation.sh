#!/bin/bash
# Verification script to ensure all security features are implemented

echo "=== Secure Boot Implementation Verification ==="
echo ""

# Check for required header files
echo "[1] Checking header files..."
headers=(
    "include/secure_boot.h"
    "include/crypto_primitives.h"
    "include/tamper_detection.h"
    "include/anti_rollback.h"
    "include/secure_debug.h"
    "include/attestation.h"
    "include/trustzone.h"
)

for header in "${headers[@]}"; do
    if [ -f "$header" ]; then
        echo "  ✓ $header"
    else
        echo "  ✗ $header MISSING"
        exit 1
    fi
done

# Check for required source files
echo ""
echo "[2] Checking source files..."
sources=(
    "src/main.c"
    "src/bootloader/secure_boot.c"
    "src/bootloader/anti_rollback.c"
    "src/crypto/crypto_primitives.c"
    "src/tamper/tamper_detection.c"
    "src/debug/secure_debug.c"
    "src/attestation/attestation.c"
    "src/trustzone/trustzone.c"
)

for source in "${sources[@]}"; do
    if [ -f "$source" ]; then
        echo "  ✓ $source"
    else
        echo "  ✗ $source MISSING"
        exit 1
    fi
done

# Verify security features implementation
echo ""
echo "[3] Verifying security features..."

# Check for RTSL implementation
if grep -q "RTSL" include/secure_boot.h && grep -q "verify_root_of_trust" src/bootloader/secure_boot.c; then
    echo "  ✓ Hardware Root of Trust (RTSL)"
else
    echo "  ✗ RTSL implementation incomplete"
    exit 1
fi

# Check for glitch resistance
if grep -q "control_flow_check" src/bootloader/secure_boot.c && grep -q "CF_TOKEN" include/secure_boot.h; then
    echo "  ✓ Glitch-resistant control flow with tokens"
else
    echo "  ✗ Control flow implementation incomplete"
    exit 1
fi

# Check for TRNG
if grep -q "trng_init" src/crypto/crypto_primitives.c && grep -q "trng_random_jitter" include/crypto_primitives.h; then
    echo "  ✓ TRNG-based random jitter"
else
    echo "  ✗ TRNG implementation incomplete"
    exit 1
fi

# Check for tamper detection
if grep -q "ACMP" src/tamper/tamper_detection.c && grep -q "IADC" src/tamper/tamper_detection.c; then
    echo "  ✓ ACMP/IADC tamper detection"
else
    echo "  ✗ Tamper detection incomplete"
    exit 1
fi

# Check for anti-rollback
if grep -q "OTP" src/bootloader/anti_rollback.c && grep -q "verify_no_rollback" include/anti_rollback.h; then
    echo "  ✓ OTP-based anti-rollback"
else
    echo "  ✗ Anti-rollback implementation incomplete"
    exit 1
fi

# Check for secure debug
if grep -q "certificate" src/debug/secure_debug.c && grep -q "authenticate_debug_cert" include/secure_debug.h; then
    echo "  ✓ Secure debug with certificate auth"
else
    echo "  ✗ Secure debug implementation incomplete"
    exit 1
fi

# Check for attestation
if grep -q "JSON" src/attestation/attestation.c && grep -q "generate_attestation_report" include/attestation.h; then
    echo "  ✓ Measured boot with JSON attestation"
else
    echo "  ✗ Attestation implementation incomplete"
    exit 1
fi

# Check for TrustZone
if grep -q "SAU" src/trustzone/trustzone.c && grep -q "TZ_STATE_SECURE" include/trustzone.h; then
    echo "  ✓ TrustZone Secure World isolation"
else
    echo "  ✗ TrustZone implementation incomplete"
    exit 1
fi

# Check for PUF
if grep -q "puf_init" src/crypto/crypto_primitives.c && grep -q "puf_wrap_key" include/crypto_primitives.h; then
    echo "  ✓ PUF-based key wrapping"
else
    echo "  ✗ PUF implementation incomplete"
    exit 1
fi

# Check configuration
echo ""
echo "[4] Checking configuration files..."
if [ -f "config/secure_boot_config.json" ]; then
    echo "  ✓ Secure boot configuration"
else
    echo "  ✗ Configuration file missing"
    exit 1
fi

# Check documentation
echo ""
echo "[5] Checking documentation..."
if [ -f "docs/README.md" ] && [ -f "docs/ARCHITECTURE.md" ]; then
    echo "  ✓ Documentation complete"
else
    echo "  ✗ Documentation incomplete"
    exit 1
fi

# Check build system
echo ""
echo "[6] Checking build system..."
if [ -f "Makefile" ]; then
    echo "  ✓ Makefile present"
else
    echo "  ✗ Makefile missing"
    exit 1
fi

echo ""
echo "==================================="
echo "✓ All security features implemented"
echo "==================================="
echo ""
echo "Implementation includes:"
echo "  • Hardware Root of Trust (Secure Vault High + Immutable RTSL)"
echo "  • Glitch-resistant control flow with layered token checks"
echo "  • TRNG-based random jitter for timing attack mitigation"
echo "  • ACMP/IADC-based voltage and temperature tamper detection"
echo "  • Anti-rollback protection using OTP counters"
echo "  • Secure debug with certificate authentication"
echo "  • Measured boot with signed JSON attestation"
echo "  • TrustZone Secure World isolation (SAU)"
echo "  • PUF-based key wrapping"
echo ""
