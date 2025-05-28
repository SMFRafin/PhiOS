all: build/PhiOS.bin

build/boot.bin: src/bootloader/boot.asm
	nasm -f bin src/bootloader/boot.asm -o build/boot.bin

build/filetable.bin: src/filetable/filetable.asm
	nasm -f bin src/filetable/filetable.asm -o build/filetable.bin

build/kernel.bin: src/kernel/kernel.c src/kernel/print.c src/kernel/input.c src/kernel/commands.c src/kernel/string.c src/kernel/filesystem.c
	# Compile with real mode 16-bit code
	i386-elf-gcc -ffreestanding -m16 -c src/kernel/kernel.c -o build/kernel.o
	i386-elf-gcc -ffreestanding -m16 -c src/kernel/print.c -o build/print.o
	i386-elf-gcc -ffreestanding -m16 -c src/kernel/input.c -o build/input.o
	i386-elf-gcc -ffreestanding -m16 -c src/kernel/commands.c -o build/commands.o
	i386-elf-gcc -ffreestanding -m16 -c src/kernel/string.c -o build/string.o
	i386-elf-gcc -ffreestanding -m16 -c src/kernel/filesystem.c -o build/filesystem.o
	
	# Link with correct offset (0x0000 relative to segment 0x2000)
	i386-elf-ld -Ttext 0x0000 -o build/kernel_full.bin build/kernel.o build/print.o build/input.o build/commands.o build/string.o build/filesystem.o --oformat binary
	
	# Create kernel.bin (this step was missing)
	cp build/kernel_full.bin build/kernel.bin

build/PhiOS.bin: build/boot.bin build/filetable.bin build/kernel.bin
	# Create a 1.44MB floppy image filled with zeros
	dd if=/dev/zero of=build/PhiOS.bin bs=512 count=2880
	
	# Write the bootloader to the first sector
	dd if=build/boot.bin of=build/PhiOS.bin conv=notrunc bs=512 count=1
	
	# Write the filetable to the second sector
	dd if=build/filetable.bin of=build/PhiOS.bin conv=notrunc bs=512 seek=1
	
	# Write the kernel starting at the third sector
	dd if=build/kernel.bin of=build/PhiOS.bin conv=notrunc bs=512 seek=2

run: build/PhiOS.bin
	qemu-system-i386 -m 64 -fda build/PhiOS.bin -boot a -vga std

clean:
	rm -f build/*.bin build/*.o