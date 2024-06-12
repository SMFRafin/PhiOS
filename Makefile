ASM=nasm
SRC_DIR=src
BOOTLOADER_DIR=$(SRC_DIR)/bootloader
KERNEL_DIR=$(SRC_DIR)/kernel
FILETABLE_DIR=$(SRC_DIR)/filetable
BUILD_DIR=build
IMG_SIZE=1440k
BOOT_SECTOR=512
KERNEL_SECTORS=3
FILETABLE_SECTORS=1

# Paths
BOOT_BIN=$(BUILD_DIR)/boot.bin
KERNEL_BIN=$(BUILD_DIR)/kernel.bin
FILETABLE_BIN=$(BUILD_DIR)/filetable.bin
FLOPPY_IMG=$(BUILD_DIR)/main_floppy.img

# Default target
all: $(FLOPPY_IMG)

# Create the floppy image and write bootloader, kernel, and filetable
$(FLOPPY_IMG): $(BOOT_BIN) $(KERNEL_BIN) $(FILETABLE_BIN)
	# Create empty floppy image
	dd if=/dev/zero of=$@ bs=512 count=2880
	# Write bootloader to the first sector
	dd if=$(BOOT_BIN) of=$@ bs=$(BOOT_SECTOR) count=1 conv=notrunc
	# Write kernel to subsequent sectors
	dd if=$(KERNEL_BIN) of=$@ bs=$(BOOT_SECTOR) seek=2 count=$(KERNEL_SECTORS) conv=notrunc
	# Write filetable after the kernel
	dd if=$(FILETABLE_BIN) of=$@ bs=$(BOOT_SECTOR) seek=$$(( 2 + $(KERNEL_SECTORS) )) count=$(FILETABLE_SECTORS) conv=notrunc

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