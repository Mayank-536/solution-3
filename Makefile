# Makefile for EFR32MG26 Secure Boot
# Copyright (c) 2026 EFR32MG26 Security Team

# Project name
PROJECT = efr32mg26_secure_boot

# Compiler settings
CC = arm-none-eabi-gcc
AR = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Source files
BOOTLOADER_SRC = $(SRC_DIR)/bootloader/secure_boot.c \
                 $(SRC_DIR)/bootloader/anti_rollback.c

TAMPER_SRC = $(SRC_DIR)/tamper_detection/tamper_detection.c

ATTESTATION_SRC = $(SRC_DIR)/attestation/attestation.c

TRUSTZONE_SRC = $(SRC_DIR)/trustzone/trustzone.c

PUF_SRC = $(SRC_DIR)/puf/puf.c

CONFIG_SRC = config/example_config.c

ALL_SRC = $(BOOTLOADER_SRC) $(TAMPER_SRC) $(ATTESTATION_SRC) \
          $(TRUSTZONE_SRC) $(PUF_SRC) $(CONFIG_SRC)

# Object files
BOOTLOADER_OBJ = $(BOOTLOADER_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TAMPER_OBJ = $(TAMPER_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
ATTESTATION_OBJ = $(ATTESTATION_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TRUSTZONE_OBJ = $(TRUSTZONE_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
PUF_OBJ = $(PUF_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
CONFIG_OBJ = $(CONFIG_SRC:%.c=$(OBJ_DIR)/%.o)

ALL_OBJ = $(BOOTLOADER_OBJ) $(TAMPER_OBJ) $(ATTESTATION_OBJ) \
          $(TRUSTZONE_OBJ) $(PUF_OBJ) $(CONFIG_OBJ)

# Compiler flags
CFLAGS = -mcpu=cortex-m33 \
         -mthumb \
         -mfloat-abi=hard \
         -mfpu=fpv5-sp-d16 \
         -O2 \
         -g \
         -Wall \
         -Wextra \
         -Werror \
         -ffunction-sections \
         -fdata-sections \
         -I$(INC_DIR)

# Security-hardened flags
CFLAGS += -fstack-protector-strong \
          -D_FORTIFY_SOURCE=2 \
          -Wformat-security \
          -fno-delete-null-pointer-checks

# TrustZone flags
CFLAGS += -mcmse

# Linker flags
LDFLAGS = -mcpu=cortex-m33 \
          -mthumb \
          -mfloat-abi=hard \
          -mfpu=fpv5-sp-d16 \
          -Wl,--gc-sections \
          -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map

# Targets
.PHONY: all clean bootloader tamper attestation trustzone puf config help

all: $(BIN_DIR)/$(PROJECT).elf $(BIN_DIR)/$(PROJECT).bin $(BIN_DIR)/$(PROJECT).hex
	@echo "=== Build Complete ==="
	@$(SIZE) $(BIN_DIR)/$(PROJECT).elf
	@echo ""
	@echo "Output files:"
	@echo "  ELF: $(BIN_DIR)/$(PROJECT).elf"
	@echo "  BIN: $(BIN_DIR)/$(PROJECT).bin"
	@echo "  HEX: $(BIN_DIR)/$(PROJECT).hex"

# Individual component targets
bootloader: $(BOOTLOADER_OBJ)
	@echo "Built bootloader components"

tamper: $(TAMPER_OBJ)
	@echo "Built tamper detection components"

attestation: $(ATTESTATION_OBJ)
	@echo "Built attestation components"

trustzone: $(TRUSTZONE_OBJ)
	@echo "Built TrustZone components"

puf: $(PUF_OBJ)
	@echo "Built PUF components"

config: $(CONFIG_OBJ)
	@echo "Built configuration"

# Create output directories
$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@
	@mkdir -p $(OBJ_DIR)/bootloader
	@mkdir -p $(OBJ_DIR)/tamper_detection
	@mkdir -p $(OBJ_DIR)/attestation
	@mkdir -p $(OBJ_DIR)/trustzone
	@mkdir -p $(OBJ_DIR)/puf
	@mkdir -p $(OBJ_DIR)/config

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@echo "Compiling $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Link
$(BIN_DIR)/$(PROJECT).elf: $(ALL_OBJ) | $(BIN_DIR)
	@echo "Linking $(PROJECT).elf"
	@$(CC) $(LDFLAGS) $^ -o $@

# Generate binary
$(BIN_DIR)/$(PROJECT).bin: $(BIN_DIR)/$(PROJECT).elf
	@echo "Creating $(PROJECT).bin"
	@$(OBJCOPY) -O binary $< $@

# Generate hex
$(BIN_DIR)/$(PROJECT).hex: $(BIN_DIR)/$(PROJECT).elf
	@echo "Creating $(PROJECT).hex"
	@$(OBJCOPY) -O ihex $< $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete"

# Help
help:
	@echo "EFR32MG26 Secure Boot Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build complete secure boot system (default)"
	@echo "  bootloader  - Build bootloader components only"
	@echo "  tamper      - Build tamper detection only"
	@echo "  attestation - Build attestation system only"
	@echo "  trustzone   - Build TrustZone configuration only"
	@echo "  puf         - Build PUF components only"
	@echo "  config      - Build configuration only"
	@echo "  clean       - Remove all build artifacts"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Build artifacts are placed in: $(BUILD_DIR)/"
	@echo ""
	@echo "Security Features Enabled:"
	@echo "  - Stack protection"
	@echo "  - Format string security"
	@echo "  - TrustZone-M support"
	@echo "  - Dead code elimination"
	@echo "  - Debug symbols"

# Print configuration
info:
	@echo "Build Configuration:"
	@echo "  Compiler: $(CC)"
	@echo "  CPU: Cortex-M33"
	@echo "  FPU: FPv5-SP-D16"
	@echo "  Optimization: -O2"
	@echo "  TrustZone: Enabled"
	@echo ""
	@echo "Source Files:"
	@echo "  Bootloader: $(words $(BOOTLOADER_SRC)) files"
	@echo "  Tamper: $(words $(TAMPER_SRC)) files"
	@echo "  Attestation: $(words $(ATTESTATION_SRC)) files"
	@echo "  TrustZone: $(words $(TRUSTZONE_SRC)) files"
	@echo "  PUF: $(words $(PUF_SRC)) files"
	@echo "  Total: $(words $(ALL_SRC)) files"
