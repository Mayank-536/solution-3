# Compiler and flags
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

# Target MCU
MCU = cortex-m33
FPU = fpv5-sp-d16
FLOAT_ABI = hard

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# Compiler flags
CFLAGS = -mcpu=$(MCU) -mthumb -mfpu=$(FPU) -mfloat-abi=$(FLOAT_ABI)
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -O2 -g3
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -I$(INC_DIR)
CFLAGS += -DARM_MATH_CM33
CFLAGS += -DEFR32MG26

# Linker flags
LDFLAGS = -mcpu=$(MCU) -mthumb -mfpu=$(FPU) -mfloat-abi=$(FLOAT_ABI)
LDFLAGS += -specs=nosys.specs
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/secure_boot.map

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/bootloader/secure_boot.c \
          $(SRC_DIR)/bootloader/anti_rollback.c \
          $(SRC_DIR)/crypto/crypto_primitives.c \
          $(SRC_DIR)/tamper/tamper_detection.c \
          $(SRC_DIR)/debug/secure_debug.c \
          $(SRC_DIR)/attestation/attestation.c \
          $(SRC_DIR)/trustzone/trustzone.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Output files
TARGET = $(BUILD_DIR)/secure_boot.elf
TARGET_HEX = $(BUILD_DIR)/secure_boot.hex
TARGET_BIN = $(BUILD_DIR)/secure_boot.bin

# Default target
all: $(BUILD_DIR) $(TARGET) $(TARGET_HEX) $(TARGET_BIN) size

# Create build directory
$(BUILD_DIR):
	mkdir -p $(OBJ_DIR)/bootloader
	mkdir -p $(OBJ_DIR)/crypto
	mkdir -p $(OBJ_DIR)/tamper
	mkdir -p $(OBJ_DIR)/debug
	mkdir -p $(OBJ_DIR)/attestation
	mkdir -p $(OBJ_DIR)/trustzone

# Compile C files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

# Generate hex file
$(TARGET_HEX): $(TARGET)
	$(OBJCOPY) -O ihex $< $@

# Generate binary file
$(TARGET_BIN): $(TARGET)
	$(OBJCOPY) -O binary $< $@

# Display size
size: $(TARGET)
	$(SIZE) $<

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean size
