org 0x0000

; Set video mode
mov ah, 0x00
mov al, 0x03
int 0x10

; Set color
mov ah, 0x0B
mov bh, 0x00
mov bl, 0x01
int 0x10

; Print File Explorer message
mov si, msg
call print_string


filetable_input:
    ; Get input from user
    mov di, cmds
    call get_input

process_filetable_commands:
    mov byte [di], 0
    mov si, cmds
    call execute_filetable_command
    jmp filetable_input

execute_filetable_command:
    mov al, [si]
    cmp al, 'q'
    je return_to_kernel
    cmp al, 'Q'
    je return_to_kernel
    jne invalid_filetable_command

return_to_kernel:
    mov ax, 0x2000
    mov ds, ax
    mov es, ax

    call 0x2000:0x0000


invalid_filetable_command:
    mov si, invalid_cmd_msg
    call print_string
    jmp filetable_input



get_input:
    mov ah, 0x00
    int 0x16

    mov ah, 0x0E
    cmp al, 0x0D 
    je done_getting_input
    int 0x10
    mov [di], al
    inc di
    jmp get_input

done_getting_input:
    ret

print_string:
    mov ah, 0x0E
    mov bh, 0x0
    mov bl, 0x1

print_char:
    lodsb
    cmp al, 0
    je print_done
    int 0x10
    jmp print_char

print_done:
    ret

msg db '-------------------------------------------', 0xA, 0xD,0x20,0x20,0x20,0x20,'File Explorer ', 0xA, 0xD, \
'-------------------------------------------', 0xA, 0xD,\
'Press Q to return to the main menu',10,13, 0
invalid_cmd_msg db 10,13,'Invalid Command',10,13,0
cmds: db '', 0

times 512-($-$$) db 0
