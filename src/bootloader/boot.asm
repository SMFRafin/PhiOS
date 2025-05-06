; bootloader/boot.asm
org 0x7C00

; Load filetable at 0x1000:0x0000
mov ax, 0x1000
mov es, ax
xor bx, bx
mov dl, 0x00   ; Drive 0 (floppy/hard)

mov ch, 0x00
mov dh, 0x00
mov cl, 0x02   ; Filetable at sector 2

read_filetable:
    mov ah, 0x02
    mov al, 64
    int 0x13
    jc read_filetable

; Load kernel at 0x2000:0x0000
mov ax, 0x2000
mov es, ax
xor bx, bx
mov cl, 0x03   ; Kernel starts at sector 3

read_kernel:
    mov ah, 0x02
    mov al, 64
    int 0x13
    jc read_kernel

; Jump to kernel
mov ax, 0x2000
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0xFFFF

jmp 0x2000:0x0000

times 510 - ($ - $$) db 0
dw 0xAA55
