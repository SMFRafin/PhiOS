;boot.asm
org 0x7C00  ; Origin of Bootloader

; Load filetable to 0x1000:0x0 from sector 2
mov bx, 0x1000  ; Set ES:BX to 0x1000:0x0
mov es, bx
mov bx, 0x0

mov dh, 0x0  ; Head number
mov dl, 0x0  ; Drive number
mov ch, 0x0  ; Track number
mov cl, 0x02  ; File table at sector 2

read_filetable:
    mov ah, 0x02  ; BIOS read sector function
    mov al, 64  ; Number of sectors to read
    int 0x13  ; Disk read interrupt
    jc read_filetable  ; Retry if there's an error

; Load kernel to 0x2000:0x0 from sector 3
mov bx, 0x2000  ; Set ES:BX to 0x2000:0x0
mov es, bx
mov bx, 0x0

mov dh, 0x0  ; Head number
mov dl, 0x0  ; Drive number
mov ch, 0x0  ; Track number
mov cl, 0x03  ; Kernel at sector 3

read_kernel:
    mov ah, 0x02  ; BIOS read sector function
    mov al, 64  ; Number of sectors to read
    int 0x13  ; Disk read interrupt
    jc read_kernel  ; Retry if there's an error

    mov ax, 0x2000  ; Move to 0x2000 for the kernel
    ; Set all the segments to 0x2000
    mov ds, ax
    mov es, ax
    mov ss, ax

jmp 0x2000:0x0  ; Jump to kernel address

times 510 - ($ - $$) db 0  ; Fill remaining bytes with 0s
dw 0xAA55  ; Boot signature