ASM=nasm
SRC_DIR=src
BOOTLOADER_DIR=$(SRC_DIR)/bootloader
KERNEL_DIR=$(SRC_DIR)/kernel
FILETABLE_DIR=$(SRC_DIR)/filetable
BUILD_DIR=build

# Paths
BOOT_BIN=$(BUILD_DIR)/boot.bin
KERNEL_BIN=$(BUILD_DIR)/kernel.bin
FILETABLE_BIN=$(BUILD_DIR)/filetable.bin

# Default target
all: $(BOOT_BIN) $(KERNEL_BIN) $(FILETABLE_BIN)

# Assemble the bootloader
$(BOOT_BIN): $(BOOTLOADER_DIR)/boot.asm
	$(ASM) $< -f bin -o $@

# Assemble the kernel
$(KERNEL_BIN): $(KERNEL_DIR)/kernel.asm
	$(ASM) $< -f bin -o $@

# Assemble the filetable
$(FILETABLE_BIN): $(FILETABLE_DIR)/filetable.asm
	$(ASM) $< -f bin -o $@

# Clean up build files
clean:
	rm -f $(BUILD_DIR)/*

.PHONY: all clean