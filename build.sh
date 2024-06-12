cat build/boot.bin build/filetable.bin build/kernel.bin > build/PhiOS.bin
qemu-system-i386 -fda build/PhiOS.bin